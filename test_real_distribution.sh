#!/bin/bash

echo "=== Real Distributed Parsing Test ==="
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

echo "Step 1: Starting 3 distributed parser nodes..."
echo ""

echo "Starting Node 1 on port 8080..."
./dist/distributed.raft 1 8080 > node1.log 2>&1 &
NODE1_PID=$!

echo "Starting Node 2 on port 8081..."
./dist/distributed.raft 2 8081 > node2.log 2>&1 &
NODE2_PID=$!

echo "Starting Node 3 on port 8082..."
./dist/distributed.raft 3 8082 > node3.log 2>&1 &
NODE3_PID=$!

echo ""
echo "All nodes started! PIDs: $NODE1_PID, $NODE2_PID, $NODE3_PID"
echo "Waiting for nodes to initialize..."

# Wait for nodes to start up and discover each other
sleep 5

echo ""
echo "Step 2: Checking node status..."
echo "Node 1 log (last 10 lines):"
tail -10 node1.log

echo ""
echo "Node 2 log (last 10 lines):"
tail -10 node2.log

echo ""
echo "Node 3 log (last 10 lines):"
tail -10 node3.log

echo ""
echo "Step 3: Testing distributed parsing with 5 files..."
echo "Running parsing test on Node 1..."

# Run the distributed parsing test on a different port to avoid conflicts
./dist/distributed.raft 4 8083 benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp benchmark/fixtures/large.lisp benchmark/fixtures/small.lisp benchmark/fixtures/medium.lisp

echo ""
echo "Step 4: Checking for network communication between nodes..."
echo "Looking for heartbeat messages and node discovery..."

echo "Node 1 heartbeats:"
grep -c "Heartbeat" node1.log 2>/dev/null || echo "No heartbeat messages found"

echo "Node 2 heartbeats:"
grep -c "Heartbeat" node2.log 2>/dev/null || echo "No heartbeat messages found"

echo "Node 3 heartbeats:"
grep -c "Heartbeat" node3.log 2>/dev/null || echo "No heartbeat messages found"

echo ""
echo "Step 5: Checking Raft consensus..."
echo "Looking for leader election messages..."

echo "Node 1 Raft messages:"
grep -E "(LEADER|FOLLOWER)" node1.log 2>/dev/null || echo "No Raft messages found"

echo "Node 2 Raft messages:"
grep -E "(LEADER|FOLLOWER)" node2.log 2>/dev/null || echo "No Raft messages found"

echo "Node 3 Raft messages:"
grep -E "(LEADER|FOLLOWER)" node3.log 2>/dev/null || echo "No Raft messages found"

echo ""
echo "Test completed! Nodes are still running in the background."
echo "Log files: node1.log, node2.log, node3.log"
echo "Press Ctrl+C to stop all nodes"

# Keep the script running
while true; do
    sleep 1
done
