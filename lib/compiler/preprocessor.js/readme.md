# Virtual C preprocessor

Virtual C is VUEngine's C++-like dialect: classes with single inheritance,
virtual methods, `static`, `abstract`, `singleton`, `secure`, `extension` and
`mutation` modifiers, and `new`/`delete`. This directory holds the transpiler
that turns it into the plain C that `v810-gcc` compiles.

It was a set of bash scripts driving `sed`, `grep` and `awk`. It is now
TypeScript, bundled for QuickJS. The scripts are still in [`../preprocessor`](../preprocessor), for
reference and for differential testing; nothing in a build calls them any more.

## Layout

```
lib/compiler/preprocessor.js/     this: the TypeScript implementation
  dist/preprocessor.mjs           the committed bundle builds run
  src/                            the sources it is built from
  src/platform/                   the only place a runtime is mentioned
  test/                           unit and differential tests

lib/compiler/preprocessor/        the shell implementation it replaced
```

The makefiles run the bundle directly, through a variable each of them defines
next to `ENGINE_HOME`:

```make
PREPROCESSOR = "$(JS_RUNTIME)" $(ENGINE_HOME)/lib/compiler/preprocessor.js/dist/preprocessor.mjs
```

so a call site reads `$(PREPROCESSOR) header -e $(ENGINE_HOME) -i $< …`. The
flags are unchanged from the shell scripts, which is what lets the two be
compared. Reverting to the shell implementation means pointing those call sites
back at `lib/compiler/preprocessor/*.sh`; `test/ab/verify.sh` does exactly that
to build the reference engine it compares against.

The bundle is committed, so building a game needs no `npm install` and no
compile step — only the runtime.

### The JavaScript runtime

Builds run the preprocessor on **QuickJS**. 
The makefiles resolve it while they are being read:

```make
JS_RUNTIME ?= qjs
```

and stop with a clear message if it is not there, rather than failing once per
source file with make's opaque `No such file or directory`:

```
*** JavaScript runtime not found at 'qjs'. Pass it explicitly,
    e.g. make JS_RUNTIME=/path/to/qjs.  Stop.
```

VUEngine Studio ships a QuickJS binary and passes its path; a shell build uses
whatever `qjs` is on `PATH`. The value is overridable from the environment or
the command line, and make propagates it to sub-makes automatically — as a
command-line variable through `MAKEFLAGS`, or as an environment variable through
the environment — so it only has to be set on the outermost invocation:

```sh
make JS_RUNTIME=/path/to/qjs …     # command line
JS_RUNTIME=/path/to/qjs make …     # environment
```

It is quoted at its point of use, so a path containing spaces is fine. The rest
of these makefiles do not quote their paths, so `ENGINE_HOME` and the project
path still must not contain any; VUEngine Studio enforces that in
`checkPathsForSpaces`.

#### Why QuickJS rather than Node

A build should not require developers to install anything, and VUEngine Studio
should stay self-contained. Three approaches were measured:

| | Size | Works in WSL |
| --- | --- | --- |
| Bundle Node | ~110 MB per platform | yes |
| Reuse Electron's binary (`ELECTRON_RUN_AS_NODE=1`) | 0 MB | **no** — a Windows `.exe` cannot be the runtime for a build in a Linux userland |
| Bundle QuickJS | **~1 MB** per platform | yes |

Two things had to be true for QuickJS to be safe, and both were measured rather
than assumed:

- **It agrees with V8.** The whole port rests on translating POSIX regular
  expressions into JavaScript ones, and QuickJS uses Bellard's libregexp rather
  than V8's engine. Running all 57 patterns from `test/regex.test.ts` over the
  engine's own sources under both produced **byte-identical output** across
  37 MB.
- **It is not slower.** QuickJS interprets where V8 compiles, and on a
  regex-saturated microbenchmark it is ~5.8× slower. But it starts in 1.8 ms
  against Node's 18.6 ms, and this preprocessor is invoked once per file, so
  start-up dominates: a full `preprocessClasses` for a project came out at
  **23.1 s on QuickJS against 26.2 s on Node**, and 49 s for the shell scripts
  it replaced.

That second point would invert if the per-file model were ever replaced by a
single batch run, where start-up is paid once and compute dominates. Re-measure
before assuming it still holds.

#### Node is still the development runtime

Only the *build* is on QuickJS. Node runs the sources directly by stripping
types, with no bundle and no build step:

```sh
node src/main.ts compiling-info path/to/File.c
node --test test/*.test.ts
```

That is why `src/platform/platform.ts` — the Node implementation — is the one
checked in as the default import, and why the tests can use `node:test` and
`node:child_process` to diff against GNU sed.

#### Host access

Everything either runtime provides — the file system, process identity,
arguments, environment, standard streams — is reached through
`src/platform/platform.ts`. Nothing else in `src/` mentions `node:` or
`process`.

```
src/platform/platform.ts           the Node implementation, used by tests
src/platform/platform.quickjs.ts   the QuickJS implementation, used by builds
src/platform/quickjs.d.ts          types for QuickJS's `os` and `std` modules
```

`build.mjs` substitutes the QuickJS one with a small esbuild plugin. The bundle
must be named `.mjs`: that is how `qjs` knows to treat it as a module.

Adding a call to a host facility means adding it to *both* implementations, or
the tests and the build will diverge. QuickJS reports errors as negative errno
values or a trailing error field rather than by throwing, and it has no
`console.error`, no `process.ppid` and no `process.getuid` — the latter two
reach only a diagnostic line in a lock file and are stubbed with zero.

### Commands

| Subcommand | Replaces | Does |
| --- | --- | --- |
| `header` | `processHeaderFile.sh` | A `.h`: emits vtable macros, the attribute list and prototypes; maintains the class hierarchy and method dictionaries |
| `source` | `processSourceFile.sh` | A `.c`: rewrites method definitions and call sites, injects the class definition and allocator |
| `setup-classes` | `setupClasses.sh` | Generates the C that installs every class' vtable at start-up |
| `gcc-output` | `processGCCOutput.sh` | Rewrites compiler diagnostics to point at the original sources |
| `compiling-info` | `printCompilingInfo.sh` | Prints a build progress line |

`cleanSyntax.sh` and `portHeader.sh` stay in `lib/compiler/preprocessor`. They
are one-shot migration tools for converting pre-Virtual C sources, are shell
rather than TypeScript, and were not ported; `makefile-game` still calls
`cleanSyntax.sh` there.

## Working on it

```sh
npm install        # once, for the dev tooling
npm run build      # rebuild dist/preprocessor.mjs after changing src/
npm run typecheck
npm test
```

Always commit `dist/preprocessor.mjs` along with the `src/` change that produced
it, or builds will silently keep using the old bundle while the tests — which run
the sources directly — report success.

The sources are written so Node can run them directly by stripping types, which
is what the tests import. `erasableSyntaxOnly` is on to keep it that way, so no
enums, namespaces or parameter properties.

## Why it is shaped like this

The shell version's behaviour lives in about a hundred `sed` and `grep`
patterns, and POSIX regular expressions differ from JavaScript's in ways that
quietly change what a pattern matches. Rather than hand-translate them, the
patterns are copied verbatim from the scripts and run through a POSIX-to-JS
translator in [`src/posix/`](src/posix/), which also reimplements the parts of
`sed`, `grep`, `cut`, `tr`, `head`, `tail`, `wc`, `sort` and `awk` those scripts
relied on.

That layer is worth understanding before changing anything, because it encodes
several traps:

- **`(`, `)`, `{`, `}`, `|`, `+`, `?`** are literals in a Basic Regular
  Expression and operators only when backslashed. Extended expressions invert
  this.
- **A backslash inside a bracket expression is an ordinary character**, so
  `[^\*]` excludes backslash *and* asterisk, where JavaScript reads it as
  excluding only the asterisk.
- **`s///g` discards a null match that starts where the previous match ended.**
  `s/.*/X/g` yields `X` in sed but `XX` via JavaScript's `replace`.
- **`&` in a replacement is the matched text**; `\&` is a literal ampersand.
- **An unquoted expansion collapses whitespace.** `cut … <<< $constructor`
  turns `constructor(Spec* spec,  int16 id)` into one with a single space, and
  the generated allocator inherits that.
- **`make` exports its command-line variables into every recipe's environment**,
  and the scripts left several variables uninitialised so they picked them up
  from there. `processHeaderFile` recursing into a base class omits `-x`, yet
  still sees `GAME_NAME` — and the generated dependency paths depend on it.

Each of these has a regression test in [`test/shell.test.ts`](test/shell.test.ts)
naming the symptom it caused.

## Verification

Two layers.

`test/regex.test.ts` runs every pattern taken from the shell scripts through
both GNU sed and this implementation, over a corpus built from the engine's own
sources, and requires the outputs to match byte for byte.

`test/ab/verify.sh` runs a real `make preprocessClasses` for a whole project —
engine plus every plugin — twice, once against an engine copy using the shell
scripts from `../preprocessor` and once against this one, then diffs what both
produced:

```sh
PATH=$(brew --prefix gnu-sed)/libexec/gnubin:$PATH \
  JS_RUNTIME=/path/to/qjs \
  bash test/ab/verify.sh /path/to/a/project    # e.g. vuengine-barebone
```

Results as of the port: the engine's 107 headers and 80 source files come out
byte-identical, as do the class hierarchy, all method dictionaries, and all
generated make dependency fragments.

Use `MAKE_JOBS=1`. The shell implementation is **not deterministic under
parallel make** — two consecutive runs of it disagree with each other by ~160
lines, because the order in which parallel jobs append to the shared
`classesHierarchy.txt` varies. There is nothing to diff against under `-j`.

### sed dialect

The shell preprocessor produces **materially different C depending on which
`sed` is first on `PATH`**. BSD sed (the `/usr/bin/sed` macOS ships) does not
support `\|` alternation or `\?`, so patterns such as
`s/\([^A-z0-9]*\)\(static\|secure\)[ \t]/\1 /g` silently never match. Under BSD
sed the generated prototypes keep their `static` keyword and static methods get
a `this` guard injected; under GNU sed they do not.

GNU sed is the intended dialect — it is what the committed build artefacts in
existing projects were produced with, and what this port implements. On macOS
that means homebrew's `gnu-sed` must shadow the system one. The port removes the
variable entirely: its behaviour no longer depends on what is installed.

## Deviations

Three, all deliberate.

1. **Stale locks are reclaimed.** The shell version's check read
   `[ ! kill -0 $PID ]`, which is a shell syntax error rather than a process
   liveness test, so it always evaluated false: a lock left behind by a killed
   build was never cleaned up and every later build blocked on it until the wait
   loop gave up. The liveness check the code plainly intended is implemented.
   This changes only how long the preprocessor waits, never what it writes.

2. **`setupClasses.c` lists components in a stable order.** The shell version
   emitted them in `find` order, which is filesystem-dependent; this sorts. The
   file's contents are otherwise identical, and the order carries no meaning —
   each entry is an independent vtable-setup call. This is the one file
   `verify.sh` reports as differing.

3. **No `sed -i.b` backups.** The shell version left a `.b` copy of every file
   beside it in the working folder. Nothing reads them.

## Known bugs preserved

Reproduced deliberately, because changing them would change the generated C:

- **The ancestor walk stops after the direct base class.** A quoting error in
  the original's `grep` — an unbalanced quote that turned `extension` into a
  filename operand — made the loop exit on its first iteration. The recorded
  hierarchy lines still show the empty field it leaves behind
  (`VUEngine:ListenerObject:::singleton`). Transitive inheritance works anyway,
  because each class copies its base class' already-transitive dictionaries.
- **`processHeaderFile` ignores `-w`**, reassigning the working folder to a
  hard-coded `build/working` right after parsing arguments. Everything is
  therefore relative to the game directory.
- **The recursive base-class call omits `-x`**, so it depends on `GAME_NAME`
  reaching it through the environment.
- Dead code that never runs: the `firstMethodLine` padding, the `singleton!`
  final-class check, `isStaticClass` in the source pass, and the
  `PRINT_DEBUG_OUTPUT` block guarded by an `anyMethodVirtualized` that is never
  set.

## Next

This is a faithful port; it kept the original's one-process-per-file structure
and its lock-directory coordination. The redesign that structure exists to
enable — a single process that parses every class up front, orders them by
hierarchy and holds the dictionaries in memory, with no locks and no polling —
is the follow-up. `src/` is split into pure transforms and thin command wrappers
with that in mind.
