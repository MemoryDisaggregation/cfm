#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <map>
#include <mutex>
#include <atomic>
#include <memory>
#include <cassert>

#define NUM_THREADS  16

std::map<std::array<char, 4>, int> word_count;

const char vowels[] = {'A', 'E', 'I', 'O', 'U', 'a', 'e', 'i', 'o', 'u'};
const uint64_t word_size = 4;
std::map<std::array<char, 4>, int> t_count[NUM_THREADS];



void map_task(int thread_id, size_t data_size) {
    std::vector<std::array<char, 4>> local_words(data_size);
    std::map<std::array<char, 4>, int>& local_count = t_count[thread_id];

    std::random_device rd;  
    std::mt19937 gen(rd()); 

    std::uniform_int_distribution<> dis(0, 9);

    for (size_t i = 0; i < data_size; ++i) {
        for(int j = 0;j < 4; ++j) {
            int random_index = dis(gen); 
            local_words[i][j] = vowels[random_index];
        }
        local_count[local_words[i]]++;
    }

    std::cout << "Thread " << thread_id << " processed " << data_size << " words.\n";
    for (const auto& pair : local_count) {
            auto p = word_count.find(pair.first);
            if(p == word_count.end()) {
                word_count[pair.first] = pair.second; 
            } else {
                word_count[pair.first] += pair.second;
            }
        }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <total size in MB>\n";
        return 1;
    }

    uint64_t total_size_mb = std::atoi(argv[1]);
    
    uint64_t num_words = (total_size_mb * 1024 * 1024) / word_size;
    uint64_t num_per_thread_words = num_words / NUM_THREADS; 

    std::cout << "Total words: " << num_per_thread_words << " Words\n";
    std::cout << "Each thread will process: " << num_per_thread_words  << " Words\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(std::thread(map_task, i, num_per_thread_words));
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    //for(int i = 0;i < NUM_THREADS; ++i) {
    //    auto& local_count = t_count[i];
        
    //}
    

    // 输出最终统计结果
    std::cout << "\nFinal word count:\n";
    int count = 0;
    for (const auto& pair : word_count) {
        for(int i = 0;i < 4; ++i)
            std::cout << char(pair.first[i]);
        std::cout << ": " << pair.second << std::endl;
        if(count++ > 10) {
            break;
        }
    }

    return 0;
}

