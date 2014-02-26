#!/bin/bash
# Copyright (c) 2014 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -eu

ppapi_dir=~/devel/newgit-chromium/src

mkdir -p out
mkdir -p out-nonsfi

cp ppapi_hellow.html ppapi_hellow.nmf out
cp ppapi_hellow.html ppapi_hellow.nmf out-nonsfi

# This doesn't work with normal PNaCl because the PPAPI shims aren't present.

# pnacl-clang -O2 -Wall -I$ppapi_dir ppapi_hellow.c -o ppapi_hellow.pexe -s
# pnacl-translate -arch x86-64 ppapi_hellow.pexe -o ppapi_hellow.nexe.x86-64 \
#     --allow-llvm-bitcode-input

nacl x86_64-nacl-gcc -O2 -Wall -I$ppapi_dir ppapi_hellow.c -nostdlib \
    -o out/ppapi_hellow.nexe.x86-64

gcc -Wall -I$ppapi_dir \
    ppapi_hellow.c \
    -nostdlib -shared -fPIC -fvisibility=hidden \
    -o out-nonsfi/ppapi_hellow.nexe.x86-64
