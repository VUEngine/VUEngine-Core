/**
 * The remaining line-oriented shell tools the original scripts leaned on: cut,
 * tr, head, tail, wc, sort and awk's deduplication idiom.
 */

import { joinLines, splitLines } from './sed.ts';

/**
 * `cut -d<delimiter> -f<fields>`. A line without the delimiter is passed
 * through untouched, which is what the class-declaration parsing relies on.
 */
export function cut(text: string, delimiter: string, fields: readonly number[]): string {
	const { lines } = splitLines(text);

	return lines
		.map((line) => {
			if (!line.includes(delimiter)) {
				return line;
			}

			const parts = line.split(delimiter);

			return fields
				.filter((field) => field <= parts.length)
				.map((field) => parts[field - 1]!)
				.join(delimiter);
		})
		.join('\n');
}

/**
 * `cut -d<delimiter> -f<from>-`, taking every field from `from` onwards.
 */
export function cutFrom(text: string, delimiter: string, from: number): string {
	const { lines } = splitLines(text);

	return lines
		.map((line) => (line.includes(delimiter) ? line.split(delimiter).slice(from - 1).join(delimiter) : line))
		.join('\n');
}

/** `tr -d <characters>`, deleting every occurrence of any of them. */
export function trDelete(text: string, characters: string): string {
	const removed = new Set(characters.split(''));

	return text
		.split('')
		.filter((char) => !removed.has(char))
		.join('');
}

/** `head -<count>`, keeping the first lines. */
export function head(text: string, count: number): string {
	if (count <= 0) {
		return '';
	}

	const split = splitLines(text);

	if (count >= split.lines.length) {
		return joinLines(split);
	}

	// head always terminates the lines it emits.
	return joinLines({ lines: split.lines.slice(0, count), trailingNewline: true });
}

/** `tail -<count>`, keeping the last lines. */
export function tail(text: string, count: number): string {
	if (count <= 0) {
		return '';
	}

	const split = splitLines(text);

	return joinLines({ ...split, lines: split.lines.slice(Math.max(0, split.lines.length - count)) });
}

/** `tail -n +<start>`, keeping everything from a 1-based line onwards. */
export function tailFrom(text: string, start: number): string {
	const split = splitLines(text);

	return joinLines({ ...split, lines: split.lines.slice(Math.max(0, start - 1)) });
}

/** `sed '<from>,<to>!d'`, keeping an inclusive 1-based range of lines. */
export function selectRange(text: string, from: number, to: number): string {
	const { lines } = splitLines(text);

	return lines.slice(Math.max(0, from - 1), to).join('\n');
}

/** `wc -l`, which counts newlines rather than lines. */
export function countLines(text: string): number {
	let count = 0;

	for (const char of text) {
		if (char === '\n') {
			count++;
		}
	}

	return count;
}

/** `sort -u`, in the byte order the C locale sorts by. */
export function sortUnique(text: string): string[] {
	const { lines } = splitLines(text);

	return [...new Set(lines)].sort((a, b) => (a < b ? -1 : a > b ? 1 : 0));
}

/** `awk '!x[$0]++'`, dropping repeats but keeping the first occurrences in order. */
export function uniqueInOrder(text: string): string[] {
	const { lines } = splitLines(text);

	return [...new Set(lines)];
}

/**
 * Splits on whitespace the way an unquoted shell expansion does, so that
 * iterating over a captured list of paths or names behaves as it did.
 */
export function words(text: string): string[] {
	return text.split(/[ \t\n]+/).filter((word) => word !== '');
}

/**
 * Splits each line into awk's whitespace-separated fields, skipping the blank
 * lines awk would produce no fields for.
 */
export function splitFields(text: string): string[][] {
	return splitLines(text)
		.lines.map((line) => line.split(/[ \t]+/).filter((field) => field !== ''))
		.filter((fields) => fields.length > 0);
}

/**
 * Trims trailing newlines the way a shell command substitution does. Every
 * value the original scripts captured with backticks went through this.
 */
export function capture(text: string): string {
	return text.replace(/\n+$/, '');
}

/**
 * Reproduces an *unquoted* shell expansion, as in `cmd <<< $variable`: the
 * value is split on whitespace and rejoined with single spaces, so runs of
 * spaces and tabs collapse.
 *
 * This is not a detail worth preserving on its own, but it is observable. A
 * constructor written `constructor(Spec* spec,  int16 id)` — with two spaces —
 * yields a generated allocator with only one, because the original script left
 * that expansion unquoted.
 */
export function expandUnquoted(text: string): string {
	return words(text).join(' ');
}
