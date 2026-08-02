/**
 * File-system helpers matching the shell tests and commands the scripts used.
 *
 * Everything here is deliberately forgiving about missing files, because the
 * scripts ran with unquoted variables and missing paths routinely produced
 * empty strings rather than errors.
 *
 * All host access goes through `../platform/platform.ts`, so this works
 * unchanged on either JavaScript engine.
 */

import {
	appendFile,
	createDirectories,
	createDirectory,
	readDirectory,
	readFile,
	remove,
	removeRecursively,
	setModifiedNow,
	stat,
	writeFile,
} from '../platform/platform.ts';

export { sleep } from '../platform/platform.ts';

/**
 * Joins a directory and an entry the way `find` prints them: by plain
 * concatenation. Normalising would collapse the doubled slash a plugin path
 * such as `vuengine//other/Foo` produces, and that slash is load-bearing — it
 * ends up verbatim in the generated make dependency files.
 */
function joinRaw(directory: string, entry: string): string {
	return `${directory}/${entry}`;
}

/** Everything before the last slash, as `dirname` gives. */
function directoryOf(path: string): string {
	const index = path.lastIndexOf('/');

	if (index < 0) {
		return '';
	}

	return index === 0 ? '/' : path.slice(0, index);
}

/** `[ -f "$path" ]` — exists and is a regular file. */
export function isFile(path: string): boolean {
	return path !== '' && stat(path)?.isFile === true;
}

/** `[ -d "$path" ]` — exists and is a directory. */
export function isDirectory(path: string): boolean {
	return path !== '' && stat(path)?.isDirectory === true;
}

/** `[ -s "$path" ]` — exists and is not empty. */
export function isNonEmptyFile(path: string): boolean {
	if (path === '') {
		return false;
	}

	const status = stat(path);

	return status !== undefined && status.isFile && status.size > 0;
}

/**
 * `[ "$a" -nt "$b" ]` — whether `a` is newer than `b`. A missing `b` counts as
 * older, and a missing `a` as not newer, which is how the shell test behaves
 * and what the up-to-date checks rely on.
 */
export function isNewerThan(a: string, b: string): boolean {
	const timeA = a === '' ? undefined : stat(a)?.modifiedMilliseconds;

	if (timeA === undefined) {
		return false;
	}

	const timeB = b === '' ? undefined : stat(b)?.modifiedMilliseconds;

	return timeB === undefined || timeA > timeB;
}

/** Reads a file, returning an empty string when it does not exist. */
export function readText(path: string): string {
	return readFile(path) ?? '';
}

/** Writes a file, creating the parent directories as the scripts' `>` did not. */
export function writeText(path: string, contents: string): void {
	mkdirp(directoryOf(path));
	writeFile(path, contents);
}

/** Appends to a file, creating it and its parents when missing. */
export function appendText(path: string, contents: string): void {
	mkdirp(directoryOf(path));
	appendFile(path, contents);
}

/** Appends a line, as `echo … >> file` does. */
export function appendLine(path: string, line: string): void {
	appendText(path, `${line}\n`);
}

/** `mkdir -p`. */
export function mkdirp(path: string): void {
	if (path !== '') {
		createDirectories(path);
	}
}

/** `touch` — creates the file when absent, bumps its timestamp when present. */
export function touch(path: string): void {
	mkdirp(directoryOf(path));
	setModifiedNow(path);
}

/** `rm -f`. */
export function removeFile(path: string): void {
	if (path !== '') {
		remove(path);
	}
}

/** `rm -Rf`. */
export function removeTree(path: string): void {
	if (path !== '') {
		removeRecursively(path);
	}
}

/**
 * `mkdir "$path"` without `-p` — succeeds only if the directory did not exist.
 * This is the atomic operation the lock protocol is built on.
 */
export function makeDirectoryExclusive(path: string): boolean {
	return createDirectory(path);
}

/**
 * `find <roots> -name "<name>" -print -quit` — the first match in a
 * breadth-agnostic walk. Returns an empty string when nothing matches, mirroring
 * the empty capture the scripts then tested for.
 */
export function findFirstByName(roots: readonly string[], name: string): string {
	for (const root of roots) {
		const found = walkForName(root, name);

		if (found !== '') {
			return found;
		}
	}

	return '';
}

function walkForName(root: string, name: string): string {
	const entries = readDirectory(root);

	if (entries === undefined) {
		return '';
	}

	// `find` walks depth-first in directory order, descending as soon as it
	// meets a subdirectory rather than after finishing the files. Matching that
	// keeps the "first match wins" choice the same as the shell version's.
	for (const entry of entries) {
		const path = joinRaw(root, entry.name);

		if (entry.isDirectory) {
			const found = walkForName(path, name);

			if (found !== '') {
				return found;
			}
		} else if (entry.name === name) {
			return path;
		}
	}

	return '';
}

/**
 * `find <root> -type f -name "<pattern>"` — every match. Results are sorted,
 * which `find` does not guarantee, so that generated files are reproducible.
 */
export function findAllByName(root: string, name: string | ((fileName: string) => boolean)): string[] {
	const matches = typeof name === 'function' ? name : (fileName: string) => fileName === name;
	const found: string[] = [];

	const walk = (directory: string): void => {
		const entries = readDirectory(directory);

		if (entries === undefined) {
			return;
		}

		for (const entry of entries) {
			const path = joinRaw(directory, entry.name);

			if (entry.isDirectory) {
				walk(path);
			} else if (matches(entry.name)) {
				found.push(path);
			}
		}
	};

	walk(root);

	return found.sort();
}
