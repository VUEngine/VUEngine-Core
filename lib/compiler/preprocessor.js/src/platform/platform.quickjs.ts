/**
 * Host services, implemented against QuickJS.
 *
 * The counterpart to `platform.ts`, substituted by the build when producing
 * `dist/preprocessor.mjs`. It exists so that a machine without Node installed
 * can still build: VUEngine Studio ships a QuickJS interpreter, which is around
 * a megabyte rather than Node's hundred.
 *
 * QuickJS reports errors as negative errno values or as a trailing error field
 * rather than by throwing, so most of this is translating that convention into
 * the same shapes `platform.ts` returns.
 */

import * as os from 'os';
import * as std from 'std';

export interface DirectoryEntry {
	name: string;
	isDirectory: boolean;
}

export interface FileStatus {
	isFile: boolean;
	isDirectory: boolean;
	size: number;
	modifiedMilliseconds: number;
}

/** Reads a whole file, or undefined when it cannot be read. */
export function readFile(path: string): string | undefined {
	return std.loadFile(path) ?? undefined;
}

function writeWithMode(path: string, contents: string, mode: string): void {
	const file = std.open(path, mode);

	if (file === null) {
		return;
	}

	file.puts(contents);
	file.close();
}

/** Writes a whole file, replacing it if it exists. */
export function writeFile(path: string, contents: string): void {
	writeWithMode(path, contents, 'w');
}

/** Appends to a file, creating it when missing. */
export function appendFile(path: string, contents: string): void {
	writeWithMode(path, contents, 'a');
}

/**
 * Creates a single directory, failing if it already exists. Returning whether
 * it was created is what makes this usable as an atomic lock.
 */
export function createDirectory(path: string): boolean {
	return os.mkdir(path) === 0;
}

/** Creates a directory and any missing parents. */
export function createDirectories(path: string): void {
	const absolute = path.startsWith('/');
	const segments = path.split('/').filter((segment) => segment !== '');
	let current = absolute ? '' : '.';

	for (const segment of segments) {
		current = `${current}/${segment}`;
		// Anything other than "already exists" is left for the caller's next
		// operation to surface, exactly as `mkdir -p` does.
		os.mkdir(current);
	}
}

/** Lists a directory, or undefined when it cannot be read. */
export function readDirectory(path: string): DirectoryEntry[] | undefined {
	const [names, error] = os.readdir(path);

	if (error !== 0) {
		return undefined;
	}

	const entries: DirectoryEntry[] = [];

	for (const name of names) {
		if (name === '.' || name === '..') {
			continue;
		}

		const [status, statError] = os.stat(`${path}/${name}`);
		entries.push({
			name,
			isDirectory: statError === 0 && (status.mode & os.S_IFMT) === os.S_IFDIR,
		});
	}

	return entries;
}

/** Stats a path, or undefined when it does not exist. */
export function stat(path: string): FileStatus | undefined {
	const [status, error] = os.stat(path);

	if (error !== 0) {
		return undefined;
	}

	return {
		isFile: (status.mode & os.S_IFMT) === os.S_IFREG,
		isDirectory: (status.mode & os.S_IFMT) === os.S_IFDIR,
		size: status.size,
		modifiedMilliseconds: status.mtime,
	};
}

/** Removes a file, ignoring a missing one. */
export function remove(path: string): void {
	os.remove(path);
}

/** Removes a path and everything below it. */
export function removeRecursively(path: string): void {
	const entries = readDirectory(path);

	if (entries !== undefined) {
		for (const entry of entries) {
			const child = `${path}/${entry.name}`;

			if (entry.isDirectory) {
				removeRecursively(child);
			} else {
				os.remove(child);
			}
		}
	}

	os.remove(path);
}

/** Creates the file if missing, and sets its modification time to now. */
export function setModifiedNow(path: string): void {
	const now = Date.now();

	if (os.utimes(path, now, now) !== 0) {
		writeWithMode(path, '', 'a');
	}
}

/** The arguments after the program name. */
export function commandLineArguments(): string[] {
	// scriptArgs[0] is the script itself, where Node also carries the executable.
	return scriptArgs.slice(1);
}

/** An environment variable, or undefined when unset. */
export function environmentVariable(name: string): string | undefined {
	return std.getenv(name);
}

/** This process' id. */
export function processId(): number {
	return os.getpid();
}

/**
 * This process' parent's id. QuickJS does not expose it; the value only ever
 * reaches a diagnostic line in a lock file, so zero is an acceptable stand-in.
 */
export function parentProcessId(): number {
	return 0;
}

/** The user this process runs as. Diagnostic only, as above. */
export function userId(): number {
	return 0;
}

/** Whether a process is still running, which `kill -0` is the shell idiom for. */
export function processIsAlive(pid: number): boolean {
	// 0 means the signal was delivered; -EPERM means it exists but is not ours.
	const result = os.kill(pid, 0);

	return result === 0 || result === -1;
}

/** Sets the status this program will exit with. */
export function setExitCode(code: number): void {
	if (code !== 0) {
		std.exit(code);
	}
}

/** Writes to standard output without adding a newline. */
export function writeStandardOutput(text: string): void {
	std.out.puts(text);
}

/**
 * Writes to standard error without adding a newline. QuickJS's console has
 * only `log`, so this cannot go through `console.error`.
 */
export function writeStandardError(text: string): void {
	std.err.puts(text);
}

/** Reads all of standard input. */
export function readStandardInput(): string {
	return std.in.readAsString() ?? '';
}

/** `[ -p /dev/stdin ]` — whether standard input is a pipe. */
export function standardInputIsPipe(): boolean {
	const [status, error] = os.stat('/dev/stdin');

	return error === 0 && (status.mode & os.S_IFMT) === os.S_IFIFO;
}

/** Blocks for the given number of milliseconds. */
export function sleep(milliseconds: number): void {
	os.sleep(milliseconds);
}
