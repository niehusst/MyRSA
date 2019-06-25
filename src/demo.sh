#!/bin/bash

## A simple test of the echo server code (using RSA encryption) in this dir
## Author: Liam Niehus-Staab

# make sure everything is made and up to date
make

# start the server running (run in bg-thread so we can run a client in the same window)
./server &
#//TODO network code in demo.sh is failing because client is being launched b4 server sets up bcus of rsa stuff
sleep 1

# start the client program (running localhost as the other network bc
#   I only have 1 computer)
./client localhost <<< 'Hello World!'
# ^^ tell client what message to send to server

# the echo server should respond by outputting Hello World! back to us
make clean
