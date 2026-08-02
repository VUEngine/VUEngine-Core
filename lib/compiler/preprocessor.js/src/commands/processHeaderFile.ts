/**
 * Port of processHeaderFile.sh.
 *
 * Turns a Virtual C header — a `class` block with attributes and possibly
 * `virtual`, `override` and `static` methods — into plain C: the class' vtable
 * macros, its attribute list, and free-function prototypes that take the
 * instance as an explicit first parameter.
 *
 * Along the way it maintains the shared state the rest of the build reads:
 *
 *   - `classes/hierarchies/classesHierarchy.txt`, one line per class recording
 *     its base class and modifiers;
 *   - `classes/dictionaries/<Class>Methods{Owned,Inherited,Virtual}.txt`, which
 *     processSourceFile uses to rewrite call sites;
 *   - `classes/dependencies/<plugin>/<Class>.d`, a make dependency fragment.
 *
 * Because a class cannot be processed before its base class, this recurses into
 * the base class' header first and coordinates with the other parallel make
 * jobs through the lock protocol in util/lock.ts.
 */

import { grep, grepText } from '../posix/grep.ts';
import { subst, substAll } from '../posix/sed.ts';
import {
	capture,
	countLines,
	cut,
	cutFrom,
	expandUnquoted,
	head,
	selectRange,
	tail,
	tailFrom,
	trDelete,
	uniqueInOrder,
	words,
} from '../posix/text.ts';
import {
	appendLine,
	appendText,
	findAllByName,
	findFirstByName,
	isDirectory,
	isFile,
	isNewerThan,
	mkdirp,
	readText,
	removeFile,
	touch,
	writeText,
} from '../util/files.ts';
import { releaseLock, tryToLock, waitLong, waitShort, type LockContext } from '../util/lock.ts';
import { ExitSignal, flagOrEnvValue, flagValue, flagValues, parseArguments } from '../util/shell.ts';

/** Matches a Virtual C class declaration: `[modifier] class Name : BaseName`. */
const CLASS_DECLARATION = '^[ \t]*[A-z0-9\\!]*[ \t]*class[ \t]\\+[A-Z][A-z0-9]*[ \t]*:[ \t]*[A-Z][A-z0-9]*';

/** How long the hierarchy computation waits for a base class, in iterations. */
const HIERARCHY_WAIT_LIMIT = 2000;

interface Settings {
	engineHome: string;
	caller: string;
	inputFile: string;
	outputFile: string;
	workingFolder: string;
	classesHierarchyFile: string;
	headersFolder: string;
	pluginsName: string;
	platformsFolder: string;
	pluginsFolder: string;
	userPluginsFolder: string;
	plugins: string[];
	gameName: string;
	gameHome: string;
}

function readSettings(argv: readonly string[]): Settings {
	const parsed = parseArguments(
		argv,
		{ valued: ['-e', '-g', '-i', '-o', '-w', '-c', '-h', '-n', '-t', '-p', '-u', '-l', '-x', '-m'], boolean: ['-d'] },
		'ignore-last',
	);

	return {
		engineHome: flagOrEnvValue(parsed, '-e', 'ENGINE_HOME'),
		caller: flagOrEnvValue(parsed, '-g', 'CALLER'),
		inputFile: flagValue(parsed, '-i'),
		outputFile: flagValue(parsed, '-o'),
		// The script assigned this unconditionally after parsing, so `-w` never
		// actually took effect and every path is relative to the game folder.
		workingFolder: 'build/working',
		classesHierarchyFile: flagValue(parsed, '-c'),
		headersFolder: flagValue(parsed, '-h'),
		pluginsName: flagValue(parsed, '-n'),
		platformsFolder: flagOrEnvValue(parsed, '-t', 'PLATFORMS_FOLDER'),
		pluginsFolder: flagOrEnvValue(parsed, '-p', 'PLUGINS_FOLDER'),
		userPluginsFolder: flagOrEnvValue(parsed, '-u', 'USER_PLUGINS_FOLDER'),
		plugins: flagValues(parsed, '-l'),
		gameName: flagOrEnvValue(parsed, '-x', 'GAME_NAME'),
		gameHome: flagOrEnvValue(parsed, '-m', 'GAME_HOME'),
	};
}

/** Entry point. Returns the process exit status. */
export function processHeaderFile(argv: readonly string[]): number {
	try {
		run(readSettings(argv));

		return 0;
	} catch (error) {
		if (error instanceof ExitSignal) {
			return error.code;
		}

		throw error;
	}
}

function run(settings: Settings): void {
	const { inputFile, outputFile, workingFolder } = settings;

	if (settings.caller === '') {
		console.log('NO CALLER!!!!');
	}

	if (inputFile === '' || !isFile(inputFile)) {
		console.log(`Input file not found: ${inputFile}`);
		throw new ExitSignal(1);
	}

	if (inputFile === outputFile) {
		console.log(`Input and output files are the same: ${inputFile}`);
		throw new ExitSignal(0);
	}

	// A copy with carriage returns stripped, which everything downstream reads.
	const cleanInputFile = `${outputFile}.clean`;
	const cleanInput = trDelete(readText(inputFile), '\r');
	writeText(cleanInputFile, cleanInput);

	const classDeclaration = grepText(cleanInput, CLASS_DECLARATION, { lineNumber: true });
	const cleanClassDeclaration = cut(classDeclaration, ':', [2, 3]);
	const className = subst(cleanClassDeclaration, '^.*class \\([A-z][A-z0-9]*\\)[ \t]*\\:.*', '\\1');
	let baseClassName = subst(cut(cleanClassDeclaration, ':', [2]), '[^[:alnum:]_-]', '', { global: true });

	// Headers without a class declaration pass through untouched.
	if (className === '') {
		writeText(outputFile, trDelete(readText(inputFile), '\r'));
		cleanUp(settings, '');
		throw new ExitSignal(0);
	}

	mkdirp(`${workingFolder}/classes/logs`);
	mkdirp(`${workingFolder}/classes/locks`);

	const classLogFile = `${workingFolder}/classes/logs/${className}.log`;
	const context: LockContext = {
		log: (message) => appendLine(classLogFile, message),
		caller: settings.caller,
		inputFile,
	};

	// Guards against the race between the makefile invoking this for a header
	// and another class' run invoking it for the same header as a base class.
	const classLock = `${workingFolder}/classes/locks/${className}`;

	if (!tryToLock(classLock, context, 'give-up')) {
		cleanUp(settings, className);
		throw new ExitSignal(0);
	}

	const releaseLocks = (): void => releaseLock(classLock, context);

	writeText(classLogFile, `Got lock on calling from ${settings.caller}\n`);
	context.log(`INPUT_FILE ${inputFile}`);
	context.log(`OUTPUT_FILE ${outputFile}`);

	if (isUpToDateByDependencies(settings, className)) {
		cleanUp(settings, className);
		releaseLocks();
		context.log(`Already processed on caller ${settings.caller}`);
		throw new ExitSignal(0);
	}

	context.log(`Will check if base class ${baseClassName} needs to be processed on caller ${settings.caller}`);

	const mustBeReprocessed = ensureBaseClassIsProcessed(settings, className, baseClassName, context);

	const ownedMethodsDictionary = `${workingFolder}/classes/dictionaries/${className}MethodsOwned.txt`;
	const inheritedMethodsDictionary = `${workingFolder}/classes/dictionaries/${className}MethodsInherited.txt`;
	const virtualMethodsDictionary = `${workingFolder}/classes/dictionaries/${className}MethodsVirtual.txt`;

	if (isFile(ownedMethodsDictionary) && isFile(inheritedMethodsDictionary)) {
		if (!mustBeReprocessed && isFile(outputFile) && isNewerThan(outputFile, inputFile)) {
			cleanUp(settings, className);
			releaseLocks();
			context.log(`Don't need processing, base class is fine, and I'm newer on caller ${settings.caller}`);
			throw new ExitSignal(0);
		}
	}

	context.log('Starting preprocessing');

	const classModifiers = subst(cleanClassDeclaration, '^\\(.*\\)class .*', '\\1');
	const declarationLine = Number(cut(classDeclaration, ':', [1]));

	const { baseClassesNames, baseClassesNamesHelper } = computeHierarchy(
		settings,
		className,
		baseClassName,
		outputFile,
		context,
		releaseLocks,
	);

	context.log(`Hierarchy on caller ${settings.caller}: ${baseClassesNames}`);

	// Object must not be made to inherit from itself.
	if (className === 'Object') {
		baseClassName = '';
	}

	const { methods, attributes } = parseClassBody(cleanInput, declarationLine);
	const bodyEnd = findClassBodyEnd(cleanInput, declarationLine);
	const bodyStart = declarationLine + 1;

	context.log(`Computing attributes and methods on caller ${settings.caller}`);

	const isExtensionClass = classModifiers.includes('extension ');
	const isMutationClass = classModifiers.includes('mutation ');

	let virtualMethodDeclarations = `#define ${className}_METHODS(ClassName)`;
	let virtualMethodOverrides = `#define ${className}_SET_VTABLE(ClassName)`;

	if (baseClassName !== '') {
		virtualMethodDeclarations += ` ${baseClassName}_METHODS(ClassName) `;
		virtualMethodOverrides += ` ${baseClassName}_SET_VTABLE(ClassName) `;
	}

	context.log(`Writing owned methods on caller ${settings.caller}`);

	// An extension adds methods to a class that already has dictionaries, so it
	// must append to them rather than start over.
	if (!isExtensionClass) {
		removeFile(ownedMethodsDictionary);
		context.log(`Writing inherited methods on caller ${settings.caller}`);
		removeFile(inheritedMethodsDictionary);
	}

	touch(ownedMethodsDictionary);
	touch(inheritedMethodsDictionary);

	context.log(`Writing virtual methods on caller ${settings.caller}`);

	if (!isExtensionClass) {
		removeFile(virtualMethodsDictionary);
		touch(virtualMethodsDictionary);
	}

	const dependenciesFile = `${workingFolder}/classes/dependencies/${settings.pluginsName}/${className}.d`;
	const searchPaths = buildSearchPaths(settings);

	context.log(`Starting computation of dependencies on caller ${settings.caller} with search path  `);

	if (baseClassesNames !== '') {
		const outputFileClean = substAll(outputFile, [
			[settings.gameHome, '', { global: true }],
			['/build/', '', { global: true }],
			['build/', '', { global: true }],
		]);
		writeText(dependenciesFile, `${settings.gameHome}/build/${outputFileClean}: \\\n`);
	}

	collectAncestors(settings, {
		className,
		baseClassName,
		baseClassesNames,
		searchPaths,
		ownedMethodsDictionary,
		inheritedMethodsDictionary,
		virtualMethodsDictionary,
		dependenciesFile,
		context,
		releaseLocks,
	});

	if (baseClassesNames !== '') {
		appendLine(dependenciesFile, ` ${inputFile} `);
	}

	context.log(`Computation of dependencies done on caller ${settings.caller}`);
	removeFile(`${dependenciesFile}-e`);

	context.log(`Computing final header text on caller ${settings.caller}`);

	const emitted = buildMethodDeclarations({
		className,
		methods,
		virtualMethodDeclarations,
		virtualMethodOverrides,
	});

	virtualMethodDeclarations = emitted.virtualMethodDeclarations;
	virtualMethodOverrides = emitted.virtualMethodOverrides;

	if (emitted.virtualMethodNames !== '') {
		appendLine(
			virtualMethodsDictionary,
			subst(emitted.virtualMethodNames, '\\(^.*\\)', `${className}_\\1 __VIRTUAL_CALL(${className},\\1,`, {
				global: true,
			}),
		);
	}

	if (emitted.methodCalls !== '') {
		appendLine(inheritedMethodsDictionary, emitted.methodCalls);
	}

	context.log(`Cleaning owned methods dictionary on caller ${settings.caller}`);
	rewriteUnique(ownedMethodsDictionary);

	context.log(`Writing owned methods dictionary on caller ${settings.caller}`);
	writeLines(ownedMethodsDictionary, grep(readText(ownedMethodsDictionary), '_constructor\\|_destructor\\|_new', { invert: true }));

	context.log(`Writing virtual methods dictionary on caller ${settings.caller}`);
	rewriteUnique(virtualMethodsDictionary);

	context.log(`Writing inherited methods dictionary on caller ${settings.caller}`);
	rewriteUnique(inheritedMethodsDictionary);

	context.log(`Computing class modifiers on caller ${settings.caller}`);
	context.log(`Modifiers: ${classModifiers}`);

	const flags = readModifierFlags(classModifiers);
	let methodDeclarations = emitted.methodDeclarations;

	if (!flags.isStaticClass && !isExtensionClass && !isMutationClass) {
		if (flags.isSingletonClass) {
			methodDeclarations +=
				`\n\tvoid ${className}_secure(ClassPointer const (*requesterClasses)[]);` +
				`\n\t${className} ${className}_getInstance(ClassPointer requesterClass);`;
		}

		methodDeclarations += `\n\tvoid ${className}_destructor(void* _this);`;
	}

	context.log(`Computing constructor/destructor/allocators on caller ${settings.caller}`);

	if (
		!flags.isAbstractClass &&
		!flags.isSingletonClass &&
		!flags.isStaticClass &&
		!isExtensionClass &&
		!isMutationClass
	) {
		const constructor = grepText(methodDeclarations, `void[ \t]\\+${className}_constructor[ \t]*(.*);`, { max: 1 });

		if (constructor === '') {
			console.log(`${className}: no constructor defined for ${className} : ${baseClassName} in ${methodDeclarations}`);
			cleanUp(settings, className);
			releaseLocks();
			throw new ExitSignal(1);
		}

		const parameters = subst(
			cutFrom(cut(cutFrom(expandUnquoted(constructor), '(', 2), ')', [1]), ',', 2),
			'^.*this',
			'',
		);
		methodDeclarations += `\n\t${className} ${className}_new(${parameters});`;
	}

	context.log(`Writing temporal file on caller ${settings.caller}`);

	const temporalFile = `${workingFolder}/classes/${className}Temporal.txt`;
	touch(temporalFile);

	let temporal = '';

	if (!flags.isStaticClass && !isExtensionClass) {
		temporal += `${virtualMethodDeclarations}\n${virtualMethodOverrides}\n`;
		temporal +=
			baseClassName !== ''
				? `#define ${className}_ATTRIBUTES ${baseClassName}_ATTRIBUTES ${attributes}\n`
				: `#define ${className}_ATTRIBUTES ${attributes}\n`;
	}

	if (!isExtensionClass) {
		if (!flags.isStaticClass) {
			temporal += `#ifndef FORWARD_DECLARE_${className}\n`;
			temporal += `#define FORWARD_DECLARE_${className}\n`;
			temporal += `__FORWARD_CLASS(${className});\n`;
			temporal += '#endif\n';
			temporal += `__CLASS(${className});\n`;
		} else {
			temporal += `__CLASS_FUNDAMENTAL_METHODS(${className});\n`;
		}
	}

	context.log(`Padding temporal file on caller ${settings.caller}`);

	temporal += `${subst(methodDeclarations, 'static[ \t]\\+', '', { global: true })}\n`;
	appendText(temporalFile, temporal);

	context.log(`Writing ${outputFile} file on caller ${settings.caller}`);

	// Splice the generated block in where the class declaration used to be.
	const prelude = bodyStart - 2;
	const totalLines = countLines(cleanInput);
	const remaining = totalLines - bodyEnd + 1;

	let output = head(cleanInput, prelude) + readText(temporalFile) + tail(cleanInput, remaining);

	output = substAll(output, [
		[
			'^[ \t]*class[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*;',
			'#ifndef FORWARD_DECLARE_\\1\\n #define FORWARD_DECLARE_\\1\\n __FORWARD_CLASS(\\1);\\n #endif\\n',
		],
		['\\([A-Z][A-z0-9]*\\)::\\([a-z][A-z0-9]*\\)', '\\1_\\2', { global: true }],
		['static[ \t]inline[ \t]', 'inline ', { global: true }],
		['inline[ \t]static[ \t]', 'inline ', { global: true }],
	]);

	writeText(outputFile, output);

	context.log(`Writing class hierarchy on caller ${settings.caller}`);

	recordHierarchy(settings, className, baseClassesNamesHelper, classModifiers, context);

	context.log(`Done on caller ${settings.caller}`);
	cleanUp(settings, className);

	console.log(isExtensionClass ? `Preprocessed extension for: ${className}` : `Preprocessed class: ${className}`);

	releaseLocks();
	context.log(`Released locks on caller ${settings.caller}`);
}

function cleanUp(settings: Settings, className: string): void {
	if (className !== '') {
		removeFile(`${settings.workingFolder}/classes/${className}Temporal.txt`);
	}

	removeFile(`${settings.outputFile}-e`);
}

/** Whether a recorded dependency changed since the output was last written. */
function isUpToDateByDependencies(settings: Settings, className: string): boolean {
	const dependenciesFile = `${settings.workingFolder}/classes/dependencies/${settings.pluginsName}/${className}.d`;

	if (!isFile(dependenciesFile)) {
		return false;
	}

	// The first line is the make target; the rest are the prerequisites.
	const dependencies = words(tailFrom(subst(readText(dependenciesFile), '[\\\\:]', '', { global: true }), 2));

	for (const dependency of dependencies) {
		if (isNewerThan(dependency, settings.outputFile)) {
			return false;
		}
	}

	return isNewerThan(settings.outputFile, settings.inputFile);
}

/**
 * Preprocesses the base class' header first, so its dictionaries exist before
 * this class copies from them. Returns whether this class must be rebuilt
 * because the base class turned out to be newer.
 */
function ensureBaseClassIsProcessed(
	settings: Settings,
	className: string,
	baseClassName: string,
	context: LockContext,
): boolean {
	if (className === 'Object') {
		return false;
	}

	const { workingFolder, outputFile } = settings;
	const baseClassFile = findFirstByName([`${settings.headersFolder}/source`], `${baseClassName}.h`);
	let processedBaseClassFile = subst(
		baseClassFile,
		'.*/source/',
		`${workingFolder}/headers/${settings.pluginsName}/source/`,
		{ global: true },
	);

	let mustBeReprocessed = false;

	if (isFile(baseClassFile)) {
		if (isFile(processedBaseClassFile) && isNewerThan(processedBaseClassFile, outputFile)) {
			mustBeReprocessed = true;
		} else if (!isDirectory(`${workingFolder}/classes/locks/${baseClassName}.lock`)) {
			context.log(`${baseClassName} needs preprocessing, calling it`);

			// Recursion rather than a subprocess. The lock this run holds is for
			// this class, and the nested run takes the base class' own lock, so
			// the protocol is unchanged. Note that `-x` is deliberately absent,
			// matching the original invocation.
			processHeaderFile([
				'-e', settings.engineHome,
				'-i', baseClassFile,
				'-o', processedBaseClassFile,
				'-m', settings.gameHome,
				'-w', settings.workingFolder,
				'-c', settings.classesHierarchyFile,
				'-n', settings.pluginsName,
				'-h', settings.headersFolder,
				'-t', settings.platformsFolder,
				'-p', settings.pluginsFolder,
				'-u', settings.userPluginsFolder,
				'-g', className,
				...settings.plugins.flatMap((plugin) => ['-l', plugin]),
			]);
		} else {
			mustBeReprocessed = true;
		}
	}

	if (!mustBeReprocessed) {
		if (!isFile(processedBaseClassFile)) {
			processedBaseClassFile = findFirstByName([`${workingFolder}/headers`], `${baseClassName}.h`);
		}

		if (isFile(processedBaseClassFile) && isNewerThan(processedBaseClassFile, outputFile)) {
			mustBeReprocessed = true;
		}
	}

	return mustBeReprocessed;
}

/**
 * Waits for the base class to be preprocessed, then records the hierarchy.
 *
 * The shell version looked as though it walked the whole ancestor chain, but a
 * quoting error in its `grep` made the walk stop after the direct base class,
 * and the recorded hierarchy lines show it: `VUEngine:ListenerObject:::singleton`
 * has the empty field the aborted walk leaves behind. Transitive inheritance
 * still works, because each class copies its base class' already-transitive
 * dictionaries. The behaviour is reproduced exactly so the generated files stay
 * byte-identical.
 */
function computeHierarchy(
	settings: Settings,
	className: string,
	baseClassName: string,
	outputFile: string,
	context: LockContext,
	releaseLocks: () => void,
): { baseClassesNames: string; baseClassesNamesHelper: string } {
	const { workingFolder } = settings;
	let baseClassesNamesHelper = `${baseClassName}:`;

	if (className === 'Object') {
		return { baseClassesNames: '', baseClassesNamesHelper };
	}

	if (baseClassName !== '') {
		let processedBaseClassFile = findFirstByName([`${workingFolder}/headers`], `${baseClassName}.h`);
		const baseClassLock = `${workingFolder}/classes/locks/${baseClassName}.lock`;
		let counter = 0;

		while (processedBaseClassFile === '' || !isFile(processedBaseClassFile) || isDirectory(baseClassLock)) {
			if (counter > HIERARCHY_WAIT_LIMIT) {
				counter = 0;
				context.log(`Waiting for ${baseClassName} during computation of whole hierarchy`);
			}

			waitLong();
			processedBaseClassFile = findFirstByName([`${workingFolder}/headers`], `${baseClassName}.h`);
			counter++;

			if (counter > HIERARCHY_WAIT_LIMIT - 1) {
				context.log(
					`Error processing ${className} while computing hierarchy on ${baseClassName} with file ${processedBaseClassFile} not found`,
				);
				console.log(`${className}: base class ${baseClassName} not found`);
				cleanUp(settings, className);
				releaseLocks();
				throw new ExitSignal(1);
			}
		}
	}

	context.log(`Starting computation of whole hierarchy on caller ${settings.caller}`);

	let classesHierarchy = '';

	for (const file of findAllByName(`${workingFolder}/classes/hierarchies`, 'classesHierarchy.txt')) {
		tryToLock(file, context);
		classesHierarchy += `\n${capture(readText(file))}`;
		releaseLock(file, context);
	}

	// A singleton or final base class cannot be inherited from.
	if (grep(classesHierarchy, `^${baseClassName}:.*`).some((line) => line.includes('singleton!'))) {
		console.log(`ERROR: ${className} inherits from ${baseClassName} but`);
		console.log(`\t${baseClassName} is final because it is a singleton`);
		removeFile(outputFile);
		throw new ExitSignal(0);
	}

	if (grep(classesHierarchy, `^${baseClassName}:.*`).some((line) => line.includes('final '))) {
		console.log(`ERROR: ${className} inherits from ${baseClassName} but`);
		console.log(`\t${baseClassName} is final`);
		removeFile(outputFile);
		throw new ExitSignal(0);
	}

	// The aborted walk described above: one step that contributes nothing but
	// the empty field, then the loop's `Object` test ends it.
	if (baseClassName === 'Object') {
		return { baseClassesNames: baseClassName, baseClassesNamesHelper };
	}

	baseClassesNamesHelper += ':';

	return { baseClassesNames: ` ${baseClassName}`, baseClassesNamesHelper };
}

/** The line holding the `}` that closes the class body. */
function findClassBodyEnd(cleanInput: string, declarationLine: number): number {
	const relative = grep(tailFrom(cleanInput, declarationLine), '}', { lineNumber: true, max: 1 })[0];

	return declarationLine + Number(relative?.split(':')[0] ?? 0);
}

/**
 * Reduces the class body to one declaration per line, then splits it into
 * method declarations and attributes.
 */
function parseClassBody(cleanInput: string, declarationLine: number): { methods: string; attributes: string } {
	const end = findClassBodyEnd(cleanInput, declarationLine);
	const body = selectRange(cleanInput, declarationLine + 1, end);

	// Drop comment lines, turn the braces into breaks, collapse everything onto
	// one line and then split it again on semicolons, so each declaration ends
	// up alone on a line regardless of how it was wrapped in the source.
	let block = grep(body, '^[ \t]*[\\*//]\\+.*', { invert: true }).join('\n');
	block = subst(block, '[{}]', '\\n');
	block = trDelete(block, '\r\n');
	block = subst(block, ';', ';\\n', { global: true });

	// Pull a parameter list up onto its declaration's line.
	block = subst(block, '[\t ]*(', '(');
	block = subst(block, '[\t ]*(', '(');

	const methods = grep(
		grep(block, '^[ \t\\*A-z0-9]\\+[ \t]*([ \t]*\\*', { invert: true }).join('\n'),
		'(.*)[ \t=0]*;[ \t]*',
	).join('\n');

	const attributes = trDelete(
		subst(
			grep(
				grep(block, '^[ \t\\*A-z0-9]\\+[ \t]*([ \t]*[^\\*]', { invert: true }).join('\n'),
				';',
			).join('\n'),
			'&\\\\',
			'',
		),
		'\r\n',
	);

	return { methods, attributes };
}

/** Where to look for a class' header: this component, the engine, then plugins. */
function buildSearchPaths(settings: Settings): string[] {
	const searchPaths = [`${settings.headersFolder}/source`, `${settings.engineHome}/source`];

	for (const rawPlugin of settings.plugins) {
		const plugin = subst(rawPlugin, '(user//|platforms//|vuengine//)', '/', { extended: true });

		if (isDirectory(`${settings.platformsFolder}/${plugin}`)) {
			searchPaths.push(`${settings.platformsFolder}/${plugin}/source`);
		} else if (isDirectory(`${settings.pluginsFolder}/${plugin}`)) {
			searchPaths.push(`${settings.pluginsFolder}/${plugin}/source`);
		} else {
			searchPaths.push(`${settings.userPluginsFolder}/${plugin}/source`);
		}
	}

	return searchPaths;
}

interface AncestorContext {
	className: string;
	baseClassName: string;
	baseClassesNames: string;
	searchPaths: string[];
	ownedMethodsDictionary: string;
	inheritedMethodsDictionary: string;
	virtualMethodsDictionary: string;
	dependenciesFile: string;
	context: LockContext;
	releaseLocks: () => void;
}

/**
 * Copies each ancestor's dictionaries into this class', rewriting the class
 * names, and records the header files this class depends on.
 */
function collectAncestors(settings: Settings, ancestors: AncestorContext): void {
	const { workingFolder } = settings;

	for (const ancestorClassName of words(ancestors.baseClassesNames)) {
		const ancestorInherited = `${workingFolder}/classes/dictionaries/${ancestorClassName}MethodsInherited.txt`;
		const ancestorVirtual = `${workingFolder}/classes/dictionaries/${ancestorClassName}MethodsVirtual.txt`;

		if (ancestors.className !== 'Object') {
			const ancestorLock = `${workingFolder}/classes/locks/${ancestorClassName}.lock`;
			let counter = 0;

			while (isDirectory(ancestorLock) || !isFile(ancestorVirtual)) {
				counter++;

				if (counter > 100) {
					counter = 0;
					ancestors.context.log(`${ancestors.className} waiting (2) for ${ancestors.baseClassName}`);
				}

				waitShort();
			}
		}

		const inherited = readText(ancestorInherited);

		// An inherited method becomes an owned entry mapping this class' name for
		// the method onto the ancestor's implementation.
		appendText(
			ancestors.ownedMethodsDictionary,
			subst(inherited, '^\\([A-Z][A-z]*\\)_\\(.*\\)', `${ancestors.className}_\\2 \\1_\\2`, { global: true }),
		);
		appendText(ancestors.inheritedMethodsDictionary, inherited);
		appendText(
			ancestors.virtualMethodsDictionary,
			subst(readText(ancestorVirtual), ancestorClassName, ancestors.className, { global: true }),
		);

		const headerFile = findFirstByName(ancestors.searchPaths, `${ancestorClassName}.h`);

		if (!isFile(headerFile)) {
			console.log(
				`${ancestors.className}: header file not found for ${ancestorClassName} in ${ancestors.searchPaths.join(' ')} with ${settings.plugins.map((plugin) => ` ${plugin}`).join('')} `,
			);
			removeFile(ancestors.dependenciesFile);
			removeFile(settings.outputFile);
			cleanUp(settings, ancestors.className);
			ancestors.releaseLocks();
			throw new ExitSignal(1);
		}

		// Both the original and the preprocessed base header are prerequisites.
		appendLine(ancestors.dependenciesFile, ` ${headerFile} \\`);

		const preprocessedHeader = substAll(headerFile, [
			[settings.pluginsFolder, `${workingFolder}/headers/vuengine`, { global: true }],
			[settings.platformsFolder, `${workingFolder}/headers/platforms`, { global: true }],
			[settings.userPluginsFolder, `${workingFolder}/headers/user`, { global: true }],
			[settings.engineHome, `${workingFolder}/headers/core`, { global: true }],
			[`^.*/${settings.gameName}`, `${workingFolder}/headers/${settings.gameName}`, { global: true }],
		]);

		appendLine(ancestors.dependenciesFile, ` ${settings.gameHome}/${preprocessedHeader} \\`);
	}
}

interface MethodEmission {
	methodDeclarations: string;
	virtualMethodDeclarations: string;
	virtualMethodOverrides: string;
	virtualMethodNames: string;
	methodCalls: string;
}

/**
 * Rewrites the class' method declarations into C prototypes and derives the
 * vtable macro bodies from the `virtual` and `override` ones.
 */
function buildMethodDeclarations(input: {
	className: string;
	methods: string;
	virtualMethodDeclarations: string;
	virtualMethodOverrides: string;
}): MethodEmission {
	const { className, methods } = input;

	// Move each modifier to the end of its line as a marker, so the later
	// patterns can key off it without having to re-parse the declaration.
	let methodDeclarations = substAll(methods, [
		['^[ \t][ \t]*\\(virtual\\)[ \t][ \t]*\\(.*\\)', '\\2<\\1>'],
		['^[ \t][ \t]*\\(override\\)[ \t][ \t]*\\(.*$\\)', '\\2<\\1>'],
		['^[ \t][ \t]*\\(static\\)[ \t][ \t]*\\(.*$\\)', '\\2<\\1>'],
	]);

	const virtualDeclarations = trDelete(
		subst(
			subst(
				grep(methodDeclarations, '<virtual>').join('\n'),
				'\\(^.*\\)[ \t][ \t]*\\([a-z][A-z0-9]*\\)(\\([^;]*;\\)<virtual>.*',
				' __VIRTUAL_DEC(ClassName,\\1,\\2,ClassName,\\3',
				{ global: true },
			),
			',[ \t]*)[ \t]*;',
			');',
			{ global: true },
		),
		'\r\n',
	);

	const overrides = trDelete(
		subst(
			grep(
				grep(methodDeclarations, '<override>\\|<virtual>').join('\n'),
				')[ \t]*=[ \t]*0[ \t]*;',
				{ invert: true },
			).join('\n'),
			'^.*[ \t][ \t]*\\([a-z][A-z0-9]*\\)(.*',
			` __VIRTUAL_SET(ClassName,${className},\\1);`,
			{ global: true },
		),
		'\r\n',
	);

	const virtualMethodNames = subst(
		subst(grep(methodDeclarations, '<virtual>').join('\n'), '^.*[ \t][ \t]*\\([a-z][A-z0-9]*\\)(.*$', '\\1', {
			global: true,
		}),
		',[ \t]*)[ \t]*;',
		');',
		{ global: true },
	);

	// Non-virtual, non-static methods are the ones subclasses inherit by name.
	const methodCalls = subst(
		grep(methodDeclarations, '<static>\\|<virtual>\\|<override>', { invert: true }).join('\n'),
		'^.*[ \t][ \t]*\\([a-z][A-z0-9]*\\)(.*$',
		`${className}_\\1`,
		{ global: true },
	);

	const virtualMethodDeclarations = substAll(`${input.virtualMethodDeclarations} ${virtualDeclarations}`, [
		[')[ \t]*=[ \t]*0[ \t]*;', ');', { global: true }],
		[',[ \t]*)[ \t]*;', ');', { global: true }],
	]);

	// Turn each declaration into a free function taking the instance first.
	methodDeclarations = subst(methodDeclarations, ')[ \t]*=[ \t]*0[ \t]*;', ');', { global: true });
	methodDeclarations = subst(
		methodDeclarations,
		'\\(^.*[ \t][ \t]*\\)\\([a-z][A-z0-9]*\\)(\\(.*\\)',
		`\\1${className}_\\2(void* _this,\\3`,
		{ global: true },
	);
	methodDeclarations = substAll(methodDeclarations, [
		['\\(^.*\\)void\\* _this,\\(.*\\)<static>', '\\1\\2', { global: true }],
		['<virtual>', '\t'],
		['<override>', '\t'],
		['<static>', '\t'],
		[',[ \t]*)[ \t]*;', ');', { global: true }],
	]);

	return {
		methodDeclarations,
		virtualMethodDeclarations,
		virtualMethodOverrides: `${input.virtualMethodOverrides} ${overrides}`,
		virtualMethodNames,
		methodCalls,
	};
}

interface ModifierFlags {
	isSingletonClass: boolean;
	isAbstractClass: boolean;
	isStaticClass: boolean;
	isFinalClass: boolean;
}

function readModifierFlags(classModifiers: string): ModifierFlags {
	const flags: ModifierFlags = {
		isSingletonClass: false,
		isAbstractClass: false,
		isStaticClass: false,
		isFinalClass: false,
	};

	for (const modifier of classModifiers.split('\n')) {
		if (modifier === '') {
			continue;
		}

		flags.isAbstractClass ||= modifier.includes('abstract ');
		flags.isFinalClass ||= modifier.includes('final ');
		flags.isSingletonClass ||= modifier.includes('singleton');
		flags.isStaticClass ||= modifier.includes('static ');
	}

	return flags;
}

/** Replaces this class' line in the shared hierarchy file, under its lock. */
function recordHierarchy(
	settings: Settings,
	className: string,
	baseClassesNamesHelper: string,
	classModifiers: string,
	context: LockContext,
): void {
	const file = settings.classesHierarchyFile;

	tryToLock(file, context);
	touch(file);

	const existing = subst(readText(file), `^${className}:.*`, '', { global: true });
	const kept = grep(existing, '^[[:space:]]*$', { invert: true });

	writeText(file, kept.length === 0 ? '' : `${kept.join('\n')}\n`);
	appendLine(file, `${className}:${baseClassesNamesHelper}:${classModifiers}`);

	releaseLock(file, context);
}

function rewriteUnique(path: string): void {
	writeLines(path, uniqueInOrder(readText(path)));
}

function writeLines(path: string, lines: readonly string[]): void {
	writeText(path, lines.length === 0 ? '' : `${lines.join('\n')}\n`);
}
