#include <iostream>
#include <thread>
#include <vector>
#include <libmemcached/memcached.h>
#include <chrono>
#include <fstream>
#include <numeric> // 用于计算平均值
#include <algorithm> // 用于 std::sort

// 插入数据：这里我们使用多个线程并发插入键值对
const uint64_t total_keys = 20 * 1024 * 1024;  // 40M KV pairs
const uint64_t value_size = 1024;  // 1KB
const uint64_t threads_num = 1;
const uint64_t keys_per_thread = total_keys / threads_num;

void insert_data(memcached_st* memc, const std::string& key, const std::string& value) {
    // 插入键值对
    memcached_return rc = memcached_set(memc, key.c_str(), key.size(), value.c_str(), value.size(), (time_t)0, (uint32_t)0);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "Memcached set failed: " << memcached_strerror(memc, rc) << std::endl;
    } 
}

void access_memcached(memcached_st* memc, const std::string& key, const std::string& value) {
    memcached_return rc;
    size_t value_length;
    uint32_t flags;
    char* result = memcached_get(memc, key.c_str(), key.size(), &value_length, &flags, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        std::cout << "Get key failed: " << key << std::endl;
        return;
    } 
    free(result); 
}

void tfunc(uint64_t tid, uint64_t port) {
    memcached_st* memc = memcached_create(nullptr);
    memcached_return rc;
    std::string filename = "/mydata/cfm/memcached/throughput" + std::to_string(tid) + ".txt"; 
    std::string latency_filename = "/mydata/cfm/memcached/latency" + std::to_string(tid) + ".txt";
    std::ofstream of(filename);
    std::ofstream latency_of(latency_filename); // 覆盖写

    rc = memcached_server_add(memc, "127.0.0.1", port);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "Memcached server add failed: " << memcached_strerror(memc, rc) << std::endl;
        return;
    }

    for(uint64_t i = tid * keys_per_thread; i < (tid+1) * keys_per_thread; ++i) {
        std::string key = std::to_string(i);
        std::string value(value_size, 'x');  // 填充1KB的值
        insert_data(memc, key, value);
    }

    while(true) {
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<double> latencies; // 存储每次请求的延迟
        
        for(uint64_t i = tid * keys_per_thread; i < (tid+1) * keys_per_thread; ++i) {
            auto req_start = std::chrono::high_resolution_clock::now();
            
            std::string key = std::to_string(i);
            std::string value;
            access_memcached(memc, key, value);
            
            auto req_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> req_latency = req_end - req_start;
            latencies.push_back(req_latency.count());
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        // 写入吞吐量
        of << (double)keys_per_thread / duration.count() << std::endl;

        // 写入 P5 到 P95 的延迟
        std::sort(latencies.begin(), latencies.end());
        for (int p = 5; p <= 95; p += 5) {
            size_t idx = static_cast<size_t>(p / 100.0 * latencies.size());
            if (idx >= latencies.size()) idx = latencies.size() - 1;
            latency_of << latencies[idx];
            if (p != 95) latency_of << ",";
        }
        latency_of << std::endl;
    }

    memcached_free(memc);
}

int main(int argc, char *argv[]) {
    long port = std::stoi(argv[1]);
    std::vector<std::thread*> threads(threads_num);

    for (uint64_t i = 0; i < threads_num; ++i) {
        threads[i] = new std::thread(tfunc, i, port);
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t->join();
    }

    return 0;
}
