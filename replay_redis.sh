#!/bin/bash

mv etc_replay/redis.conf etc/redis.conf
# Find processes named 'redis-server'
PIDS=$(ps -aux | grep -w 'gramine' | grep -v grep | awk '{print $2}')

if [ -z "$PIDS" ]; then
  echo "No processes named 'redis-server' found."
else
  # Loop through each PID and terminate it
  for PID in $PIDS; do
    echo "Terminating the process with PID: $PID."
    kill -9 $PID
  done
fi

sudo gramine-sgx redis-server