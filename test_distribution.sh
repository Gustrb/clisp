#!/bin/bash

echo "=== Distributed Lisp Parser - File Distribution Demo ==="
echo ""

echo "Testing with 5 files across different node configurations:"
echo ""

echo "1. Single node (Node 1):"
./dist/distributed.raft 1 8080 benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp benchmark/fixtures/large.lisp benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp 2>/dev/null | grep -E "(DIST:|INFO: Parsing|INFO: Successfully parsed|INFO: Parsing completed)"

echo ""
echo "2. Two nodes (Node 2):"
./dist/distributed.raft 2 8081 benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp benchmark/fixtures/large.lisp benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp 2>/dev/null | grep -E "(DIST:|INFO: Parsing|INFO: Successfully parsed|INFO: Parsing completed)"

echo ""
echo "3. Three nodes (Node 3):"
./dist/distributed.raft 3 8082 benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp benchmark/fixtures/large.lisp benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp 2>/dev/null | grep -E "(DIST:|INFO: Parsing|INFO: Successfully parsed|INFO: Parsing completed)"

echo ""
echo "=== Distribution Summary ==="
echo "With 5 files:"
echo "- 1 node: All 5 files -> Node 1"
echo "- 2 nodes: 3 files -> Node 1, 2 files -> Node 2"
echo "- 3 nodes: 2 files -> Node 1, 2 files -> Node 2, 1 file -> Node 3"
echo ""
echo "The system automatically distributes files evenly across available nodes!"
