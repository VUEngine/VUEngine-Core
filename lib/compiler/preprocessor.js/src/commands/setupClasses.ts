/**
 * Port of setupClasses.sh.
 *
 * Generates the C file that installs every class' vtable at start-up. One is
 * produced per component (the engine, each plugin, the game), plus a top-level
 * `setupClasses.c` that calls all of them.
 */

import { grep } from '../posix/grep.ts';
import { subst } from '../posix/sed.ts';
import { findAllByName, isDirectory, isFile, readText, removeFile, writeText } from '../util/files.ts';
import { ExitSignal, flagOrEnvValue, flagValue, parseArguments } from '../util/shell.ts';

/** Entry point. Returns the process exit status. */
export function setupClasses(argv: readonly string[]): number {
	try {
		run(argv);

		return 0;
	} catch (error) {
		if (error instanceof ExitSignal) {
			return error.code;
		}

		throw error;
	}
}

function run(argv: readonly string[]): void {
	const parsed = parseArguments(argv, { valued: ['-n', '-o', '-w', '-b', '-c'] }, 'ignore-last');

	const plugin = flagValue(parsed, '-n', 'LibraryName');
	const setupFunction = subst(flagValue(parsed, '-n', 'ClassName'), '[^[:alnum:]\t]', '', { global: true });
	const outputCFile = flagValue(parsed, '-o', 'setupClasses.c');
	const workingFolder = flagValue(parsed, '-w', 'build/working');
	const buildMode = flagOrEnvValue(parsed, '-b', 'BUILD_MODE');
	const classesHierarchyFile = flagValue(parsed, '-c', `${workingFolder}/classesHierarchy.txt`);

	if (!isDirectory(workingFolder)) {
		throw new ExitSignal(0);
	}

	if (classesHierarchyFile === '') {
		console.log(`Setting up plugin: ${plugin}`);

		return;
	}

	// Static classes have no vtable, so they are left out.
	const classNames = isFile(classesHierarchyFile)
		? grep(readText(classesHierarchyFile), ':.*static.*', { invert: true })
				.map((line) => subst(line, ':.*', '', { global: true }))
				.filter((name) => name !== '')
		: [];

	let output = '// Do not modify this file, it is auto-generated\n';
	output += ' \n';
	output += '// includes\n';

	for (const className of classNames) {
		output += `#include <${className}.h>\n`;
	}

	output += ' \n';
	output += '// setup function\n';
	output += `void ${setupFunction}(void)\n`;
	output += '{\n';

	for (const className of classNames) {
		output += `\t${className}_setVTable(true);\n`;
	}

	output += '}\n';

	writeText(outputCFile, output);

	// Now the aggregate file that calls every component's setup function.
	const finalSetupClassesFile = `${workingFolder}/objects/${buildMode}/setupClasses.c`;
	removeFile(finalSetupClassesFile);

	const setupClassesFiles = findAllByName(`${workingFolder}/objects/${buildMode}`, (fileName) =>
		fileName.endsWith('SetupClasses.c'),
	);

	const forwardDeclarations: string[] = [];
	let calls = '';

	for (const setupClassFile of setupClassesFiles) {
		const setupFunctionName = subst(
			grep(readText(setupClassFile), 'SetupClasses').join('\n'),
			'.*void[ \t]*\\(.*SetupClasses\\)(.*',
			'\\1',
			{ global: true },
		);

		calls += `\t${setupFunctionName}();\n`;
		// The original prepended each declaration, which reverses their order.
		forwardDeclarations.unshift(`void ${setupFunctionName}(void);`);
	}

	let finalOutput = forwardDeclarations.length === 0 ? '' : `${forwardDeclarations.join('\n')}\n`;
	finalOutput += '// setup function\n';
	finalOutput += 'void setupClasses(void)\n';
	finalOutput += '{\n';
	finalOutput += calls;
	finalOutput += '}\n';

	writeText(finalSetupClassesFile, finalOutput);

	console.log(`Setting up plugin: ${plugin}`);
}
