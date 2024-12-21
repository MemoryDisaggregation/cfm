#!/bin/bash

program_path="/mydata/cfm/quicksort/random_access"
program_arg=32768

cmd="taskset -c 0,1,2,3,4,5,6,7 /usr/bin/time -v $program_path $program_arg"


echo $((2 * 1024 * 1024 * 1024)) | sudo tee /sys/fs/cgroup/memory/memctl2/memory.limit_in_bytes

$cmd &
pid=$!

sleep 0.5

program_pid=$(pgrep -P $pid -n)
if [ -z "$program_pid" ]; then
    echo "Failed to find the actual program PID."
    kill $taskset_pid 2>/dev/null
    exit 1
fi


echo $program_pid | sudo tee /sys/fs/cgroup/memory/memctl2/cgroup.procs

echo "Program is running with PID: $program_pid"
cat /sys/fs/cgroup/memory/memctl2/memory.limit_in_bytes
cat /sys/fs/cgroup/memory/memctl2/cgroup.procs


wait $pid
exit_status=$?