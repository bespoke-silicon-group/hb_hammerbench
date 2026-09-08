#!/usr/bin/env python3
"""Check the actual rolling recurrence against independent full DP matrices.

Python 3 and clang++ (or CXX) are required. Tests include first/last-position
matches, gaps, seeded random pairs, all 512 fixture scores, rejected sizes,
and malformed/missing fixture records. --output retains compiler/run artifacts.
--source can select an archived kernel.cpp to reproduce the original failure.
--regenerate-output explicitly replaces output32 with independent oracle scores.
"""
import argparse
import json
import os
from pathlib import Path
import random
import subprocess
import tempfile


def reference(a, b):
    h = [[0] * (len(b) + 1) for _ in range(len(a) + 1)]
    e = [[-1000] * (len(b) + 1) for _ in range(len(a) + 1)]
    f = [[-1000] * (len(b) + 1) for _ in range(len(a) + 1)]
    best = 0
    for i in range(1, len(a) + 1):
        for j in range(1, len(b) + 1):
            e[i][j] = max(e[i][j-1] - 1, h[i][j-1] - 3)
            f[i][j] = max(f[i-1][j] - 1, h[i-1][j] - 3)
            h[i][j] = max(0, e[i][j], f[i][j],
                          h[i-1][j-1] + (1 if a[i-1] == b[j-1] else -3))
            best = max(best, h[i][j])
    return best


app = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path, default=app / 'kernel.cpp')
parser.add_argument('--output', type=Path)
parser.add_argument('--regenerate-output', action='store_true')
args = parser.parse_args()
scratch = tempfile.TemporaryDirectory() if args.output is None else None
out = args.output or Path(scratch.name)
out.mkdir(parents=True, exist_ok=True)
compiler = os.environ.get('CXX', 'clang++')
flags = ['-std=c++14', '-O1', '-g', '-fsanitize=address,undefined',
         '-I' + str(app), '-DNUM_SEQ=4', '-Dbsg_tiles_X=2', '-Dbsg_tiles_Y=2',
         '-DNUM_POD_X=1']

def compile_test(name, source, extra=(), expect_success=True):
    cpp = out / (name + '.cpp')
    cpp.write_text(source)
    cmd = [compiler] + flags + list(extra) + [str(cpp), '-o', str(out / name)]
    with (out / (name + '-compile.log')).open('w') as log:
        result = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT)
    assert (result.returncode == 0) == expect_success, cmd
    return (out / name).resolve()


source = args.source.read_text()
arrays = source[source.index('int E_spm'):source.index('inline int max')]
recurrence = source[source.index('inline int max'):source.index('// Kernel main;')]
harness = r'''#include <cstdint>
#include <cstdio>
#include <cstring>
#include "sw_parameters.hpp"
''' + arrays + recurrence + r'''
int main() {
  char a[33], b[33];
  while (scanf("%32s %32s", a, b) == 2) {
    memset(E_spm, 0, sizeof(E_spm)); memset(F_spm, 0, sizeof(F_spm));
    memset(H_spm, 0, sizeof(H_spm)); memset(H_prev_spm, 0, sizeof(H_prev_spm));
    int result;
    align((uint8_t*)a, (uint8_t*)b, &result);
    printf("%d\n", result);
  }
}
'''
binary = compile_test('alignment', harness)
pairs = [('A'*32, 'A'*32), ('A'+'C'*31, 'A'+'G'*31),
         ('A'*31+'C', 'G'*31+'C'), ('A'+'C'*31, 'G'*31+'A'),
         ('C'*31+'A', 'A'+'G'*31), ('A'*32, 'C'*32),
         ('A'*12+'C'*8+'T'*12, 'A'*12+'G'*8+'T'*12),
         ('A'*15+'C'*2+'T'*15, 'A'*15+'T'*15+'G'*2)]
rng = random.Random(20260908)
pairs += [(''.join(rng.choices('ACGT', k=32)),
           ''.join(rng.choices('ACGT', k=32))) for _ in range(256)]
query = (app / 'dna-query32.fasta').read_text().split()[1::2]
ref = (app / 'dna-reference32.fasta').read_text().split()[1::2]
fixture = list(map(int, (app / 'output32').read_text().split()))
assert len(fixture) == 512 and len(query) >= 512 and len(ref) >= 512
fixture_pairs = list(zip(query[:512], ref[:512]))
fixture_scores = [reference(a, b) for a, b in fixture_pairs]
if args.regenerate_output:
    (app / 'output32').write_text(''.join(str(x) + '\n' for x in fixture_scores))
    fixture = fixture_scores
assert fixture == fixture_scores, 'output32 disagrees with the full-matrix oracle'
pairs += fixture_pairs
assert all(len(a) == len(b) == 32 for a, b in pairs)
data = ''.join(a + ' ' + b + '\n' for a, b in pairs)
(out / 'pairs.txt').write_text(data)
expected = [reference(a, b) for a, b in pairs]
(out / 'expected.txt').write_text(''.join(str(x) + '\n' for x in expected))
with (out / 'scores.txt').open('w') as log:
    subprocess.run([str(binary)], input=data, text=True, stdout=log, check=True)
actual = list(map(int, (out / 'scores.txt').read_text().split()))
failures = [(i, x, y) for i, (x, y) in enumerate(zip(actual, expected)) if x != y]
assert len(actual) == len(expected) and not failures, failures[:20]

# Compile-time launch and capacity guards, including the last accepted count.
config = '#include "sw_parameters.hpp"\nint main() {}\n'
for count, accepted in [(0, False), (5, False), (188, True), (192, False)]:
    compile_test('count-' + str(count), config,
                 ['-UNUM_SEQ', '-DNUM_SEQ=' + str(count)], accepted)

# Exercise the exact input readers without linking a simulator or host runtime.
source = (app / 'main.cpp').read_text()
readers = source[source.index('bool read_seq'):source.index('// Host main;')]
harness = r'''#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <fstream>
#include <string>
#include <vector>
#include "sw_parameters.hpp"
''' + readers + r'''
int main(int argc, char** argv) {
  int count = argc > 3 ? atoi(argv[3]) : 1;
  if (argv[1][0] == 's') {
    std::vector<uint8_t> seq(32*count);
    if (!read_seq(argv[2], seq.data(), count)) return 1;
    for (int i = 0; i < count; i++) {
      fwrite(&seq[32*i], 1, 32, stdout);
      putchar('\n');
    }
  } else {
    std::vector<int> scores(count);
    if (!read_output(argv[2], scores.data(), count)) return 1;
    for (int score : scores) printf("%d\n", score);
  }
}
'''
binary = compile_test('readers', harness)
reader_results = []
for name, mode, content, accepted in [
        ('valid-sequence', 's', '>0\n' + 'A'*32 + '\n', True),
        ('valid-crlf', 's', '>123\r\n' + 'C'*32 + '\r\n', True),
        ('short-sequence', 's', '>0\nA\n', False),
        ('long-sequence', 's', '>0\n' + 'A'*80 + '\n', False),
        ('missing-sequence', 's', '>0\n', False),
        ('long-label-as-sequence', 's', '>' + 'x'*94 + '\n' + 'A'*32 + '\n', False),
        ('label-without-sequence', 's', '>' + 'x'*94 + '\n', False),
        ('long-decimal-label', 's', '>' + '9'*94 + '\n' + 'A'*32 + '\n', True),
        ('missing-label-marker', 's', '0\n' + 'A'*32 + '\n', False),
        ('joined-label-sequence', 's', '>0 ' + 'A'*32 + '\n', False),
        ('valid-score', 'o', '32\n', True),
        ('zero-score', 'o', '0\n', True),
        ('invalid-score', 'o', '33\n', False),
        ('score-wrap-to-zero', 'o', '4294967296\n', False),
        ('score-wrap-to-max', 'o', '4294967328\n', False),
        ('score-with-junk', 'o', '32xyz\n', False),
        ('score-long-overflow', 'o', '9'*80 + '\n', False),
        ('score-negative-wrap', 'o', '-4294967296\n', False),
        ('empty-score', 'o', '', False),
        ('missing-file', 's', None, False)]:
    path = out / name
    if content is not None:
        path.write_text(content)
    command = [str(binary), mode, str(path)]
    with (out / (name + '.log')).open('w') as log:
        result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
    reader_results.append(dict(name=name, command=command, input=content,
                               status=result.returncode, accepted=accepted))
    assert (result.returncode == 0) == accepted, name
    if accepted:
        expected_data = content.splitlines()[1] if mode == 's' else str(int(content))
        assert (out / (name + '.log')).read_text() == expected_data + '\n', name

# Check that parsing preserves every shipped sequence and each expected score.
for name, mode, values in [('dna-query32.fasta', 's', query),
                           ('dna-reference32.fasta', 's', ref),
                           ('output32', 'o', fixture)]:
    command = [str(binary), mode, str(app / name), str(len(values))]
    log_path = out / (name + '-parsed.log')
    with log_path.open('w') as log:
        result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
    reader_results.append(dict(name=name, command=command, status=result.returncode,
                               accepted=True, records=len(values)))
    assert result.returncode == 0, name
    assert log_path.read_text().splitlines() == list(map(str, values)), name
(out / 'reader-results.json').write_text(json.dumps(reader_results, indent=2) + '\n')
print('PASS: 776 pairs match independent DP; 512 fixture scores verified; '
      'size and input checks pass; all shipped records preserved; ASan/UBSan clean')
