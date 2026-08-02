/**
 * The slice of QuickJS's built-in modules that `platform.quickjs.ts` uses.
 *
 * QuickJS ships no type definitions, and these are deliberately minimal: they
 * describe only what is actually called, including the `[value, errno]` return
 * convention its `os` module uses in place of exceptions.
 */

/** Arguments the interpreter was invoked with; index 0 is the script itself. */
declare const scriptArgs: string[];

declare module 'os' {
	interface Status {
		mode: number;
		size: number;
		/** Modification time in milliseconds. */
		mtime: number;
	}

	/** Bit mask selecting the file-type bits of a mode. */
	const S_IFMT: number;
	/** File type: regular file. */
	const S_IFREG: number;
	/** File type: directory. */
	const S_IFDIR: number;
	/** File type: FIFO, which is what a pipe on standard input is. */
	const S_IFIFO: number;

	function stat(path: string): [Status, number];
	function readdir(path: string): [string[], number];
	function mkdir(path: string, mode?: number): number;
	function remove(path: string): number;
	function utimes(path: string, atime: number, mtime: number): number;
	function kill(pid: number, signal: number): number;
	function getpid(): number;
	function sleep(milliseconds: number): void;
}

declare module 'std' {
	export interface File {
		puts(text: string): void;
		close(): number;
		readAsString(maximumBytes?: number): string | null;
	}

	export const out: File;
	export const err: File;

	// `in` is a reserved word, so it cannot be declared directly.
	const standardInput: File;
	export { standardInput as in };

	export function open(path: string, mode: string): File | null;
	export function loadFile(path: string): string | null;
	export function getenv(name: string): string | undefined;
	export function exit(code: number): never;
}
