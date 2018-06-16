#!/bin/sh
HASK_URL=https://get.haskellstack.org
ROOT="$(dirname "$(readlink -f "$0")")"
STACK_DIR="$ROOT/.stack"
SBIN="$STACK_DIR/stack"
HASK_SCRIPT="$ROOT/h.sh"

echo "This script will build and install the Haskell component."
echo "You may be prompted to install packages to build Haskell."
echo "If you wish to inspect that script, see: $HASK_URL"
echo "---------------------------------------------------------"

trap 'command rm -r "$STACK_DIR" "$HASK_SCRIPT" "$ROOT/.stack-work" "$ROOT/src/.stack-work" >/dev/null 2>&1' EXIT

if command -v curl >/dev/null 2>&1; then
    curl -sSL "$HASK_URL" > "$HASK_SCRIPT"
elif command -v wget >/dev/null 2>&1; then
    wget -qO- "$HASK_URL" > "$HASK_SCRIPT"
else
    echo "I need one of the following:"
    echo " - curl"
    echo " - wget"
    exit 1
fi

sh "$HASK_SCRIPT" -d "$STACK_DIR"
$SBIN setup
$SBIN build
$SBIN install

echo "---------------------------------------------------------"
echo "To use Haskell version, put following in your ~/.zshrc"
echo "    export GIT_PROMPT_EXECUTABLE=haskell"
