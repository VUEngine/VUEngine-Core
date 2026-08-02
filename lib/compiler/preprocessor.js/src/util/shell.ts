import { environmentVariable } from '../platform/platform.ts';

/**
 * Shell semantics the ported scripts depend on: how `exit` unwinds, how
 * arguments were parsed, and how `[ a -nt b ]` compares timestamps.
 */

/**
 * Models a script calling `exit`. Thrown rather than calling `process.exit`
 * because processHeaderFile invokes itself recursively for base classes, and a
 * base class exiting must not take the caller down with it.
 */
export class ExitSignal extends Error {
	readonly code: number;

	constructor(code: number) {
		super(`exit ${code}`);
		this.name = 'ExitSignal';
		this.code = code;
	}
}

/** Ends the current script with the given status. */
export function exit(code: number): never {
	throw new ExitSignal(code);
}

/**
 * Parses arguments the way the original `while … case` loops did: every
 * recognised flag takes the following argument, and anything unrecognised is
 * skipped silently.
 *
 * `trailing` reproduces the difference between the scripts' loop conditions.
 * processHeaderFile and setupClasses used `while [ $# -gt 1 ]`, which leaves a
 * lone final argument unread; processSourceFile used `while [ $# -gt 0 ]`.
 */
export function parseArguments(
	argv: readonly string[],
	flags: { readonly valued: readonly string[]; readonly boolean?: readonly string[] },
	trailing: 'consume-last' | 'ignore-last' = 'consume-last',
): Map<string, string[]> {
	const parsed = new Map<string, string[]>();
	const booleans = new Set(flags.boolean ?? []);
	const valued = new Set(flags.valued);
	const limit = trailing === 'ignore-last' ? argv.length - 1 : argv.length;

	const record = (key: string, value: string): void => {
		const existing = parsed.get(key);

		if (existing === undefined) {
			parsed.set(key, [value]);
		} else {
			existing.push(value);
		}
	};

	for (let i = 0; i < limit; i++) {
		const key = argv[i]!;

		if (booleans.has(key)) {
			record(key, 'true');
			continue;
		}

		if (valued.has(key)) {
			const value = argv[i + 1];

			if (value !== undefined) {
				record(key, value);
				i++;
			}
		}
	}

	return parsed;
}

/** The last value given for a flag, or the fallback when it was not given. */
export function flagValue(parsed: Map<string, string[]>, key: string, fallback = ''): string {
	const values = parsed.get(key);

	return values === undefined || values.length === 0 ? fallback : values[values.length - 1]!;
}

/**
 * The value of a flag, falling back to an environment variable of the given
 * name and then to `fallback`.
 *
 * This models the shell scripts' most easily missed behaviour. They declared
 * some variables empty before parsing arguments but left others alone, and the
 * ones they left alone kept whatever the environment held. That is not
 * incidental: make exports the variables assigned on its command line into
 * every recipe's environment, so `GAME_NAME`, `PLUGINS_FOLDER` and friends were
 * always set even when the corresponding flag was not passed — which is exactly
 * what happens when processHeaderFile recurses into a base class, since that
 * call omits `-x`.
 */
export function flagOrEnvValue(
	parsed: Map<string, string[]>,
	key: string,
	environmentName: string,
	fallback = '',
): string {
	const values = parsed.get(key);

	if (values !== undefined && values.length > 0) {
		return values[values.length - 1]!;
	}

	return environmentVariable(environmentName) ?? fallback;
}

/** Every value given for a repeatable flag, in order. */
export function flagValues(parsed: Map<string, string[]>, key: string): string[] {
	return parsed.get(key) ?? [];
}

/** Whether a boolean flag was present. */
export function flagSet(parsed: Map<string, string[]>, key: string): boolean {
	return parsed.has(key);
}
