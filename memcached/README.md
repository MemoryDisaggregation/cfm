sudo apt-get install openjdk-8-jdk

sudo sh -c "echo 69896 > /cgroup2/benchmarks/memcached1/cgroup.procs"
sudo sh -c "echo 10855m > /cgroup2/benchmarks/memcached1/memory.high" 
sudo sh -c "echo 10855m > /cgroup2/benchmarks/memcached0/memory.max"

taskset -c 0 /mydata/cfm/memcached/ycsb-0.17.0/bin/ycsb.sh load memcached -s -P /mydata/cfm/memcached/ycsb-0.17.0/workloads/workloadb -p "memcached.hosts=localhost:7777" -p "operationcount=30000000" -p "recordcount=10000000" -p "fieldlength=1024" -p "fieldcount=2"

taskset -c 4 /mydata/cfm/memcached/ycsb-0.17.0/bin/ycsb.sh run memcached -s -P /mydata/cfm/memcached/ycsb-0.17.0/workloads/workloadb -p "memcached.hosts=localhost:7777" -p "operationcount=30000000" -p "requestdistribution=uniform"

taskset -c 5,6,7,8 /mydata/cfm/memcached/ycsb-0.17.0/bin/ycsb.sh run memcached -s -P /mydata/cfm/memcached/ycsb-0.17.0/workloads/workloadb -p "memcached.hosts=localhost:7777" -p "operationcount=30000000" -p "requestdistribution=uniform" -p "threads=4"
