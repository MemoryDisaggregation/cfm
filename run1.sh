#!/bin/bash

#program_path="/mydata/cfm/quicksort/quicksort"
program_path="/mydata/cfm/xgboost/higgs.py"
#program_arg=10240

#cmd="taskset -c 9 /usr/bin/time -v $program_path $program_arg"
cmd="taskset -c 9 /usr/bin/time -v $program_path"


echo $((5 * 1024 * 1024 * 1024)) | sudo tee /sys/fs/cgroup/memory/memctl/memory.limit_in_bytes

$cmd &
pid=$!

sleep 0.5

program_pid=$(pgrep -P $pid -n)
if [ -z "$program_pid" ]; then
    echo "Failed to find the actual program PID."
    kill $taskset_pid 2>/dev/null
    exit 1
fi


echo $program_pid | sudo tee /sys/fs/cgroup/memory/memctl/cgroup.procs

echo "Program is running with PID: $program_pid"
cat /sys/fs/cgroup/memory/memctl/memory.limit_in_bytes
cat /sys/fs/cgroup/memory/memctl/cgroup.procs


wait $pid
exit_status=$?