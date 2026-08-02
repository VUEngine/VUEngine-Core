/**
 * Differential tests for the POSIX regex translation.
 *
 * Every pattern below is copied verbatim from the shell scripts this package
 * replaces. Each one is run through GNU sed and through our own implementation
 * over a corpus built from the engine's real sources, and the two outputs must
 * agree byte for byte. That is a far stronger check than hand-written
 * expectations, and it is what catches the dialect differences between POSIX
 * and JavaScript regular expressions.
 */

import { execFileSync } from 'node:child_process';
import { readFileSync, readdirSync, statSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';
import { before, describe, test } from 'node:test';

import { grep } from '../src/posix/grep.ts';
import { subst } from '../src/posix/sed.ts';

const here = dirname(fileURLToPath(import.meta.url));

/** Delimiter for the generated sed scripts: the patterns already use /, # and @. */
const DELIMITER = '\u0001';
const engineHome = join(here, '../../../..');

/** GNU sed, which is what the original scripts were written against. */
const GNU_SED = ['gsed', '/usr/bin/sed', 'sed'].find((candidate) => {
	try {
		return execFileSync(candidate, ['--version'], { encoding: 'utf8' }).includes('GNU');
	} catch {
		return false;
	}
});

function gatherSources(directory: string, extensions: string[], limit: number, found: string[] = []): string[] {
	if (found.length >= limit) {
		return found;
	}

	for (const entry of readdirSync(directory)) {
		if (found.length >= limit) {
			break;
		}

		const path = join(directory, entry);

		if (statSync(path).isDirectory()) {
			gatherSources(path, extensions, limit, found);
		} else if (extensions.some((extension) => entry.endsWith(extension))) {
			found.push(path);
		}
	}

	return found;
}

let corpus = '';

before(() => {
	const files = gatherSources(join(engineHome, 'source'), ['.h', '.c'], 60);
	const realLines = files.flatMap((file) => readFileSync(file, 'utf8').split('\n'));

	// Synthetic lines covering the shapes the scripts care about but that the
	// engine's own sources may not happen to contain.
	const edgeCases = [
		'singleton class Timer : ListenerObject',
		'abstract class ListenerObject : Object',
		'static class Math : Object',
		'	virtual void update(uint32 elapsedTime);',
		'	override bool handleMessage(void* telegram);',
		'	static void interruptHandler();',
		'	virtual void render() = 0;',
		'	void method(int a, int b,);',
		'void Timer::update(uint32 elapsedTime)',
		'secure void VUEngine::run(GameState currentGameState)',
		'	Timer::reset();',
		'	this->x = Object::safeCast(that);',
		'extension class Foo;',
		'mutation class Bar;',
		'	void (*callback)(void*);',
		'	uint32 counter;',
		'static inline uint32 helper(void)',
		'inline static uint32 helper2(void)',
		'	class ForwardDeclared;',
		'a*b+c?d|e(f)g{h}i[j]k^l$m.n\\o',
		'',
		'   ',
		'\t\t',
		'*leading star',
		'^caret in middle^',
		'dollar$in$middle',
	];

	corpus = [...realLines, ...edgeCases].join('\n') + '\n';
});

interface PatternCase {
	name: string;
	pattern: string;
	replacement: string;
	global?: boolean;
}

/** Substitutions lifted from processHeaderFile.sh and processSourceFile.sh. */
const SUBSTITUTIONS: PatternCase[] = [
	{ name: 'class name', pattern: '^.*class \\([A-z][A-z0-9]*\\)[ \t]*\\:.*', replacement: '\\1' },
	{ name: 'base class cleanup', pattern: '[^[:alnum:]_-]', replacement: '', global: true },
	{ name: 'class modifiers', pattern: '^\\(.*\\)class .*', replacement: '\\1' },
	{ name: 'brace to newline', pattern: '[{}]', replacement: '\\n' },
	{ name: 'split on semicolon', pattern: ';', replacement: ';\\n', global: true },
	{ name: 'tighten paren', pattern: '[\t ]*(', replacement: '(' },
	{ name: 'strip ampersand backslash', pattern: '&\\\\', replacement: '' },
	{
		name: 'virtual modifier',
		pattern: '^[ \t][ \t]*\\(virtual\\)[ \t][ \t]*\\(.*\\)',
		replacement: '\\2<\\1>',
	},
	{
		name: 'override modifier',
		pattern: '^[ \t][ \t]*\\(override\\)[ \t][ \t]*\\(.*$\\)',
		replacement: '\\2<\\1>',
	},
	{
		name: 'static modifier',
		pattern: '^[ \t][ \t]*\\(static\\)[ \t][ \t]*\\(.*$\\)',
		replacement: '\\2<\\1>',
	},
	{
		name: 'virtual declaration',
		pattern: '\\(^.*\\)[ \t][ \t]*\\([a-z][A-z0-9]*\\)(\\([^;]*;\\)<virtual>.*',
		replacement: ' __VIRTUAL_DEC(ClassName,\\1,\\2,ClassName,\\3',
		global: true,
	},
	{ name: 'trailing comma paren', pattern: ',[ \t]*)[ \t]*;', replacement: ');', global: true },
	{
		name: 'virtual set',
		pattern: '^.*[ \t][ \t]*\\([a-z][A-z0-9]*\\)(.*',
		replacement: ' __VIRTUAL_SET(ClassName,Timer,\\1);',
		global: true,
	},
	{ name: 'pure virtual', pattern: ')[ \t]*=[ \t]*0[ \t]*;', replacement: ');', global: true },
	{
		name: 'inject this parameter',
		pattern: '\\(^.*[ \t][ \t]*\\)\\([a-z][A-z0-9]*\\)(\\(.*\\)',
		replacement: '\\1Timer_\\2(void* _this,\\3',
		global: true,
	},
	{
		name: 'drop this for static',
		pattern: '\\(^.*\\)void\\* _this,\\(.*\\)<static>',
		replacement: '\\1\\2',
		global: true,
	},
	{ name: 'virtual marker to tab', pattern: '<virtual>', replacement: '\t' },
	{
		name: 'owned method remap',
		pattern: '^\\([A-Z][A-z]*\\)_\\(.*\\)',
		replacement: 'Timer_\\2 \\1_\\2',
		global: true,
	},
	{ name: 'strip static keyword', pattern: 'static[ \t]\\+', replacement: '', global: true },
	{
		name: 'scope resolution',
		pattern: '\\([A-Z][A-z0-9]*\\)::\\([a-z][A-z0-9]*\\)',
		replacement: '\\1_\\2',
		global: true,
	},
	{ name: 'static inline', pattern: 'static[ \t]inline[ \t]', replacement: 'inline ', global: true },
	{ name: 'inline static', pattern: 'inline[ \t]static[ \t]', replacement: 'inline ', global: true },
	{
		name: 'forward declaration',
		pattern: '^[ \t]*class[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*;',
		replacement: '#ifndef FORWARD_DECLARE_\\1\\n #define FORWARD_DECLARE_\\1\\n __FORWARD_CLASS(\\1);\\n #endif\\n',
	},
	{ name: 'space before scope', pattern: '\\([A-z][A-z0-9]*::[a-z][A-z0-9]*\\)', replacement: ' \\1', global: true },
	{ name: 'mark block start', pattern: '{', replacement: '{<START_BLOCK>', global: true },
	{ name: 'mark trailing comma', pattern: ',[ \t]*$', replacement: ',<\u00c2\u00b7>', global: true },
	{ name: 'mark static line', pattern: '.*static.*', replacement: '&<%>', global: true },
	{ name: 'mark secure line', pattern: '^secure[ \t].*', replacement: '&<#>', global: true },
	{ name: 'prefix line mark', pattern: '.*', replacement: '@N@&', global: true },
	{
		name: 'declaration middle',
		pattern: '\\(!DECLARATION_MIDDLE!_[^(]*\\)(\\([^%{]*{\\)',
		replacement: '\\1(void* _this __attribute__((unused)), \\2',
		global: true,
	},
	{ name: 'drop trailing comma', pattern: ',[ \t]*)', replacement: ')', global: true },
	{
		name: 'static block marker',
		pattern: '\\(<%>[^;!]*\\?{\\)\\(<START_BLOCK><method>\\)',
		replacement: '\\1<STATIC>\\2',
		global: true,
	},
	{
		name: 'strip declaration marker',
		pattern: '<[%]*DECLARATION>[ \t]*\\(static\\|secure\\)[ \t][ \t]*',
		replacement: ' ',
		global: true,
	},
	{ name: 'method call split', pattern: '\\([A-Za-z0-9]*::[^(]*\\)(', replacement: '<\\1>\\n', global: true },
	{ name: 'extract angle content', pattern: '.*<\\(.*\\)>', replacement: '\\1', global: true },
	{ name: 'strip to scope', pattern: '::.*', replacement: '', global: true },
	{
		name: 'new operator',
		pattern: '\\([^A-z0-9]\\)new[ \t][ \t]*\\([A-Z][A-z0-9]*\\)[ \t]*(',
		replacement: '\\1\\2_new(',
		global: true,
	},
	{
		name: 'delete operator',
		pattern: '\\([^A-z0-9]\\)delete[ \t][ \t]*\\(.*\\);',
		replacement: '\\1__DELETE(\\2);',
		global: true,
	},
	{
		name: 'friend class',
		pattern: '[ \t]*friend[ \t][ \t]*class[ \t][ \t]*\\([A-z0-9][A-z0-9]*\\)',
		replacement: '__CLASS_FRIEND_DEFINITION(\\1)',
	},
	{
		name: 'base method call',
		pattern: 'Base_\\([A-z][A-z0-0][A-z0-0]*\\)(',
		replacement: '__CALL_BASE_METHOD(Container,\\1, ',
		global: true,
	},
	{ name: 'safe cast', pattern: '\\([A-Z][A-z0-9]*\\)_safeCast[ \t]*(', replacement: '__SAFE_CAST(\\1, ', global: true },
	{
		name: 'overrides check',
		pattern: '\\([A-Z][A-z0-9]*\\)_overrides[ \t]*(',
		replacement: '__OVERRIDES_METHOD(\\1, ',
		global: true,
	},
];

/** grep patterns lifted from the same scripts. */
const GREPS: { name: string; pattern: string; invert?: boolean }[] = [
	{
		name: 'class declaration',
		pattern: '^[ \t]*[A-z0-9\\!]*[ \t]*class[ \t]\\+[A-Z][A-z0-9]*[ \t]*:[ \t]*[A-Z][A-z0-9]*',
	},
	{ name: 'comment lines', pattern: '^[ \t]*[\\*//]\\+.*', invert: true },
	{ name: 'function pointers', pattern: '^[ \t\\*A-z0-9]\\+[ \t]*([ \t]*\\*', invert: true },
	{ name: 'method declarations', pattern: '(.*)[ \t=0]*;[ \t]*' },
	{ name: 'attribute exclusion', pattern: '^[ \t\\*A-z0-9]\\+[ \t]*([ \t]*[^\\*]', invert: true },
	{ name: 'virtual marker', pattern: '<virtual>' },
	{ name: 'override or virtual', pattern: '<override>\\|<virtual>' },
	{ name: 'pure virtual', pattern: ')[ \t]*=[ \t]*0[ \t]*;', invert: true },
	{ name: 'modifiers', pattern: '<static>\\|<virtual>\\|<override>', invert: true },
	{ name: 'destructor definition', pattern: '^.*::[ \t]*destructor[ \t]*(' },
	{ name: 'extension class', pattern: '^extension[ \t][ \t]*class[ \t][ \t]*' },
	{ name: 'constructor declaration', pattern: 'void[ \t]\\+Timer_constructor[ \t]*(.*);' },
	{ name: 'singleton definition', pattern: '#define[ \t][ \t]*.*SINGLETON.*(' },
	{ name: 'blank lines', pattern: '^[[:space:]]*$' },
	{ name: 'declaration marker', pattern: 'DECLARATION>' },
];

describe('POSIX regex translation', () => {
	test('GNU sed is available for differential testing', () => {
		assert.ok(GNU_SED, 'GNU sed (gsed) is required; install it with `brew install gnu-sed`');
	});

	for (const { name, pattern, replacement, global } of SUBSTITUTIONS) {
		test(`s/${name}/ matches GNU sed`, () => {
			const script = `s${DELIMITER}${pattern}${DELIMITER}${replacement}${DELIMITER}${global === true ? 'g' : ''}`;
			const expected = execFileSync(GNU_SED!, [script], { input: corpus, encoding: 'utf8' });
			const actual = subst(corpus, pattern, replacement, { global });

			assert.equal(actual, expected);
		});
	}

	for (const { name, pattern, invert } of GREPS) {
		test(`grep ${name} matches GNU sed`, () => {
			// Expressed through sed's line selection so the same GNU engine
			// decides both sides, and so an absent GNU grep is not a blocker.
			const script = `\\${DELIMITER}${pattern}${DELIMITER}${invert === true ? '!' : ''}p`;
			const expected = execFileSync(GNU_SED!, ['-n', script], { input: corpus, encoding: 'utf8' });
			const actual = grep(corpus, pattern, { invert });

			assert.equal(actual.length === 0 ? '' : actual.join('\n') + '\n', expected);
		});
	}
});
