/**
 * The subset of grep the original preprocessor scripts used.
 *
 * Results are returned as arrays of lines. Callers that need grep's textual
 * output — because the original captured it into a shell variable — join them
 * with newlines, which is what {@link grepText} does.
 */

import { compile, type CompileOptions } from './regex.ts';
import { splitLines } from './sed.ts';

export interface GrepOptions extends CompileOptions {
	/** Keep the lines that do *not* match, as `-v` does. */
	invert?: boolean;
	/** Stop after this many matches, as `-m` does. */
	max?: number;
	/** Prefix each result with its 1-based line number, as `-n` does. */
	lineNumber?: boolean;
	/** Emit only the matched text rather than the whole line, as `-o` does. */
	onlyMatching?: boolean;
}

/** Runs grep over text and returns the matching lines. */
export function grep(text: string, pattern: string, options: GrepOptions = {}): string[] {
	const regex = compile(pattern, { ...options, global: options.onlyMatching === true });
	const { lines } = splitLines(text);
	const results: string[] = [];

	for (const [index, line] of lines.entries()) {
		if (options.max !== undefined && results.length >= options.max) {
			break;
		}

		// `-o` and `-v` are mutually exclusive in practice; `-o` wins here
		// because that is how the scripts use it.
		if (options.onlyMatching === true) {
			regex.lastIndex = 0;

			for (const match of line.matchAll(regex)) {
				// grep -o never reports empty matches.
				if (match[0] === '') {
					continue;
				}

				results.push(options.lineNumber === true ? `${index + 1}:${match[0]}` : match[0]);

				if (options.max !== undefined && results.length >= options.max) {
					break;
				}
			}

			continue;
		}

		if (regex.test(line) === (options.invert !== true)) {
			results.push(options.lineNumber === true ? `${index + 1}:${line}` : line);
		}
	}

	return results;
}

/**
 * Runs grep and returns its output the way a shell command substitution would
 * see it: lines joined by newlines, with no trailing newline.
 */
export function grepText(text: string, pattern: string, options: GrepOptions = {}): string {
	return grep(text, pattern, options).join('\n');
}

/** Whether any line matches, which is all a `if grep -q` test needs. */
export function grepMatches(text: string, pattern: string, options: GrepOptions = {}): boolean {
	return grep(text, pattern, { ...options, max: 1 }).length > 0;
}
