#!/bin/bash

set -eu

ppapi_dir=~/devel/newgit-chromium/src

# This doesn't work with normal PNaCl because the PPAPI shims aren't present.

# pnacl-clang -O2 -Wall -I$ppapi_dir ppapi_hellow.c -o ppapi_hellow.pexe -s
# pnacl-translate -arch x86-64 ppapi_hellow.pexe -o ppapi_hellow.nexe.x86-64 \
#     --allow-llvm-bitcode-input

nacl x86_64-nacl-gcc -O2 -Wall -I$ppapi_dir ppapi_hellow.c -nostdlib \
    -o ppapi_hellow.nexe.x86-64
