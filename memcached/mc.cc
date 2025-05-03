#include <iostream>
#include <thread>
#include <vector>
#include <libmemcached/memcached.h>
#include <chrono>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <random>

const uint64_t total_keys = 16 * 1024 * 1024;  // 16M keys
const uint64_t value_size = 930;               // ~1KB
const uint64_t threads_num = 2;
const uint64_t keys_per_thread = total_keys / threads_num;
const uint64_t batch_size = 1;

void insert_data(memcached_st* memc, const std::string& key, const std::string& value) {
    memcached_return rc = memcached_set(memc, key.c_str(), key.size(), value.c_str(), value.size(), (time_t)0, (uint32_t)0);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "Memcached set failed: " << memcached_strerror(memc, rc) << std::endl;
    }
}

void access_memcached_batch(memcached_st* memc, const std::vector<std::string>& keys) {
    std::vector<const char*> key_ptrs;
    std::vector<size_t> key_lens;

    for (const auto& key : keys) {
        key_ptrs.push_back(key.c_str());
        key_lens.push_back(key.size());
    }

    memcached_return rc = memcached_mget(memc, key_ptrs.data(), key_lens.data(), keys.size());
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "memcached_mget failed: " << memcached_strerror(memc, rc) << std::endl;
        return;
    }

    memcached_result_st* result;
    while ((result = memcached_fetch_result(memc, nullptr, &rc)) != nullptr) {
        memcached_result_free(result);
    }
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

    // 插入初始数据
    for (uint64_t i = tid * keys_per_thread; i < (tid + 1) * keys_per_thread; ++i) {
        std::string key = std::to_string(i);
        std::string value(value_size, 'x');
        insert_data(memc, key, value);
    }

    // 访问循环
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<double> latencies;
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> dis(tid * keys_per_thread, (tid + 1) * keys_per_thread - 1);

        for (uint64_t i = tid * keys_per_thread; i < (tid + 1) * keys_per_thread; i += batch_size) {
            std::vector<std::string> batch_keys;
            for (size_t j = 0; j < batch_size; ++j) {
                batch_keys.emplace_back(std::to_string(dis(gen)));
            }

            auto req_start = std::chrono::high_resolution_clock::now();
            access_memcached_batch(memc, batch_keys);
            auto req_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> latency = req_end - req_start;
            latencies.push_back(latency.count());
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        // 吞吐量：总请求次数 = keys_per_thread / batch_size
        double throughput = (keys_per_thread / (double)batch_size) / duration.count();
        of << throughput << std::endl;

        // 延迟 P5-P95
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./memcached_client <port>" << std::endl;
        return 1;
    }

    long port = std::stoi(argv[1]);
    std::vector<std::thread*> threads(threads_num);

    for (uint64_t i = 0; i < threads_num; ++i) {
        threads[i] = new std::thread(tfunc, i, port);
    }

    for (auto& t : threads) {
        t->join();
    }

    return 0;
}
