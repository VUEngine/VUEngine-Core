/**
 * Port of processGCCOutput.sh.
 *
 * The compiler sees the preprocessed copies under build/working, so its
 * diagnostics point there. This rewrites those paths back to the sources the
 * developer actually edits, and on WSL converts them to Windows form so editors
 * can follow them.
 */

import { subst } from '../posix/sed.ts';
import { readStandardInput, standardInputIsPipe, writeStandardOutput } from '../platform/platform.ts';
import { isDirectory, readText, removeFile, writeText } from '../util/files.ts';
import { ExitSignal, flagOrEnvValue, parseArguments } from '../util/shell.ts';

/** Entry point. Returns the process exit status. */
export function processGCCOutput(argv: readonly string[]): number {
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
	const parsed = parseArguments(argv, { valued: ['-o', '-w', '-p', '-l', '-n', '-h'] }, 'ignore-last');

	const outputFile = flagOrEnvValue(parsed, '-o', 'OUTPUT_FILE');
	const workingFolder = flagOrEnvValue(parsed, '-w', 'WORKING_FOLDER');
	const plugins = flagOrEnvValue(parsed, '-p', 'PLUGINS');
	const pluginsPath = flagOrEnvValue(parsed, '-l', 'PLUGINS_PATH');
	const name = flagOrEnvValue(parsed, '-n', 'NAME');
	const nameHome = flagOrEnvValue(parsed, '-h', 'NAME_HOME');

	if (!standardInputIsPipe()) {
		throw new ExitSignal(0);
	}

	const gccOutput = `${outputFile}.out`;
	const piped = readStandardInput();

	if (piped === '') {
		throw new ExitSignal(0);
	}

	writeText(gccOutput, piped);

	if (!isDirectory(workingFolder)) {
		removeFile(gccOutput);
		throw new ExitSignal(0);
	}

	if (plugins === '' && name === '' && nameHome === '') {
		removeFile(gccOutput);
		throw new ExitSignal(0);
	}

	let text = readText(gccOutput);

	for (const plugin of plugins.split(/\s+/).filter((entry) => entry !== '')) {
		// The original interpolated an unset `$objects` here, leaving the double
		// slash below. Kept so the rewriting behaves identically.
		text = subst(text, '^.*build/working/objects/[a-z][a-z]*//', `${pluginsPath}/${plugin}/`, { global: true });
	}

	text = subst(text, `^.*build/working/objects/[a-z][a-z]*/${name}`, nameHome, { global: true });

	if (pluginsPath.includes('/mnt/')) {
		text = subst(text, '/mnt/([A-z]+)/', '\\1:/', { extended: true, global: true });
		text = subst(text, '/', '\\\\', { global: true });
	}

	writeStandardOutput(text);
	writeStandardOutput('\n');
	removeFile(gccOutput);
}

