#!/bin/bash

echo "=== Starting Multiple Distributed Parser Nodes ==="
echo ""

# Function to cleanup background processes
cleanup() {
    echo ""
    echo "Cleaning up background processes..."
    pkill -f "distributed.raft" 2>/dev/null
    exit 0
}

# Set up signal handlers
trap cleanup SIGINT SIGTERM

echo "Starting Node 1 on port 8080..."
./dist/distributed.raft 1 8080 &
NODE1_PID=$!

echo "Starting Node 2 on port 8081..."
./dist/distributed.raft 2 8081 &
NODE2_PID=$!

echo "Starting Node 3 on port 8082..."
./dist/distributed.raft 3 8082 &
NODE3_PID=$!

echo ""
echo "All nodes started! PIDs: $NODE1_PID, $NODE2_PID, $NODE3_PID"
echo "Press Ctrl+C to stop all nodes"
echo ""

# Wait for nodes to start up
sleep 3

echo "Testing distributed parsing with 5 files..."
echo "Running: ./dist/distributed.raft 1 8080 benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp benchmark/fixtures/large.lisp benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp"
echo ""

# Run the distributed parsing test
./dist/distributed.raft 1 8080 benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp benchmark/fixtures/large.lisp benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp

echo ""
echo "Test completed. Nodes are still running in the background."
echo "Press Ctrl+C to stop all nodes"

# Keep the script running
while true; do
    sleep 1
done
