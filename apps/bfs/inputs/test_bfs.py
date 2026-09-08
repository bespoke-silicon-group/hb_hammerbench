"""Check preprocessing against an independent queue BFS on directed graphs.

Run with python3 -m unittest discover -s apps/bfs/inputs -p test_bfs.py.
BFS_PREPROCESSOR may name an older script to reproduce its failures.
"""
from collections import deque
import os
from pathlib import Path
import random
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(os.environ.get('BFS_PREPROCESSOR', Path(__file__).with_name('bfs.py'))).resolve()


def oracle(graph, root):
    distance = [-1] * len(graph)
    distance[root] = 0
    queue = deque([root])
    while queue:
        src = queue.popleft()
        for dst in graph[src]:
            if distance[dst] < 0:
                distance[dst] = distance[src] + 1
                queue.append(dst)
    return distance


def write_csr(directory, name, graph):
    offsets = [0]
    neighbors = []
    for row in graph:
        neighbors.extend(row)
        offsets.append(len(neighbors))
    for suffix, data in [('offsets', offsets), ('nonzeros', neighbors)]:
        (directory / ('graph.' + name + '_' + suffix + '.txt')).write_text(
            ''.join(str(x) + '\n' for x in data))


class PreprocessTest(unittest.TestCase):
    def check_graph(self, graph, root=0, starts_in_pull=False):
        expected = oracle(graph, root)
        reverse = [[] for _ in graph]
        for src, row in enumerate(graph):
            for dst in row:
                reverse[dst].append(src)
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            write_csr(directory, 'fwd', graph)
            write_csr(directory, 'rev', reverse)
            for push_pull in (0, 1):
                with self.subTest(push_pull=push_pull):
                    subprocess.run([sys.executable, str(SCRIPT), 'graph', str(root),
                                    str(push_pull)], cwd=directory, check=True,
                                   capture_output=True, text=True, timeout=10)
                    distance = [int(x) for x in (directory / 'graph.distance.txt').read_text().split()]
                    direction = [int(x) for x in (directory / 'graph.direction.txt').read_text().split()]
                    self.assertEqual(distance, expected)
                    self.assertEqual(len(direction), max(expected) + 1)
                    self.assertTrue(all(d in (0, 1) for d in direction))
                    if not push_pull:
                        self.assertTrue(all(d == 0 for d in direction))
                    elif starts_in_pull:
                        self.assertEqual(direction[0], 1)

    def test_pull_chain_cannot_cascade_within_one_level(self):
        self.check_graph([[1], [2], [3], [4], [5], [6], [7], []], starts_in_pull=True)

    def test_diamond_duplicate_edges_self_loops_and_disconnected_vertex(self):
        self.check_graph([[0, 1, 1, 2], [3], [3], [4, 4], [], []], starts_in_pull=True)

    def test_reverse_numbering(self):
        self.check_graph([[], [0], [1], [2], [3], [4], [5], [6]], root=7, starts_in_pull=True)

    def test_seeded_directed_graphs(self):
        for size in (1, 2, 8, 32, 41, 64):
            for seed in range(4):
                rng = random.Random(seed * 101 + size)
                graph = [[dst for dst in range(size) if rng.random() < 0.12]
                         for _ in range(size)]
                with self.subTest(size=size, seed=seed):
                    self.check_graph(graph, root=seed % size)


if __name__ == '__main__':
    unittest.main()
