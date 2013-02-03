#!/bin/sh

REMOTE_DIR="~/cs452/microkernel"

PROJECT_DIR=.
MAKE_CMD="make"

# If from Sublime
if [[ $# == 1 ]]; then
	PROJECT_DIR=$1
	MAKE_CMD="make >/dev/null"
fi

cd $PROJECT_DIR

echo "=== CLEAN ==="

ssh uw "rm -rf $REMOTE_DIR && mkdir -p $REMOTE_DIR"

echo "=== COPY ==="

scp -r `ls -1 . | grep -v .git` uw:$REMOTE_DIR

echo "=== COMPILE ==="

ssh uw "cd $REMOTE_DIR \
		&& make clean >/dev/null \
		&& $MAKE_CMD \
		&& make install >/dev/null \
		&& echo \"=== SUCCESS ===\""

