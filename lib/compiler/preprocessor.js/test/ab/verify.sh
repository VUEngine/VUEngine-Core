#!/bin/bash
#
# Differential verification of the TypeScript preprocessor against the shell
# implementation still kept in lib/compiler/preprocessor.
#
# It runs a real `make preprocessClasses` for a whole project — the engine plus
# every plugin the project uses — twice: once against an engine copy whose
# preprocessor is the original shell scripts, and once against one using the
# TypeScript port. Both trees are then normalised (the sandbox paths and the
# project name differ by construction) and diffed.
#
#   usage: verify.sh <project-path> [sandbox-root] [make-jobs]
#
# Use `make-jobs` of 1. The shell implementation is not deterministic under
# parallel make — two consecutive runs of it disagree with each other, because
# the order in which jobs append to the shared classesHierarchy.txt varies — so
# a parallel run cannot be diffed meaningfully against anything.
#
# Requires GNU sed on PATH. The shell implementation produces materially
# different C under BSD sed; see readme.md.
set -u

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
PREPROCESSOR=$(cd "$HERE/../.." && pwd)
ENGINE_SRC=$(cd "$PREPROCESSOR/../../.." && pwd)
VUENGINE_ROOT=$(cd "$ENGINE_SRC/.." && pwd)

PROJECT_SRC=${1:?usage: verify.sh <project-path> [sandbox-root] [make-jobs]}
ROOT=${2:-$(mktemp -d)}
JOBS=${3:-1}

PLATFORMS_FOLDER=${PLATFORMS_FOLDER:-$VUENGINE_ROOT/platforms}
PLUGINS_FOLDER=${PLUGINS_FOLDER:-$VUENGINE_ROOT/plugins}

if ! sed --version 2>/dev/null | grep -q GNU; then
	echo "error: GNU sed must be on PATH (try: PATH=\$(brew --prefix gnu-sed)/libexec/gnubin:\$PATH)" >&2
	exit 1
fi

# The preprocessor runs on QuickJS. JS_RUNTIME reaches the makefiles through the
# environment; check it here so the failure names the cause.
if ! command -v "${JS_RUNTIME:-qjs}" > /dev/null 2>&1; then
	echo "error: no QuickJS at '${JS_RUNTIME:-qjs}'. Set JS_RUNTIME=/path/to/qjs" >&2
	exit 1
fi

echo "project:  $PROJECT_SRC"
echo "sandbox:  $ROOT"
echo "jobs:     $JOBS"
echo

rm -rf "$ROOT"
mkdir -p "$ROOT"

# Two equally fresh engine copies, so that directory creation order — which is
# what `find` walks in — cannot differ between the flavours.
echo "Preparing engine copies..."
for flavour in bash ts; do
	rsync -a --exclude .git --exclude node_modules --exclude build "$ENGINE_SRC/" "$ROOT/engine-$flavour/"
done
# The shell implementation is already present in the copy, at
# lib/compiler/preprocessor; it just is not what the makefiles call any more.
# Point the reference copy's makefiles back at it, one subcommand at a time.
sed -i \
	-e 's@\$(PREPROCESSOR) header@bash $(ENGINE_HOME)/lib/compiler/preprocessor/processHeaderFile.sh@g' \
	-e 's@\$(PREPROCESSOR) source@bash $(ENGINE_HOME)/lib/compiler/preprocessor/processSourceFile.sh@g' \
	-e 's@\$(PREPROCESSOR) setup-classes@bash $(ENGINE_HOME)/lib/compiler/preprocessor/setupClasses.sh@g' \
	-e 's@\$(PREPROCESSOR) gcc-output@bash $(ENGINE_HOME)/lib/compiler/preprocessor/processGCCOutput.sh@g' \
	-e 's@\$(PREPROCESSOR) compiling-info@bash $(ENGINE_HOME)/lib/compiler/preprocessor/printCompilingInfo.sh@g' \
	"$ROOT/engine-bash/lib/compiler/make/makefile-preprocess" \
	"$ROOT/engine-bash/lib/compiler/make/makefile-compile" \
	"$ROOT/engine-bash/lib/compiler/make/makefile-compile-randomize"

if grep -q 'PREPROCESSOR)' "$ROOT/engine-bash/lib/compiler/make/makefile-preprocess"; then
	echo "error: the reference makefiles still reference \$(PREPROCESSOR)" >&2
	exit 1
fi

run_one() {
	local flavour=$1
	local engine=$ROOT/engine-$flavour
	local game=$ROOT/game-$flavour

	rsync -a --exclude build "$PROJECT_SRC/" "$game/"
	mkdir -p "$game/build"

	( cd "$game" && make preprocessClasses \
		-f "$engine/lib/compiler/make/makefile-game" \
		-e ENGINE_FOLDER="$engine" \
		-e PLATFORMS_FOLDER="$PLATFORMS_FOLDER" \
		-e PLUGINS_FOLDER="$PLUGINS_FOLDER" \
		-e GAME_CONFIG_MAKE_FILE="$game/config.make" \
		-e TYPE=release \
		-e MAKE_JOBS="$JOBS" ) > "$ROOT/$flavour.stdout" 2> "$ROOT/$flavour.stderr"

	echo "  $flavour finished (exit $?)"
}

echo "=== running the shell implementation ==="
time run_one bash
echo "=== running the TypeScript implementation ==="
time run_one ts

# Each run lives in its own directory and is named after it, and those names end
# up inside the generated dependency files and setup functions.
normalise() {
	local flavour=$1
	local to=$ROOT/normalised-$flavour

	rm -rf "$to"
	cp -R "$ROOT/game-$flavour/build/working" "$to"

	find "$to" -depth -name "game-$flavour*" \
		| while IFS= read -r entry; do
			mv "$entry" "$(dirname "$entry")/$(basename "$entry" | sed -e "s@game-$flavour@game@")"
		done

	find "$to" -type f -print0 \
		| xargs -0 sed -i \
			-e "s@$ROOT/game-$flavour@<GAME>@g" \
			-e "s@$ROOT/engine-$flavour@<ENGINE>@g" \
			-e "s@game-$flavour@<GAMENAME>@g" \
			-e "s@game$flavour@<GAMEFN>@g"
}

normalise bash
normalise ts

echo
echo "=== diff ==="
# Class logs record lock chatter and timing. The shell version's `sed -i.b`
# backups have no counterpart, because the port does not write files it has no
# use for.
diff -r -x logs -x '*.log' -x '*.b' -x '*-e' \
	"$ROOT/normalised-bash" "$ROOT/normalised-ts" > "$ROOT/tree.diff" 2>&1

CONTENT=$(grep -c '^[<>]' "$ROOT/tree.diff" || true)

if [ "$CONTENT" -eq 0 ] && [ ! -s "$ROOT/tree.diff" ]; then
	echo "IDENTICAL"
	exit 0
fi

echo "differing files:"
grep '^diff -r' "$ROOT/tree.diff" | sed -E 's@.*/normalised-ts/@  @'
grep '^Only in' "$ROOT/tree.diff" | sed -E 's@^@  @'
echo
echo "content diff lines: $CONTENT"
echo "full diff: $ROOT/tree.diff"

# objects/*/setupClasses.c is expected to differ in the order of its
# declarations only; see readme.md.
if [ "$(grep '^diff -r' "$ROOT/tree.diff" | grep -cv 'setupClasses\.c')" -eq 0 ] \
	&& diff <(sort "$ROOT/normalised-bash/objects/release/setupClasses.c") \
		<(sort "$ROOT/normalised-ts/objects/release/setupClasses.c") > /dev/null 2>&1; then
	echo
	echo "OK: the only difference is the known ordering of setupClasses.c"
	exit 0
fi

exit 1
