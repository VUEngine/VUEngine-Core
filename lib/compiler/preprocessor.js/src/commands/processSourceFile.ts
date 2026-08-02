/**
 * Port of processSourceFile.sh.
 *
 * Rewrites a Virtual C implementation file into plain C:
 *
 *   - `void Foo::bar(int x) { … }` becomes a free function that takes the
 *     instance as a hidden first parameter and opens with the assertions that
 *     recover the typed `this`;
 *   - `Foo::bar(…)` call sites become `Foo_bar(…)`, a cast to the class that
 *     actually owns the method, or a `__VIRTUAL_CALL` through the vtable,
 *     depending on what the dictionaries written by processHeaderFile say;
 *   - `new`, `delete`, `Base::method`, `friend class` and the singleton and
 *     allocator boilerplate expand into the engine's OOP macros;
 *   - the class definition macro and all the prototypes are injected ahead of
 *     the first method.
 *
 * The shell original worked by folding the file onto a single line, marking it
 * up with sentinels such as `<DECLARATION>` and `<START_BLOCK>`, transforming
 * it, and then unfolding it. That structure is kept here: the sentinels are the
 * only practical way to express "the brace that opens this method" in a
 * line-oriented rewrite, and reproducing them keeps the output identical.
 */

import { grep, grepText } from '../posix/grep.ts';
import { subst, substAll, substWhole } from '../posix/sed.ts';
import {
	capture,
	cut,
	cutFrom,
	expandUnquoted,
	sortUnique,
	splitFields,
	tailFrom,
	trDelete,
	uniqueInOrder,
	words,
} from '../posix/text.ts';
import {
	appendText,
	findAllByName,
	isFile,
	isNonEmptyFile,
	makeDirectoryExclusive,
	mkdirp,
	readText,
	removeFile,
	removeTree,
	sleep,
	writeText,
} from '../util/files.ts';
import { ExitSignal, flagOrEnvValue, flagValue, parseArguments } from '../util/shell.ts';

/** Sentinel marking where a line break used to be while the file is folded. */
const LINE_MARK = '@N@';

/** Sentinel appended to a line ending in a comma, i.e. a wrapped parameter list. */
const CONTINUATION_MARK = '<Â·>';

interface Settings {
	engineHome: string;
	inputFile: string;
	outputFile: string;
	workingFolder: string;
	classesHierarchyFile: string;
	printDebugOutput: boolean;
}

function readSettings(argv: readonly string[]): Settings {
	const parsed = parseArguments(argv, { valued: ['-e', '-i', '-o', '-w', '-c'], boolean: ['-d'] });

	return {
		engineHome: flagOrEnvValue(parsed, '-e', 'ENGINE_HOME'),
		inputFile: flagValue(parsed, '-i'),
		outputFile: flagValue(parsed, '-o'),
		workingFolder: flagValue(parsed, '-w', 'build/preprocessor'),
		classesHierarchyFile: flagOrEnvValue(parsed, '-c', 'CLASSES_HIERARCHY_FILE'),
		printDebugOutput: parsed.has('-d'),
	};
}

/** Entry point. Returns the process exit status. */
export function processSourceFile(argv: readonly string[]): number {
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
	const classesHierarchyFile = mergeClassesHierarchy(settings);
	const { inputFile, outputFile } = settings;

	if (inputFile === '') {
		console.log('Compiling error (1): no input file give');
		throw new ExitSignal(0);
	}

	if (!isFile(inputFile)) {
		console.log(`Compiling error (2): file not found ${inputFile}`);
		throw new ExitSignal(0);
	}

	let text = trDelete(readText(inputFile), '\r');

	// Pull a parameter list that was wrapped onto its own line back up, so a
	// method's signature and its opening brace end up in one pattern space.
	text = subst(text, '^[\t]\\+(', '(', { global: true });
	text = substWhole(text, '\n(', '(<NEW_LINE>', { global: true });

	writeText(outputFile, text);

	if (inputFile.includes('assets/')) {
		console.log(subst(expandUnquoted(inputFile), '^.*assets/\\(.*$\\)', 'Compiling asset: \\1', { global: true }));
		throw new ExitSignal(0);
	}

	const detected = detectClass(text);
	let className = detected.className;
	const { isExtensionClass, isMutationClass } = detected;

	// Add a space before every `Class::method`, mark every opening brace, and
	// mark lines that end in a comma so the wrapped ones can be rejoined.
	text = substAll(text, [
		['\\([A-z][A-z0-9]*::[a-z][A-z0-9]*\\)', ' \\1', { global: true }],
		['{', '{<START_BLOCK>', { global: true }],
		[',[ \t]*$', `,${CONTINUATION_MARK}`, { global: true }],
	]);

	text = joinContinuations(text);

	// Mark declarations carrying a modifier, then pad as the original did: the
	// two `echo`s appended a blank line each.
	text = subst(text, '.*static.*', '&<%>', { global: true }) + '\n';
	text = subst(text, '^secure[ \t].*', '&<#>', { global: true }) + '\n';

	text = markMethodDefinitions(text, className);

	// `<%>` sits at the end of the signature line, but the brace may be on the
	// next one; move the marker next to the brace. Then give every method the
	// hidden instance parameter.
	text = substAll(text, [
		['\\(<DECLARATION>[^<]*\\)<%>\\([^{]*\\)@N@{', `\\1${LINE_MARK}\\2<%>{`, { global: true }],
		[
			'\\(!DECLARATION_MIDDLE!_[^(]*\\)(\\([^%{]*{\\)',
			'\\1(void* _this __attribute__((unused)), \\2',
			{ global: true },
		],
		[',[ \t]*)', ')', { global: true }],
	]);

	// Distinguish the opening braces of static and secure methods.
	text = subst(text, '\\(<%>[^;!]*\\?{\\)\\(<START_BLOCK><method>\\)', '\\1<STATIC>\\2', { global: true });
	text = subst(text, '<#>\\([^;!]*\\?{\\)\\(<START_BLOCK><method>\\)', '\\1<SECURE>\\2', { global: true });

	// Method calls have to be read now, while declarations are still tagged and
	// can therefore be excluded.
	const unfolded = subst(text, LINE_MARK, '\\n', { global: true });
	const methodCalls = sortUnique(
		grep(
			subst(
				grep(grep(unfolded, '<DECLARATION>', { invert: true }).join('\n'), '::').join('\n'),
				'\\([A-Za-z0-9]*::[^(]*\\)(',
				'<\\1>\\n',
				{ global: true },
			),
			'<.*::.*>',
		).join('\n'),
	)
		.map((line) => subst(line, '.*<\\(.*\\)>', '\\1', { global: true }))
		.join('\n');

	const referencedClassesNames = `${className}\n${sortUnique(subst(methodCalls, '::.*', '', { global: true })).join('\n')}`;

	text = subst(text, '\\([A-Z][A-z0-9]*\\)::\\([a-z][A-z0-9]*\\)', '\\1_\\2', { global: true });

	const prototypes = extractPrototypes(text);

	text = subst(text, LINE_MARK, '\\n', { global: true });

	// Drop the empty first line the folding introduced.
	text = tailFrom(text, 2);

	text = injectThisPointer(text, className);

	const firstMethodDeclarationLine = Number(
		grepText(text, '^<DECLARATION>', { lineNumber: true, max: 1 }).split(':')[0] ?? '',
	);

	if (text === '') {
		writeText(outputFile, text);
		console.log(`Compiling error (3): could no processes file ${outputFile}`);
		throw new ExitSignal(0);
	}

	let fileName = className;

	if (className === '') {
		text = cleanUpMarkers(text, '');

		if (inputFile.includes('source')) {
			fileName = subst(expandUnquoted(inputFile), '^.*source[s]*/\\(.*$\\)', '\\1', { global: true });
		} else if (inputFile.includes('object')) {
			fileName = subst(expandUnquoted(inputFile), '^.*object[s]*/\\(.*$\\)', '\\1', { global: true });
		}

		text = subst(text, 'getInstance()', 'getInstance(NULL)', { global: true });
		console.log(`Compiling file: ${fileName}`);
	} else {
		// `\&` is a literal ampersand in a sed replacement; a bare `&` would
		// splice the matched text back in.
		text = subst(text, 'getInstance(', `getInstance((ClassPointer)\\&${className}_getBaseClass`, {
			global: true,
		});
	}

	if (text === '') {
		writeText(outputFile, text);
		console.log(` error (4): could no process file ${outputFile}`);
		throw new ExitSignal(0);
	}

	if (!isFile(classesHierarchyFile)) {
		writeText(outputFile, cleanUpMarkers(text, className));
		console.log(` error (5): no classes hierarchy file ${classesHierarchyFile}`);
		throw new ExitSignal(0);
	}

	const baseClassName = findBaseClassName(settings, classesHierarchyFile, className, isExtensionClass, isMutationClass);

	if (!inputFile.includes('source/')) {
		console.log(` error (7): ${inputFile} must be inside source folder`);
	}

	mkdirp(settings.workingFolder);

	// Move the declaration mark to the end of the line, so the call-site
	// substitutions below cannot mistake a declaration for a call.
	text = subst(text, '<DECLARATION>.*', '&<DECLARATION>', { global: true });

	text = translateMethodCalls(settings, {
		text,
		className,
		fileName,
		methodCalls,
		referencedClassesNames,
	});

	if (text === '') {
		writeText(outputFile, text);
		console.log(` error (8): could not processess file ${outputFile}`);
		throw new ExitSignal(0);
	}

	text = substAll(text, [
		['<%>', '', { global: true }],
		['<[%]*DECLARATION>[ \t]*\\(static\\|secure\\)[ \t][ \t]*', ' ', { global: true }],
		['<[%]*DECLARATION>', '', { global: true }],
		['<START_BLOCK>', '', { global: true }],
		['<method>.*<%method>', '', { global: true }],
	]);

	const built = buildClassDefinition({
		text,
		className,
		baseClassName,
		classesHierarchyFile,
		prototypes,
		isExtensionClass,
		isMutationClass,
	});

	text = built.text;

	if (text === '') {
		writeText(outputFile, text);
		console.log(` error (9): could not processess file ${outputFile}`);
		throw new ExitSignal(0);
	}

	if (Number.isInteger(firstMethodDeclarationLine) && firstMethodDeclarationLine > 0) {
		const classDefinition = trDelete(`/*CLASS_IN_FILE(${className})*/${built.classDefinition}`, '\r\n');
		const lines = text.split('\n');
		const index = firstMethodDeclarationLine - 1;

		if (lines[index] !== undefined) {
			lines[index] = classDefinition + lines[index];
			text = lines.join('\n');
		}
	}

	text = substAll(text, [
		['[ \t]*friend[ \t][ \t]*class[ \t][ \t]*\\([A-z0-9][A-z0-9]*\\)', '__CLASS_FRIEND_DEFINITION(\\1)'],
		['Base_constructor(\\(.*\\)', `__CONSTRUCT_BASE(${baseClassName},this,\\1`, { global: true }],
		[',[ \t]*);', ');'],
		['Base_destructor()', '__DESTROY_BASE', { global: true }],
		['Base_\\([A-z][A-z0-0][A-z0-0]*\\)(', `__CALL_BASE_METHOD(${baseClassName},\\1, `, { global: true }],
	]);

	text = subst(
		text,
		'\\([A-z][A-z0-0][A-z0-0]*\\)_mutateMethod(\\(.*\\), \\(.*\\))',
		'__CLASS_MUTATE_METHOD(\\1, \\2, \\3)',
		{ global: true },
	);

	text = substAll(text, [
		['[ \t]*\\(extension\\|mutation\\)[ \t][ \t]*class[ \t][ \t]*\\([A-z0-9][A-z0-9]*\\)', '__CLASS_FRIEND_DEFINITION(\\2)'],
		['Base_\\([A-z][A-z0-0][A-z0-0]*\\)(', `__CALL_BASE_METHOD(${baseClassName},\\1, `, { global: true }],
	]);

	text = cleanUpMarkers(text, className);

	// `new` and `delete` become the engine's allocation macros.
	text = substAll(text, [
		['\\([^A-z0-9]\\)new[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*(', '\\1\\2_new(', { global: true }],
		['\\([^A-z0-9]\\)delete[ \t][ \t]*\\(.*\\);', '\\1__DELETE(\\2);', { global: true }],
		['\\([^A-z0-9]\\)new[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*;', '\\1__NEW_BASIC(\\2);', { global: true }],
	]);

	writeText(outputFile, text);

	if (!isNonEmptyFile(outputFile)) {
		console.log(` error (10): could not processess file ${outputFile}`);
		throw new ExitSignal(0);
	}

	removeFile(`${outputFile}-e`);
}

/**
 * Folds this component's hierarchy file into the shared one, under a lock,
 * because every parallel compile job does the same thing.
 */
function mergeClassesHierarchy(settings: Settings): string {
	const generalFile = `${settings.workingFolder}/classes/hierarchies/classesHierarchy.txt`;
	const lockDirectory = `${settings.workingFolder}/classes/hierarchies/classesHierarchy.lock`;

	mkdirp(`${settings.workingFolder}/classes/hierarchies`);

	while (!makeDirectoryExclusive(lockDirectory)) {
		sleep(100);
	}

	try {
		appendText(generalFile, readText(settings.classesHierarchyFile));

		const unique = uniqueInOrder(readText(generalFile));
		writeText(generalFile, unique.length === 0 ? '' : `${unique.join('\n')}\n`);
	} finally {
		removeTree(lockDirectory);
	}

	return generalFile;
}

interface DetectedClass {
	className: string;
	isExtensionClass: boolean;
	isMutationClass: boolean;
}

/**
 * Works out which class this file implements. A destructor definition is the
 * reliable marker; failing that the file may declare itself an extension or a
 * mutation, or contain only static or secure methods.
 */
function detectClass(text: string): DetectedClass {
	let className = subst(
		grepText(text, '^.*::[ \t]*destructor[ \t]*(', { max: 1 }),
		'^.*[ \t][ \t]*\\([A-Z][A-z0-9]*\\)::.*',
		'\\1',
	);

	let isExtensionClass = false;
	let isMutationClass = false;

	if (className === '') {
		className = subst(
			grepText(text, '^extension[ \t][ \t]*class[ \t][ \t]*', { max: 1 }),
			'^extension[ \t][ \t]*class[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*;',
			'\\1',
		);

		if (className !== '') {
			isExtensionClass = true;
		} else {
			className = subst(
				grepText(text, '^mutation[ \t][ \t]*class[ \t][ \t]*', { max: 1 }),
				'^mutation[ \t][ \t]*class[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*;',
				'\\1',
			);

			if (className !== '') {
				isMutationClass = true;
			}
		}
	}

	if (className === '') {
		className = subst(
			grepText(text, '^\\(static\\|secure\\)[ \t]*.*[ \t][ \t]*[A-Z][A-z0-9]*[ \t]*::[ \t]*[a-z][A-z0-9]*[ \t]*[(]*', {
				max: 1,
				onlyMatching: true,
			}),
			'^.*[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*::.*',
			'\\1',
		);
	}

	return { className, isExtensionClass, isMutationClass };
}

/** Rejoins the lines that a wrapped parameter list was split across. */
function joinContinuations(text: string): string {
	const lines = text.endsWith('\n') ? text.slice(0, -1).split('\n') : text.split('\n');
	let out = '';

	for (const line of lines) {
		out += line.includes(CONTINUATION_MARK) ? `${line} ` : `${line}\n`;
	}

	return out;
}

/**
 * Folds the file onto one line and tags every `Type ClassName::method(…) {`
 * with the sentinels the later passes key off.
 */
function markMethodDefinitions(text: string, className: string): string {
	const folded = trDelete(subst(text, '.*', `${LINE_MARK}&`, { global: true }), '\r\n');

	return subst(
		folded,
		`${LINE_MARK}\\([ \t]*[A-z0-9_ \t]*[A-z0-9_\\*][A-z0-9_\\*]*[ \t][ \t]*${className}\\)[ \t]*::\\([ \t]*[a-z][A-z0-9]*[ \t]*\\)\\(([^{}]*{[ \t]*<START_BLOCK>\\)`,
		`${LINE_MARK}<DECLARATION>\\1!DECLARATION_MIDDLE!_\\2\\3<method>\\2<%method><%DECLARATION>`,
		{ global: true },
	);
}

/** Derives the file's prototypes from its tagged method definitions. */
function extractPrototypes(text: string): string {
	let prototypes = subst(text, '<DECLARATION>', '\\n<DECLARATION>', { global: true });
	prototypes = subst(prototypes, '<%DECLARATION>', '<%DECLARATION>\\n', { global: true });
	prototypes = grep(prototypes, 'DECLARATION>').join('\n');
	prototypes = subst(prototypes, '<[%]*DECLARATION>', '', { global: true });
	prototypes = subst(prototypes, '{<START_BLOCK>.*<method>.*<%method>', ';', { global: true });
	prototypes = subst(prototypes, '{<\\(STATIC\\|SECURE\\)><START_BLOCK>.*<method>.*<%method>', ';', { global: true });
	prototypes = subst(prototypes, LINE_MARK, '', { global: true });
	prototypes = subst(prototypes, '<%>', '', { global: true });
	prototypes = trDelete(prototypes, '\r\n');

	return subst(prototypes, '\\([^A-z0-9]*\\)\\(static\\|secure\\)[ \t]', '\\1 ', { global: true });
}

/**
 * Opens every method body with the assertions that turn the untyped `_this`
 * into a checked, typed `this`. Secure methods additionally assert that the
 * caller was authorised.
 */
function injectThisPointer(text: string, className: string): string {
	const guard =
		`{__CHECK_STACK_STATUS NM_ASSERT(!isDeleted(_this), "${className}::\\2: null this"); ` +
		`${className} this __attribute__((unused)) = __SAFE_CAST(${className} , _this); ` +
		`ASSERT(!isDeleted(this), "${className}::\\2: this failed the cast");`;

	return substAll(text, [
		['<%>[ \t]*{[ \t]*<START_BLOCK>', '{', { global: true }],
		['{[ \t]*<SECURE>[ \t]*<START_BLOCK>', '{<START_SECURE_BLOCK>', { global: true }],
		['{[ \t]*<START_BLOCK>\\(.*\\)<method>\\(.*\\)<%method><%DECLARATION>', `${guard}\\1`, { global: true }],
		[
			'{[ \t]*<START_SECURE_BLOCK>\\(.*\\)<method>\\(.*\\)<%method><%DECLARATION>',
			`${guard} NM_ASSERT(_authorized, "${className}::\\2: unauthorized access");\\1`,
			{ global: true },
		],
	]);
}

/** Strips the working sentinels and expands the casts they guarded. */
function cleanUpMarkers(text: string, className: string): string {
	void className;

	let out = substAll(text, [
		['<%>', '', { global: true }],
		['<[%]*DECLARATION>[ \t]*\\(static\\|secure\\)[ \t][ \t]*', ' ', { global: true }],
		['<[%]*DECLARATION>', '', { global: true }],
		['!DECLARATION_MIDDLE!', '', { global: true }],
		['\\([A-Z][A-z0-9]*\\)::\\([a-z][A-z0-9]*\\)', '\\1_\\2', { global: true }],
		['<START_BLOCK>', '', { global: true }],
		[`,${CONTINUATION_MARK}`, ',\\n', { global: true }],
	]);

	out = subst(out, '\\([A-Z][A-z0-9]*\\)_safeCast[ \t]*(', '__SAFE_CAST(\\1, ', { global: true });
	out = subst(out, '\\([A-Z][A-z0-9]*\\)_overrides[ \t]*(', '__OVERRIDES_METHOD(\\1, ', { global: true });

	return substAll(out, [
		['(<NEW_LINE>', '\\n(', { global: true }],
		['<NEW_LINE>', '\\n', { global: true }],
		['<STATIC>', '', { global: true }],
		['<SECURE>', '', { global: true }],
	]);
}

/** Looks the class' base up in the hierarchy files. */
function findBaseClassName(
	settings: Settings,
	classesHierarchyFile: string,
	className: string,
	isExtensionClass: boolean,
	isMutationClass: boolean,
): string {
	if (!isExtensionClass && !isMutationClass) {
		return cut(grepText(readText(classesHierarchyFile), `^${className}:`, { max: 1 }), ':', [2]);
	}

	// An extension is compiled on its own, so its class may only be recorded in
	// another component's hierarchy file.
	for (const file of findAllByName(`${settings.workingFolder}/classes/hierarchies`, 'classesHierarchy.txt')) {
		const baseClassName = cut(grepText(readText(file), `^${className}:`, { max: 1 }), ':', [2]);

		if (baseClassName !== '') {
			return baseClassName;
		}
	}

	return '';
}

/**
 * Rewrites the file's `Class_method(` call sites using the dictionaries the
 * header pass wrote: inherited methods gain a cast to the owning class, virtual
 * ones become dispatches through the vtable.
 */
function translateMethodCalls(
	settings: Settings,
	input: {
		text: string;
		className: string;
		fileName: string;
		methodCalls: string;
		referencedClassesNames: string;
	},
): string {
	const dictionaries = `${settings.workingFolder}/classes/dictionaries`;
	const normalMethodsFile = `${dictionaries}/${input.fileName}MethodsOwnedToApply.txt`;
	const virtualMethodsFile = `${dictionaries}/${input.fileName}MethodsVirtualToApply.txt`;

	removeFile(normalMethodsFile);
	removeFile(virtualMethodsFile);

	let normalExists = false;
	let virtualExists = false;

	for (const referencedClassName of words(input.referencedClassesNames)) {
		const referencedNormal = `${dictionaries}/${referencedClassName}MethodsOwned.txt`;
		const referencedVirtual = `${dictionaries}/${referencedClassName}MethodsVirtual.txt`;

		// A BRE alternation of every method this file calls on that class. The
		// dummy keeps the pattern non-empty when the file calls none of them.
		const referencedMethodNames =
			trDelete(
				subst(
					subst(grep(input.methodCalls, referencedClassName).join('\n'), '::', '_', { global: true }),
					'$',
					'\\\\|',
					{ global: true },
				),
				'\r\n',
			) + 'DUMMY_METHOD_NAME';

		if (isFile(referencedNormal)) {
			normalExists = true;
			appendLinesTo(normalMethodsFile, grep(readText(referencedNormal), referencedMethodNames));
		}

		if (isFile(referencedVirtual)) {
			virtualExists = true;
			appendLinesTo(virtualMethodsFile, grep(readText(referencedVirtual), referencedMethodNames));
		}
	}

	void virtualExists;
	let text = input.text;

	if (normalExists) {
		const normal = sortUnique(readText(normalMethodsFile));
		writeLines(normalMethodsFile, normal);

		// The original sorted the virtual file inside this branch too, which
		// also created it when it did not exist.
		const virtual = sortUnique(readText(virtualMethodsFile));
		writeLines(virtualMethodsFile, virtual);

		console.log(`Compiling class: ${input.className} (complexity: ${normal.length + virtual.length})`);

		if (normal.length > 0) {
			text = applyMethodDictionary(text, readText(normalMethodsFile), 'normal');
			removeFile(normalMethodsFile);
		}
	}

	if (isFile(virtualMethodsFile)) {
		const virtual = readText(virtualMethodsFile);

		if (capture(virtual) !== '') {
			text = applyMethodDictionary(text, virtual, 'virtual');
			removeFile(virtualMethodsFile);
		}
	}

	return text;
}

/**
 * Port of normalMethodTraduction.awk and virtualMethodTraduction.awk.
 *
 * Each dictionary line is `<call site> <replacement>`. A normal replacement
 * also opens a cast to the class that owns the method, so
 * `Actor_update(this, x)` becomes `Container_update((Container)this, x)`.
 *
 * The awk original iterated the dictionary in hash order; the keys are applied
 * in sorted order here so a build is reproducible.
 */
function applyMethodDictionary(text: string, dictionary: string, kind: 'normal' | 'virtual'): string {
	const replacements = new Map<string, string>();

	for (const line of splitFields(dictionary)) {
		const [key, value] = line;

		if (key !== undefined && value !== undefined) {
			replacements.set(key, value);
		}
	}

	let out = text;

	for (const key of [...replacements.keys()].sort()) {
		const value = replacements.get(key)!;
		const replacement =
			kind === 'virtual' ? ` ${value}` : ` ${value}((${value.slice(0, value.indexOf('_'))})`;

		out = subst(out, `[ \t][ \t]*${key}[ \t]*[(]`, replacement.replace(/\\/g, '\\\\').replace(/&/g, '\\&'), {
			global: true,
		});
	}

	return out;
}

interface ClassDefinitionInput {
	text: string;
	className: string;
	baseClassName: string;
	classesHierarchyFile: string;
	prototypes: string;
	isExtensionClass: boolean;
	isMutationClass: boolean;
}

/**
 * Builds the `__CLASS_DEFINITION` block, the allocator and, for singletons, the
 * instance holder that go in ahead of the first method.
 */
function buildClassDefinition(input: ClassDefinitionInput): { text: string; classDefinition: string } {
	const { className, baseClassName, prototypes } = input;
	let text = input.text;

	let classModifiers = subst(
		grepText(readText(input.classesHierarchyFile), `^${className}:`, { max: 1 }),
		'^.*::\\(.*\\)',
		'\\1',
		{ global: true },
	);

	if (classModifiers === '') {
		classModifiers = 'normal';
	}

	if (input.isExtensionClass) {
		return { text, classDefinition: prototypes };
	}

	if (classModifiers.includes('static ')) {
		return {
			text,
			classDefinition: `__CLASS_FUNDAMENTAL_DEFINITION(${className}, ${baseClassName}) ${prototypes}`,
		};
	}

	let classDefinition = input.isMutationClass
		? `__MUTATION_CLASS_DEFINITION(${className}, ${baseClassName}) ${prototypes}`
		: `__CLASS_DEFINITION(${className}, ${baseClassName}) ${prototypes}`;

	const isPlainClass =
		!classModifiers.includes('singleton') &&
		!classModifiers.includes('static ') &&
		!classModifiers.includes('abstract ') &&
		!classModifiers.includes('mutation ');

	if (isPlainClass) {
		const constructor = grepText(text, `${className}!DECLARATION_MIDDLE!_constructor[ \t]*(.*)`, { max: 1 });

		let constructorParameters = subst(constructor, '__attribute__ *\\(\\([a-z]+\\)\\) *', '', {
			extended: true,
			global: true,
		});
		constructorParameters = subst(constructorParameters, '^.*(\\(.*\\))[ \t{]*$', '\\1');

		let allocatorParameters = cutFrom(`${constructorParameters},`, ',', 2);
		const allocatorArguments = subst(
			trDelete(
				subst(
					subst(allocatorParameters, '[ \t*][ \t*]*\\([A-z0-9][A-z0-9]*[ \t]*,\\)', '<\\1>\\n', { global: true }),
					'.*<\\(.*\\)>.*',
					'\\1',
					{ global: true },
				),
				'\r\n',
			),
			'\\(.*\\),',
			'\\1',
		);

		allocatorParameters = subst(allocatorParameters, '\\(.*\\),', '\\1');

		classDefinition +=
			allocatorParameters === ''
				? `__CLASS_NEW_DEFINITION(${className}, void)__CLASS_NEW_END(${className}, this);`
				: `__CLASS_NEW_DEFINITION(${className}, ${allocatorParameters})__CLASS_NEW_END(${className}, this, ${allocatorArguments});`;

		return { text, classDefinition };
	}

	if (classModifiers.includes('singleton')) {
		const customSingletonDefinition = grepText(text, '#define[ \t][ \t]*.*SINGLETON.*(', {
			max: 1,
			onlyMatching: true,
		});

		if (customSingletonDefinition === '') {
			if (classModifiers.includes('dynamic_singleton')) {
				classDefinition += `__SINGLETON_DYNAMIC(${className});`;
			} else if (className.includes('MemoryPool')) {
				classDefinition += `__SINGLETON(${className}, __MEMORY_POOL_SECTION_ATTRIBUTE);`;
			} else {
				classDefinition += `__SINGLETON(${className}, __STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE);`;
			}
		} else {
			classDefinition += `${subst(expandUnquoted(customSingletonDefinition), '^.*[ \t][ \t]*\\(.*SINGLETON.*\\)(', '\\1')}(${className});`;
		}

		text = subst(
			text,
			'Base_destructor();',
			'_singletonConstructed = __SINGLETON_NOT_CONSTRUCTED; Base_destructor();',
		);
	}

	return { text, classDefinition };
}

function appendLinesTo(path: string, lines: readonly string[]): void {
	if (lines.length > 0) {
		appendText(path, `${lines.join('\n')}\n`);
	} else {
		appendText(path, '');
	}
}

function writeLines(path: string, lines: readonly string[]): void {
	writeText(path, lines.length === 0 ? '' : `${lines.join('\n')}\n`);
}
