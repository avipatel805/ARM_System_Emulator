#!/bin/bash

# clearn reamining connections if there is any
./clear_connection.sh

# Function to stop the Node.js server
cleanup() {
    echo "Shutting down Node.js server..."
    echo $NODE_PID
    kill $NODE_PID  # Terminate the Node.js server
    exit
}

# Catch SIGINT (Ctrl+C) and SIGTERM signals, call cleanup
trap cleanup SIGINT SIGTERM

# Run the C program in the background
"$@" -f 1 &
C_PID=$!

# time sleep
sleep 1

# Start the Node.js server in the background
node proxy_server.js &
NODE_PID=$!

# Wait for the C program to finish
wait $C_PID

# Cleanup and exit (optional here since the script would end anyway)
cleanup
