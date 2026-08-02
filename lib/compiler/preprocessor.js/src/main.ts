/**
 * Command-line entry point for the Virtual C preprocessor.
 *
 * One bundle serves every command so that the hundreds of per-file invocations
 * a build makes pay Node's start-up cost only, with no module resolution. The
 * `.sh` files beside this bundle are thin shims that forward to it, which is
 * what keeps the makefiles unchanged.
 */

import { commandLineArguments, setExitCode, writeStandardError } from './platform/platform.ts';
import { printCompilingInfo } from './commands/printCompilingInfo.ts';
import { processGCCOutput } from './commands/processGCCOutput.ts';
import { processHeaderFile } from './commands/processHeaderFile.ts';
import { processSourceFile } from './commands/processSourceFile.ts';
import { setupClasses } from './commands/setupClasses.ts';

const COMMANDS: Record<string, (argv: readonly string[]) => number> = {
	header: processHeaderFile,
	source: processSourceFile,
	'setup-classes': setupClasses,
	'gcc-output': processGCCOutput,
	'compiling-info': printCompilingInfo,
};

function main(): number {
	const [command, ...rest] = commandLineArguments();

	if (command === undefined || !(command in COMMANDS)) {
		writeStandardError(`Usage: preprocessor <${Object.keys(COMMANDS).join('|')}> [options]\n`);

		return 1;
	}

	return COMMANDS[command]!(rest);
}

setExitCode(main());
