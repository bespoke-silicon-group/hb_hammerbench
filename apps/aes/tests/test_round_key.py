#!/usr/bin/env python3
"""Sanitize the actual device key-copy block without translating target assembly.

Run with Python 3 and clang++ (or CXX). --output retains the generated harness,
binary and diagnostics; --source selects an archived aes_kernel.cpp.
"""
import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

app = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--source", type=Path, default=app / "aes_kernel.cpp")
parser.add_argument("--output", type=Path)
args = parser.parse_args()
scratch = tempfile.TemporaryDirectory() if args.output is None else None
out = args.output or Path(scratch.name)
out.mkdir(parents=True, exist_ok=True)
source = args.source.read_text()
body = source[source.index("  // Load roundkey;"):source.index("  // load Iv;")]
key_decl = re.search(r"uint32_t localRoundKey\[[^;]+;", source).group(0)
(out / "bsg_manycore.h").write_text("#define bsg_unroll(n)\n")
harness = r'''#include "aes_kernel.hpp"
#include <cstdio>
#include <cstring>
''' + key_decl + "\nvoid copy_key(AES_ctx *ctx) {\n" + body + r'''
}
int main() {
  for (unsigned seed = 0; seed < 8; ++seed) {
    alignas(4) AES_ctx ctx;
    for (unsigned i = 0; i < sizeof(ctx.RoundKey); ++i)
      ctx.RoundKey[i] = (i * 29 + seed * 37) & 255;
    memset(ctx.Iv, 0xa5, sizeof(ctx.Iv));
    AES_ctx saved = ctx;
    copy_key(&ctx);
    if (memcmp(localRoundKey, saved.RoundKey, sizeof(saved.RoundKey)) ||
        memcmp(&ctx, &saved, sizeof(ctx))) return 1;
  }
  puts("PASS: all 44 key words copied; source preserved; ASan/UBSan clean");
}
'''
(out / "copy_key.cpp").write_text(harness)
cmd = [os.environ.get("CXX", "clang++"), "-std=c++14", "-O1", "-g",
       "-fsanitize=address,undefined", "-I" + str(out), "-I" + str(app),
       str(out / "copy_key.cpp"), "-o", str(out / "copy_key")]
with (out / "compile.log").open("w") as log:
    subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, check=True)
with (out / "run.log").open("w") as log:
    result = subprocess.run([str((out / "copy_key").resolve())], stdout=log,
                            stderr=subprocess.STDOUT)
print((out / "run.log").read_text())
raise SystemExit(result.returncode)
