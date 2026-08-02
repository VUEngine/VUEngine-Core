/**
 * The advisory lock protocol the preprocessor uses to coordinate the parallel
 * `make -j` jobs that all preprocess headers into one shared working folder.
 *
 * A lock is a directory named `<path>.lock`; `mkdir` is atomic on POSIX, so
 * whichever process creates it wins. The directory holds a stamp file naming
 * the owner, which is what makes stale locks recoverable.
 *
 * Deviation from the shell original: there, the stale-lock check read
 * `[ ! kill -0 $PID ]`, which is a shell syntax error rather than a process
 * liveness test — it always evaluated false, so a lock left behind by a killed
 * process was never reclaimed and every later build blocked on it until the
 * wait loop gave up. The liveness check the code plainly intended is
 * implemented here. It affects only how long the preprocessor waits, never what
 * it writes.
 */

import {
	appendLine,
	isDirectory,
	makeDirectoryExclusive,
	readText,
	removeFile,
	removeTree,
	sleep,
	writeText,
} from './files.ts';
import { grep } from '../posix/grep.ts';
import { parentProcessId, processId, processIsAlive, userId } from '../platform/platform.ts';

/** The polling intervals the original used, in milliseconds. */
const SHORT_WAIT = 10;
const LONG_WAIT = 100;

/** How many short waits pass before the wait is reported to the log. */
const REPORT_EVERY = 100;

export interface LockContext {
	/** Appends a line to this class' log file. */
	log(message: string): void;
	/** The `-g` argument: which class or makefile asked for this work. */
	caller: string;
	/** The file being preprocessed, recorded in the stamp for diagnostics. */
	inputFile: string;
}

function stampOf(): string {
	return `Stamp ${processId()} : ${parentProcessId()} : ${userId()}`;
}

function lockFolderOf(path: string): string {
	return `${path}.lock`;
}

/** Reads the owning process id out of a lock's stamp file. */
function lockOwnerPid(lockFile: string): number | undefined {
	const stamp = grep(readText(lockFile), 'Stamp')[0];

	if (stamp === undefined) {
		return undefined;
	}

	const pid = Number(stamp.split(':')[1]?.trim());

	return Number.isInteger(pid) && pid > 0 ? pid : undefined;
}

/** Waits until nobody holds the lock on `path`. */
export function waitForLockToRelease(path: string, context: LockContext): void {
	const lockFolder = lockFolderOf(path);
	let counter = 0;

	while (isDirectory(lockFolder)) {
		counter++;

		if (counter > REPORT_EVERY) {
			counter = 0;
			context.log(`Waiting on caller ${context.caller} for:`);
			context.log(`\t\t${lockFolder.replace(/.*(\/[A-Za-z][A-Za-z0-9]*\.*)/, '$1')}`);
		}

		sleep(SHORT_WAIT);
	}
}

/** What to do when the lock is already held. */
export type LockContention =
	/** Wait for the holder and then take the lock. */
	| 'wait'
	/** Give up and end this script, as the `exit` command argument did. */
	| 'give-up';

/**
 * Acquires the lock for `path`, blocking until it is free.
 *
 * Returns false only when `contention` is `give-up` and somebody else held the
 * lock, in which case the caller is expected to stop work.
 */
export function tryToLock(path: string, context: LockContext, contention: LockContention = 'wait'): boolean {
	if (path === '') {
		return true;
	}

	const lockFolder = lockFolderOf(path);
	const lockFile = `${lockFolder}/stamp.txt`;

	for (;;) {
		sleep(SHORT_WAIT);
		context.log(`Trying to lock on ${path} on caller ${context.caller}`);

		if (makeDirectoryExclusive(lockFolder)) {
			break;
		}

		const pid = lockOwnerPid(lockFile);

		if (pid !== undefined && !processIsAlive(pid)) {
			context.log(`Removing stale lock of nonexistent PID ${pid} for ${path}`);
			removeFile(lockFile);
			sleep(SHORT_WAIT);
			removeTree(lockFolder);
			continue;
		}

		waitForLockToRelease(path, context);

		if (contention === 'give-up') {
			context.log(`Gived up with command exit on caller ${context.caller}`);

			return false;
		}
	}

	context.log(`Succeeded to lock ${path} on caller ${context.caller}`);

	const stamp = stampOf();
	writeText(lockFile, `${stamp}\n`);
	appendLine(lockFile, `Locked by ${context.inputFile}`);
	appendLine(lockFile, `Caller ${context.caller}`);

	const readStamp = grep(readText(lockFile), 'Stamp')[0] ?? '';

	if (readStamp !== stamp) {
		console.log(
			`Error on reading ${path} read stamp (${readStamp}) doesn't match my stamp (${stamp}) on caller ${context.caller}`,
		);
	}

	return true;
}

/** Releases a lock this process holds. */
export function releaseLock(path: string, context: LockContext): void {
	if (path === '') {
		return;
	}

	const lockFolder = lockFolderOf(path);

	if (!isDirectory(lockFolder)) {
		context.log(`Cannot release lock on ${path}, doesn't exist the folder ${lockFolder}`);

		return;
	}

	const stamp = stampOf();
	const lockFile = `${lockFolder}/stamp.txt`;
	const readStamp = grep(readText(lockFile), 'Stamp')[0] ?? '';

	if (readStamp !== stamp) {
		const message = `Error on unlocking ${path} read stamp (${readStamp}) doesn't match my stamp (${stamp}) on caller ${context.caller}`;
		context.log(message);
		console.log(message);
	}

	removeFile(lockFile);
	sleep(SHORT_WAIT);
	removeTree(lockFolder);
	context.log(`Released lock ${path} on caller ${context.caller}`);
}

/** Waits with the longer interval the hierarchy computation used. */
export function waitLong(): void {
	sleep(LONG_WAIT);
}

/** Waits with the shorter interval used everywhere else. */
export function waitShort(): void {
	sleep(SHORT_WAIT);
}
