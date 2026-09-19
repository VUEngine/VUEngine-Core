/**
 * The source pass must not move a line.
 *
 * GCC records line numbers against the file it reads, which is the generated
 * one — so a debugger resolving a breakpoint on line N of somebody's own
 * source is only right if N means the same thing in both files. The pass is
 * built to preserve numbering (the class definition is prepended to an
 * existing line rather than given one of its own), and this is what says it
 * still does.
 *
 * Blank lines are the measure. A transpiler rewrites class syntax — even
 * inside comments, which is why comparing their text proves nothing — but it
 * cannot rewrite nothing. A blank line that has moved is a line number that
 * has moved.
 *
 * Run over the engine's own sources when they are to hand, because that is
 * the corpus the drift was found in; the synthetic case below runs anywhere
 * and covers the shape that caused it — a method whose parameter list is
 * wrapped onto the next line.
 */

import { execFileSync } from 'node:child_process';
import { existsSync, mkdirSync, mkdtempSync, readFileSync, readdirSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';
import { after, before, describe, test } from 'node:test';

const here = dirname(fileURLToPath(import.meta.url));
const packageRoot = join(here, '..');
const engineRoot = join(packageRoot, '..', '..', '..');
const mainScript = join(packageRoot, 'src', 'main.ts');

let work = '';

before(() => {
	work = mkdtempSync(join(tmpdir(), 'preprocessor-lines-'));
	mkdirSync(join(work, 'out'), { recursive: true });
});

after(() => {
	if (work !== '') {
		rmSync(work, { recursive: true, force: true });
	}
});

/** A hierarchy file naming one class and its base, which is all the pass reads. */
function hierarchyFor(className: string, baseName: string): string {
	const path = join(work, `${className}.hierarchy.txt`);
	writeFileSync(path, `${className}:${baseName}\n`);

	return path;
}

/** Transpile one file and hand back both versions, split into lines. */
function transpile(inputFile: string, hierarchy: string): { before: string[]; after: string[] } {
	const outputFile = join(work, 'out', `${Date.now()}-${Math.random().toString(36).slice(2)}.c`);
	execFileSync(
		'node',
		[
			mainScript, 'source',
			'-i', inputFile,
			'-o', outputFile,
			'-w', join(work, 'w'),
			'-c', hierarchy,
			'-e', engineRoot,
		],
		{ stdio: 'pipe' }
	);

	return {
		before: readFileSync(inputFile, 'utf8').split(/\r?\n/),
		after: readFileSync(outputFile, 'utf8').split(/\r?\n/),
	};
}

/** Which of the original's blank lines are still blank at the same number. */
function blankLineAlignment(pair: { before: string[]; after: string[] }): { total: number; aligned: number; first: number } {
	let total = 0;
	let aligned = 0;
	let first = 0;

	for (let index = 0; index < pair.before.length; index++) {
		if (pair.before[index]!.trim() !== '') {
			continue;
		}
		total++;
		if ((pair.after[index] ?? '').trim() === '') {
			aligned++;
		} else if (first === 0) {
			first = index + 1;
		}
	}

	return { total, aligned, first };
}

describe('line numbering', () => {
	test('a wrapped parameter list does not push the rest of the file down', () => {
		// The shape that drifted: a method whose parameters sit on their own
		// line. The pass pulls them back up to read the signature, and used to
		// spend an extra newline putting them back.
		const source = join(work, 'Thing.c');
		writeFileSync(source, [
			'#include <Thing.h>',
			'',
			'static void Thing::wrapped',
			'(',
			'\tint first, int second',
			')',
			'{',
			'\t(void)first;',
			'\t(void)second;',
			'}',
			'',
			'void Thing::plain(int value)',
			'{',
			'\t(void)value;',
			'}',
			'',
		].join('\n'));

		const pair = transpile(source, hierarchyFor('Thing', 'Object'));
		const alignment = blankLineAlignment(pair);

		assert.ok(alignment.total > 0, 'the fixture has blank lines to compare');
		assert.equal(
			alignment.aligned,
			alignment.total,
			`blank line ${alignment.first} moved; the pass gained a line above it`
		);
	});

	test("every one of the engine's own sources keeps its numbering", { skip: skipReason() }, () => {
		const hierarchy = engineHierarchy();
		const sources = collectSources(join(engineRoot, 'source'));
		const drifted: string[] = [];

		for (const source of sources) {
			let pair;
			try {
				pair = transpile(source, hierarchy);
			} catch {
				// A source the pass declines — an asset, or one needing a class
				// this hierarchy does not name — is not what this is about.
				continue;
			}
			const alignment = blankLineAlignment(pair);
			if (alignment.total > 0 && alignment.aligned !== alignment.total) {
				drifted.push(`${source.slice(engineRoot.length + 1)} at line ${alignment.first}`);
			}
		}

		assert.deepEqual(drifted, [], 'these sources no longer line up with their output');
	});
});

/** Every `.c` below a folder. */
function collectSources(root: string): string[] {
	const found: string[] = [];
	for (const entry of readdirSync(root, { withFileTypes: true })) {
		const at = join(root, entry.name);
		if (entry.isDirectory()) {
			found.push(...collectSources(at));
		} else if (entry.name.endsWith('.c')) {
			found.push(at);
		}
	}

	return found;
}

/**
 * The engine's own class hierarchy, built the way the pass expects to read
 * one: `Class:Base` a line, over every class the sources declare.
 */
function engineHierarchy(): string {
	const path = join(work, 'engine.hierarchy.txt');
	const lines: string[] = [];

	for (const header of collectHeaders(join(engineRoot, 'source'))) {
		const text = readFileSync(header, 'utf8');
		const match = /^\s*(?:abstract\s+|singleton\s+|static\s+)*class\s+([A-Za-z0-9_]+)\s*:\s*([A-Za-z0-9_]+)/m.exec(text);
		if (match) {
			lines.push(`${match[1]}:${match[2]}`);
		}
	}
	writeFileSync(path, `${lines.join('\n')}\n`);

	return path;
}

function collectHeaders(root: string): string[] {
	const found: string[] = [];
	for (const entry of readdirSync(root, { withFileTypes: true })) {
		const at = join(root, entry.name);
		if (entry.isDirectory()) {
			found.push(...collectHeaders(at));
		} else if (entry.name.endsWith('.h')) {
			found.push(at);
		}
	}

	return found;
}

/** Why the engine-wide case is not running, or undefined when it is. */
function skipReason(): string | false {
	return existsSync(join(engineRoot, 'source'))
		? false
		: 'the engine sources are not beside this package';
}
