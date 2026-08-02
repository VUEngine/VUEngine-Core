/**
 * The subset of sed the original preprocessor scripts used.
 *
 * sed reads one line at a time into its pattern space, so a substitution's `^`
 * and `$` anchor to line boundaries and a `.` never crosses one. That is
 * reproduced here by splitting the text, transforming each line and joining it
 * back, rather than by leaning on JavaScript's multiline flag, which differs
 * subtly around the final line of a file.
 */

import { compile, type CompileOptions } from './regex.ts';

export interface Lines {
	lines: string[];
	/** Whether the text ended with a newline, which is not a line of its own. */
	trailingNewline: boolean;
}

/** Splits text the way a line-oriented tool sees it. */
export function splitLines(text: string): Lines {
	if (text === '') {
		return { lines: [], trailingNewline: false };
	}

	if (text.endsWith('\n')) {
		return { lines: text.slice(0, -1).split('\n'), trailingNewline: true };
	}

	return { lines: text.split('\n'), trailingNewline: false };
}

/** Reassembles what {@link splitLines} took apart. */
export function joinLines({ lines, trailingNewline }: Lines): string {
	if (lines.length === 0) {
		return trailingNewline ? '\n' : '';
	}

	return lines.join('\n') + (trailingNewline ? '\n' : '');
}

type ReplacementPart =
	| { kind: 'literal'; text: string }
	| { kind: 'group'; index: number }
	| { kind: 'match' };

const replacementCache = new Map<string, ReplacementPart[]>();

/**
 * Parses a sed replacement into parts. Done ahead of time so the substitution
 * can use a replacer function and never has to worry about JavaScript's own `$`
 * escapes leaking in from the replacement text.
 */
function parseReplacement(replacement: string): ReplacementPart[] {
	const cached = replacementCache.get(replacement);

	if (cached !== undefined) {
		return cached;
	}

	const parts: ReplacementPart[] = [];
	let literal = '';

	const flushLiteral = (): void => {
		if (literal !== '') {
			parts.push({ kind: 'literal', text: literal });
			literal = '';
		}
	};

	for (let i = 0; i < replacement.length; i++) {
		const char = replacement[i]!;

		if (char === '&') {
			flushLiteral();
			parts.push({ kind: 'match' });
			continue;
		}

		if (char === '\\') {
			const next = replacement[i + 1];

			if (next === undefined) {
				literal += '\\';
				continue;
			}

			i++;

			if (next >= '0' && next <= '9') {
				const index = Number(next);
				flushLiteral();
				parts.push(index === 0 ? { kind: 'match' } : { kind: 'group', index });
				continue;
			}

			// GNU sed's control-character escapes. Unknown escapes stand for the
			// escaped character itself, which also covers `\&` and `\\`.
			literal += next === 'n' ? '\n' : next === 't' ? '\t' : next === 'r' ? '\r' : next;
			continue;
		}

		literal += char;
	}

	flushLiteral();
	replacementCache.set(replacement, parts);

	return parts;
}

/** A substitution compiled once and then applied to many lines. */
interface CompiledSubstitution {
	regex: RegExp;
	parts: ReplacementPart[];
	global: boolean;
}

function expand(parts: ReplacementPart[], match: RegExpExecArray): string {
	let out = '';

	for (const part of parts) {
		if (part.kind === 'literal') {
			out += part.text;
		} else if (part.kind === 'match') {
			out += match[0];
		} else {
			out += match[part.index] ?? '';
		}
	}

	return out;
}

function applyCompiled(line: string, { regex, parts, global }: CompiledSubstitution): string {
	regex.lastIndex = 0;

	if (!global) {
		const match = regex.exec(line);

		return match === null
			? line
			: line.slice(0, match.index) + expand(parts, match) + line.slice(match.index + match[0].length);
	}

	// sed's `s///g` loop, which differs from JavaScript's `replace` in one
	// respect that matters: a null match starting exactly where the previous
	// match ended is discarded rather than replaced. Without that rule
	// `s/.*/X/g` would yield "XX" for a line rather than sed's "X".
	let result = '';
	let position = 0;
	let previousMatchEnd = -1;

	while (position <= line.length) {
		regex.lastIndex = position;

		const match = regex.exec(line);

		if (match === null) {
			break;
		}

		const start = match.index;
		const end = start + match[0].length;

		if (start === end && start === previousMatchEnd) {
			if (start >= line.length) {
				break;
			}

			result += line[start];
			position = start + 1;
			continue;
		}

		result += line.slice(position, start) + expand(parts, match);
		previousMatchEnd = end;

		if (start === end) {
			// An empty match cannot advance on its own, so copy a character.
			if (end >= line.length) {
				position = end;
				break;
			}

			result += line[end];
			position = end + 1;
		} else {
			position = end;
		}
	}

	return result + line.slice(position);
}

export interface SubstOptions extends Omit<CompileOptions, 'global'> {
	/** Replace every occurrence in the line rather than only the first. */
	global?: boolean;
}

function compileSubstitution(
	pattern: string,
	replacement: string,
	options: SubstOptions,
): CompiledSubstitution {
	return {
		regex: compile(pattern, { ...options, global: options.global === true }),
		parts: parseReplacement(replacement),
		global: options.global === true,
	};
}

/** Applies `s/pattern/replacement/` to a single line. */
export function substLine(
	line: string,
	pattern: string,
	replacement: string,
	options: SubstOptions = {},
): string {
	return applyCompiled(line, compileSubstitution(pattern, replacement, options));
}

/** Applies `s/pattern/replacement/` to every line, as sed's default cycle does. */
export function subst(
	text: string,
	pattern: string,
	replacement: string,
	options: SubstOptions = {},
): string {
	return substAll(text, [[pattern, replacement, options]]);
}

/**
 * Applies a substitution to the whole text at once, the way `sed -z` does when
 * the file has no NUL bytes. Needed where the original scripts matched across
 * line boundaries.
 */
export function substWhole(
	text: string,
	pattern: string,
	replacement: string,
	options: SubstOptions = {},
): string {
	return substLine(text, pattern, replacement, options);
}

/** Runs several substitutions in order, as a `s/…/…/; s/…/…/` script does. */
export function substAll(
	text: string,
	substitutions: readonly (readonly [pattern: string, replacement: string, options?: SubstOptions])[],
): string {
	const compiled = substitutions.map(([pattern, replacement, options]) =>
		compileSubstitution(pattern, replacement, options ?? {}),
	);
	const split = splitLines(text);

	return joinLines({
		...split,
		lines: split.lines.map((line) => {
			let current = line;

			for (const substitution of compiled) {
				current = applyCompiled(current, substitution);
			}

			return current;
		}),
	});
}

/** Deletes the lines matching a pattern, as `/pattern/d` does. */
export function deleteLines(text: string, pattern: string, options: CompileOptions = {}): string {
	const regex = compile(pattern, options);
	const split = splitLines(text);

	return joinLines({ ...split, lines: split.lines.filter((line) => !regex.test(line)) });
}
