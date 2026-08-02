/**
 * Unit tests for the shell semantics the port depends on.
 *
 * Each case here corresponds to a behaviour that differs between the shell
 * tools and their obvious JavaScript equivalent, and that produced — or would
 * have produced — wrong C output. They are regression tests for those specific
 * traps.
 */

import assert from 'node:assert/strict';
import { describe, test } from 'node:test';

import { grep } from '../src/posix/grep.ts';
import { subst, substAll } from '../src/posix/sed.ts';
import { capture, cut, cutFrom, expandUnquoted, countLines, head, tail, sortUnique, uniqueInOrder, words } from '../src/posix/text.ts';
import { flagOrEnvValue, flagValue, flagValues, parseArguments } from '../src/util/shell.ts';

describe('sed substitution', () => {
	test('a null match adjacent to the previous match is discarded', () => {
		// GNU sed prints "X" here; JavaScript's replace would give "XX".
		assert.equal(subst('abc', '.*', 'X', { global: true }), 'X');
		assert.equal(subst('abc', 'b*', 'X', { global: true }), 'XaXcX');
		assert.equal(subst('abc', 'x*', '-', { global: true }), '-a-b-c-');
	});

	test('an unescaped ampersand is the whole match', () => {
		assert.equal(subst('abc', 'b', '[&]', { global: true }), 'a[b]c');
	});

	test('an escaped ampersand is a literal', () => {
		// The bug this guards against turned `getInstance((ClassPointer)&Foo…`
		// into `getInstance((ClassPointer)getInstance(Foo…`.
		assert.equal(
			subst('x getInstance()', 'getInstance(', 'getInstance((ClassPointer)\\&Foo_getBaseClass', {
				global: true,
			}),
			'x getInstance((ClassPointer)&Foo_getBaseClass)',
		);
	});

	test('anchors bind to each line, not to the whole text', () => {
		assert.equal(subst('a\nb\n', '^', '>', { global: true }), '>a\n>b\n');
	});

	test('a trailing newline is not a line of its own', () => {
		assert.equal(subst('a\n', '.*', '[&]', { global: true }), '[a]\n');
		assert.equal(subst('a', '.*', '[&]', { global: true }), '[a]');
	});

	test('a backslash inside a bracket expression is a literal', () => {
		// POSIX reads [^\*] as "not backslash and not asterisk".
		assert.deepEqual(grep('a\\b\nc*d\nefg', '^[^\\*]*$'), ['efg']);
	});

	test('substitutions run in sequence over each line', () => {
		assert.equal(
			substAll('one two', [
				['one', '1'],
				['two', '2'],
			]),
			'1 2',
		);
	});
});

describe('cut', () => {
	test('a line without the delimiter passes through unchanged', () => {
		assert.equal(cut('nodelimiter', ':', [2]), 'nodelimiter');
	});

	test('selecting several fields rejoins them with the delimiter', () => {
		assert.equal(cut('28:singleton class Timer : ListenerObject', ':', [2, 3]), 'singleton class Timer : ListenerObject');
	});

	test('cutFrom takes every field from the given one onwards', () => {
		assert.equal(cutFrom('a,b,c,d', ',', 2), 'b,c,d');
	});
});

describe('unquoted expansion', () => {
	test('runs of whitespace collapse to a single space', () => {
		// A constructor declared with two spaces yields a generated allocator
		// with one, because the original script left the expansion unquoted.
		assert.equal(
			expandUnquoted('void constructor(const Spec* spec,  int16 id);'),
			'void constructor(const Spec* spec, int16 id);',
		);
	});
});

describe('line counting and slicing', () => {
	test('countLines counts newlines, as wc -l does', () => {
		assert.equal(countLines('a\nb\n'), 2);
		assert.equal(countLines('a\nb'), 1);
		assert.equal(countLines(''), 0);
	});

	test('head and tail keep whole lines', () => {
		assert.equal(head('a\nb\nc\n', 2), 'a\nb\n');
		assert.equal(tail('a\nb\nc\n', 2), 'b\nc\n');
		assert.equal(head('a\nb\nc', 5), 'a\nb\nc');
	});

	test('capture trims the trailing newlines a command substitution drops', () => {
		assert.equal(capture('value\n\n'), 'value');
	});
});

describe('deduplication and word splitting', () => {
	test('uniqueInOrder keeps first occurrences, as awk !x[$0]++ does', () => {
		assert.deepEqual(uniqueInOrder('b\na\nb\nc\n'), ['b', 'a', 'c']);
	});

	test('sortUnique sorts by byte value, as sort -u does in the C locale', () => {
		assert.deepEqual(sortUnique('b\nA\na\nB\nA\n'), ['A', 'B', 'a', 'b']);
	});

	test('words splits the way an unquoted expansion does', () => {
		assert.deepEqual(words('  a\tb\n c  '), ['a', 'b', 'c']);
	});
});

describe('argument parsing', () => {
	const flags = { valued: ['-i', '-o', '-l'], boolean: ['-d'] };

	test('repeated flags accumulate', () => {
		const parsed = parseArguments(['-l', 'one', '-l', 'two'], flags);
		assert.deepEqual(flagValues(parsed, '-l'), ['one', 'two']);
	});

	test('unrecognised arguments are skipped rather than rejected', () => {
		const parsed = parseArguments(['-z', 'junk', '-i', 'in.c'], flags);
		assert.equal(flagValue(parsed, '-i'), 'in.c');
	});

	test('ignore-last leaves a lone trailing argument unread', () => {
		// processHeaderFile looped `while [ $# -gt 1 ]`, which stops with one
		// argument still in place, so a flag in the final position is never seen.
		const consumed = parseArguments(['-i', 'in.c', '-d'], flags, 'consume-last');
		const ignored = parseArguments(['-i', 'in.c', '-d'], flags, 'ignore-last');

		assert.equal(flagValue(consumed, '-i'), 'in.c');
		assert.equal(flagValue(ignored, '-i'), 'in.c');
		assert.equal(consumed.has('-d'), true);
		assert.equal(ignored.has('-d'), false);
	});

	test('a value in the final position is still read', () => {
		// `-x $(GAME_NAME)` is last when a component has no plugins, and the
		// shell loop did consume it: with two arguments left, $# is still > 1.
		const parsed = parseArguments(['-i', 'in.c', '-o', 'out.c'], flags, 'ignore-last');
		assert.equal(flagValue(parsed, '-o'), 'out.c');
	});

	test('a missing flag falls back to the environment', () => {
		// make exports its command-line variables into every recipe's
		// environment, which is how the recursive call still sees GAME_NAME
		// despite not being passed -x.
		process.env.TEST_GAME_NAME = 'mygame';

		try {
			const parsed = parseArguments([], { valued: ['-x'] });
			assert.equal(flagOrEnvValue(parsed, '-x', 'TEST_GAME_NAME'), 'mygame');
			assert.equal(flagOrEnvValue(parsed, '-x', 'TEST_ABSENT', 'default'), 'default');
		} finally {
			delete process.env.TEST_GAME_NAME;
		}
	});

	test('an explicit flag wins over the environment', () => {
		process.env.TEST_GAME_NAME = 'fromenv';

		try {
			const parsed = parseArguments(['-x', 'fromflag'], { valued: ['-x'] });
			assert.equal(flagOrEnvValue(parsed, '-x', 'TEST_GAME_NAME'), 'fromflag');
		} finally {
			delete process.env.TEST_GAME_NAME;
		}
	});
});
