/**
 * Host services, implemented against Node.
 *
 * Everything the preprocessor needs from outside the language lives behind this
 * module, so the same code can run on a second engine. `platform.quickjs.ts` is
 * the other implementation; the build substitutes it when producing the QuickJS
 * bundle. Keep the two in step, and keep this surface as small as possible.
 */

import {
	appendFileSync,
	closeSync,
	fstatSync,
	mkdirSync,
	openSync,
	readFileSync,
	readdirSync,
	rmSync,
	statSync,
	utimesSync,
	writeFileSync,
} from 'node:fs';

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
	try {
		return readFileSync(path, 'utf8');
	} catch {
		return undefined;
	}
}

/** Writes a whole file, replacing it if it exists. */
export function writeFile(path: string, contents: string): void {
	writeFileSync(path, contents);
}

/** Appends to a file, creating it when missing. */
export function appendFile(path: string, contents: string): void {
	appendFileSync(path, contents);
}

/**
 * Creates a single directory, failing if it already exists. Returning whether
 * it was created is what makes this usable as an atomic lock.
 */
export function createDirectory(path: string): boolean {
	try {
		mkdirSync(path);

		return true;
	} catch {
		return false;
	}
}

/** Creates a directory and any missing parents. */
export function createDirectories(path: string): void {
	mkdirSync(path, { recursive: true });
}

/** Lists a directory, or undefined when it cannot be read. */
export function readDirectory(path: string): DirectoryEntry[] | undefined {
	try {
		return readdirSync(path, { withFileTypes: true }).map((entry) => ({
			name: entry.name,
			isDirectory: entry.isDirectory(),
		}));
	} catch {
		return undefined;
	}
}

/** Stats a path, or undefined when it does not exist. */
export function stat(path: string): FileStatus | undefined {
	try {
		const status = statSync(path);

		return {
			isFile: status.isFile(),
			isDirectory: status.isDirectory(),
			size: status.size,
			modifiedMilliseconds: status.mtimeMs,
		};
	} catch {
		return undefined;
	}
}

/** Removes a file, ignoring a missing one. */
export function remove(path: string): void {
	rmSync(path, { force: true });
}

/** Removes a path and everything below it. */
export function removeRecursively(path: string): void {
	rmSync(path, { force: true, recursive: true });
}

/** Creates the file if missing, and sets its modification time to now. */
export function setModifiedNow(path: string): void {
	try {
		const now = new Date();
		utimesSync(path, now, now);
	} catch {
		closeSync(openSync(path, 'a'));
	}
}

/** The arguments after the program name. */
export function commandLineArguments(): string[] {
	return process.argv.slice(2);
}

/** An environment variable, or undefined when unset. */
export function environmentVariable(name: string): string | undefined {
	return process.env[name];
}

/** This process' id. */
export function processId(): number {
	return process.pid;
}

/** This process' parent's id. */
export function parentProcessId(): number {
	return process.ppid;
}

/** The user this process runs as. */
export function userId(): number {
	return process.getuid?.() ?? 0;
}

/** Whether a process is still running, which `kill -0` is the shell idiom for. */
export function processIsAlive(pid: number): boolean {
	try {
		process.kill(pid, 0);

		return true;
	} catch (error) {
		// EPERM means the process exists but belongs to someone else.
		return (error as NodeJS.ErrnoException).code === 'EPERM';
	}
}

/** Sets the status this program will exit with. */
export function setExitCode(code: number): void {
	process.exitCode = code;
}

/** Writes to standard output without adding a newline. */
export function writeStandardOutput(text: string): void {
	process.stdout.write(text);
}

/** Writes to standard error without adding a newline. */
export function writeStandardError(text: string): void {
	process.stderr.write(text);
}

/** Reads all of standard input. */
export function readStandardInput(): string {
	try {
		return readFileSync(0, 'utf8');
	} catch {
		return '';
	}
}

/** `[ -p /dev/stdin ]` — whether standard input is a pipe. */
export function standardInputIsPipe(): boolean {
	try {
		return fstatSync(0).isFIFO();
	} catch {
		return false;
	}
}

/** Blocks for the given number of milliseconds. */
export function sleep(milliseconds: number): void {
	Atomics.wait(new Int32Array(new SharedArrayBuffer(4)), 0, 0, milliseconds);
}
