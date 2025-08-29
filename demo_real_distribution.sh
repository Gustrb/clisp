#!/bin/bash

echo "=== Real Distributed Parsing Demonstration ==="
echo ""

# Clean up any existing processes
pkill -f "distributed.raft" 2>/dev/null

echo "Step 1: Starting 3 distributed parser nodes in background..."
echo ""

# Start 3 nodes in background
./dist/distributed.raft 1 8080 > /dev/null 2>&1 &
NODE1_PID=$!

./dist/distributed.raft 2 8081 > /dev/null 2>&1 &
NODE2_PID=$!

./dist/distributed.raft 3 8082 > /dev/null 2>&1 &
NODE3_PID=$!

echo "Nodes started with PIDs: $NODE1_PID, $NODE2_PID, $NODE3_PID"
echo "Waiting for nodes to initialize..."

# Wait for nodes to start
sleep 3

echo ""
echo "Step 2: Testing distributed parsing with 5 files..."
echo "This will show how files are distributed across the 3 nodes:"
echo ""

# Test distributed parsing
./dist/distributed.raft 4 8083 benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp benchmark/fixtures/large.lisp benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp

echo ""
echo "Step 3: Checking if nodes are still running..."
ps aux | grep distributed.raft | grep -v grep

echo ""
echo "Step 4: Stopping all nodes..."
pkill -f "distributed.raft"

echo "Demo completed!"
