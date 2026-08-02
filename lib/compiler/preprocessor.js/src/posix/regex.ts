/**
 * Translation of POSIX regular expressions into JavaScript ones.
 *
 * The shell scripts this package replaces drove sed, grep and awk, all of which
 * speak POSIX regular expressions rather than JavaScript ones. The two dialects
 * differ in ways that silently change what a pattern matches, so the patterns
 * are kept verbatim from the original scripts and translated here instead of
 * being hand-converted. The differences that actually bite:
 *
 *   - In a Basic Regular Expression, `(`, `)`, `{`, `}`, `|`, `+` and `?` are
 *     literals, and the backslashed forms are the operators. Extended Regular
 *     Expressions invert that.
 *   - Inside a bracket expression a backslash is an ordinary character, so
 *     `[^\*]` excludes backslash *and* asterisk, where JavaScript reads it as
 *     excluding only the asterisk.
 *   - `^` and `$` are anchors only at the edges of a BRE; elsewhere they are
 *     literal characters.
 *   - Named classes such as `[:alnum:]` have no JavaScript equivalent.
 *
 * The GNU extensions that GNU sed and grep accept in BREs (`\+`, `\?`, `\|`,
 * `\n`, `\t`, `\w`, `\<`, `\>`) are supported too, because the original scripts
 * rely on them.
 */

/** Characters that must be escaped to be matched literally in JavaScript. */
const JS_SPECIAL = new Set('^$\\.*+?()[]{}|/'.split(''));

/** POSIX named character classes, mapped to JavaScript character-class bodies. */
const NAMED_CLASSES: Record<string, string> = {
	alpha: 'a-zA-Z',
	digit: '0-9',
	alnum: 'a-zA-Z0-9',
	upper: 'A-Z',
	lower: 'a-z',
	space: ' \\t\\n\\r\\f\\v',
	blank: ' \\t',
	punct: '!-/:-@\\[-`{-~',
	print: ' -~',
	graph: '!-~',
	cntrl: '\\x00-\\x1f\\x7f',
	xdigit: '0-9A-Fa-f',
	word: 'A-Za-z0-9_',
};

/** Escapes a single character so JavaScript matches it literally. */
export function escapeLiteral(char: string): string {
	return JS_SPECIAL.has(char) ? `\\${char}` : char;
}

/** Escapes a whole string for literal matching (the `grep -F` case). */
export function escapeLiteralString(text: string): string {
	return text.split('').map(escapeLiteral).join('');
}

/**
 * Translates a bracket expression starting at `pattern[start]` (which must be
 * the opening `[`). Returns the JavaScript equivalent and the index just past
 * the closing `]`.
 */
function translateBracket(pattern: string, start: number): { source: string; next: number } {
	let i = start + 1;
	let body = '';

	if (pattern[i] === '^') {
		body += '^';
		i++;
	}

	// A `]` in the first position is a literal, not the terminator.
	if (pattern[i] === ']') {
		body += '\\]';
		i++;
	}

	for (; i < pattern.length; i++) {
		const char = pattern[i]!;

		if (char === ']') {
			return { source: `[${body}]`, next: i + 1 };
		}

		// [:alnum:], [.collating.] and [=equivalence=] forms.
		if (char === '[' && (pattern[i + 1] === ':' || pattern[i + 1] === '.' || pattern[i + 1] === '=')) {
			const kind = pattern[i + 1]!;
			const close = pattern.indexOf(`${kind}]`, i + 2);

			if (close !== -1) {
				const name = pattern.slice(i + 2, close);

				if (kind === ':') {
					const expansion = NAMED_CLASSES[name];

					if (expansion === undefined) {
						throw new Error(`unsupported POSIX character class [:${name}:]`);
					}

					body += expansion;
				} else {
					// Collating/equivalence elements degrade to their literal text.
					body += escapeLiteralString(name);
				}

				i = close + 1;
				continue;
			}
		}

		// A backslash is an ordinary character inside a POSIX bracket expression.
		if (char === '\\') {
			body += '\\\\';
			continue;
		}

		// Ranges and everything else pass through, with JavaScript's own
		// metacharacters neutralised.
		if (char === '^' || char === '[') {
			body += `\\${char}`;
			continue;
		}

		body += char;
	}

	throw new Error(`unterminated bracket expression in /${pattern}/`);
}

interface TranslateOptions {
	/** True for Extended Regular Expressions (`sed -E`, `grep -E`). */
	extended: boolean;
}

function translate(pattern: string, { extended }: TranslateOptions): string {
	let out = '';
	let i = 0;

	// Tracks whether the next token sits at the start of the expression or of a
	// subexpression, which is where BRE treats `^` as an anchor and `*` as a
	// literal.
	let atStart = true;

	const isAtEnd = (index: number): boolean => {
		if (index >= pattern.length) {
			return true;
		}

		if (extended) {
			return pattern[index] === ')' || pattern[index] === '|';
		}

		return pattern.startsWith('\\)', index) || pattern.startsWith('\\|', index);
	};

	while (i < pattern.length) {
		const char = pattern[i]!;

		if (char === '\\') {
			const next = pattern[i + 1];

			if (next === undefined) {
				out += '\\\\';
				i++;
				atStart = false;
				continue;
			}

			// In a BRE the backslashed forms are the operators; in an ERE they
			// are the literals. Either way the *other* dialect's handling of the
			// bare character is dealt with further down.
			if ('(){}|+?'.includes(next)) {
				if (extended) {
					out += `\\${next}`;
					atStart = false;
				} else {
					out += next;
					atStart = next === '(' || next === '|';
				}

				i += 2;
				continue;
			}

			if (next >= '1' && next <= '9') {
				out += `\\${next}`;
				i += 2;
				atStart = false;
				continue;
			}

			// GNU escapes that mean the same thing in JavaScript.
			if ('nrtfvwWsSdDbB'.includes(next)) {
				out += `\\${next}`;
				i += 2;
				atStart = false;
				continue;
			}

			// GNU word boundaries.
			if (next === '<' || next === '>') {
				out += '\\b';
				i += 2;
				atStart = false;
				continue;
			}

			out += escapeLiteral(next);
			i += 2;
			atStart = false;
			continue;
		}

		if (char === '[') {
			const { source, next } = translateBracket(pattern, i);
			out += source;
			i = next;
			atStart = false;
			continue;
		}

		if (char === '^') {
			// Anchor at the start of an expression or subexpression, literal
			// everywhere else in a BRE. EREs treat it as an anchor throughout.
			out += extended || atStart ? '^' : '\\^';
			i++;
			atStart = false;
			continue;
		}

		if (char === '$') {
			out += extended || isAtEnd(i + 1) ? '$' : '\\$';
			i++;
			atStart = false;
			continue;
		}

		if (char === '*') {
			// A `*` with nothing to repeat is a literal asterisk in POSIX.
			out += atStart ? '\\*' : '*';
			i++;
			atStart = false;
			continue;
		}

		if (char === '.') {
			out += '.';
			i++;
			atStart = false;
			continue;
		}

		if ('(){}|+?'.includes(char)) {
			if (extended) {
				out += char;
				atStart = char === '(' || char === '|';
			} else {
				out += `\\${char}`;
				atStart = false;
			}

			i++;
			continue;
		}

		out += escapeLiteral(char);
		i++;
		atStart = false;
	}

	return out;
}

const breCache = new Map<string, string>();
const ereCache = new Map<string, string>();

/** Translates a POSIX Basic Regular Expression into JavaScript regex source. */
export function breToJs(pattern: string): string {
	let cached = breCache.get(pattern);

	if (cached === undefined) {
		cached = translate(pattern, { extended: false });
		breCache.set(pattern, cached);
	}

	return cached;
}

/** Translates a POSIX Extended Regular Expression into JavaScript regex source. */
export function ereToJs(pattern: string): string {
	let cached = ereCache.get(pattern);

	if (cached === undefined) {
		cached = translate(pattern, { extended: true });
		ereCache.set(pattern, cached);
	}

	return cached;
}

export interface CompileOptions {
	/** Treat the pattern as an ERE rather than a BRE. */
	extended?: boolean;
	/** Match the pattern literally, as `grep -F` does. */
	fixed?: boolean;
	/** Apply the match repeatedly, as the `g` flag does for sed. */
	global?: boolean;
	/** Case-insensitive matching. */
	ignoreCase?: boolean;
}

/** Compiles a POSIX pattern into a JavaScript regular expression. */
export function compile(pattern: string, options: CompileOptions = {}): RegExp {
	const source = options.fixed
		? escapeLiteralString(pattern)
		: options.extended
			? ereToJs(pattern)
			: breToJs(pattern);

	let flags = '';

	if (options.global) {
		flags += 'g';
	}

	if (options.ignoreCase) {
		flags += 'i';
	}

	return new RegExp(source, flags);
}
