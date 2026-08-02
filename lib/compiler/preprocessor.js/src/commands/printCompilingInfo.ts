/**
 * Port of printCompilingInfo.sh.
 *
 * Prints the progress line for a file the makefile compiles directly, with the
 * uninteresting leading path folded away.
 */

import { subst } from '../posix/sed.ts';

/** Entry point. Returns the process exit status. */
export function printCompilingInfo(argv: readonly string[]): number {
	const inputFile = argv[0] ?? '';
	let message = `Compiling file:  ${inputFile}`;

	if (inputFile.includes('assets/')) {
		message = subst(inputFile, '^.*assets/\\(.*$\\)', 'Compiling asset: \\1', { global: true });
	}

	if (inputFile.includes('source')) {
		message = subst(inputFile, '^.*source[s]*/\\(.*$\\)', 'Compiling file:  \\1', { global: true });
	} else if (inputFile.includes('object')) {
		message = subst(inputFile, '^.*object[s]*/\\(.*$\\)', 'Compiling file:  \\1', { global: true });
	}

	console.log(message);

	return 0;
}
