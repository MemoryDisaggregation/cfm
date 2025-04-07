#include <iostream>
#include <vector>
#include <cstring>
#include <chrono>
#include <random>
#include <thread>

#define NUM_THREADS 32
#define PAGE_SIZE_4KB 4096

const size_t MB = 1024 * 1024;
using namespace std::chrono;

void die(const char *msg) {
    std::cerr << msg << "\n";
    exit(1);
}

void random_page_access(char* memory, size_t total_size) {
    // Calculate number of 4KB pages
    size_t num_pages = total_size / PAGE_SIZE_4KB;
    
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();  
    std::mt19937 gen(seed);  
    std::uniform_int_distribution<size_t> dis_page(0, num_pages - 1);
    
    // Pattern to write (can be any 4KB pattern)
    char pattern[PAGE_SIZE_4KB];
    memset(pattern, 0xAA, PAGE_SIZE_4KB); // Example pattern
    
    while(true) {
        // Select a random 4KB page
        size_t page_idx = dis_page(gen);
        char* page_ptr = memory + page_idx * PAGE_SIZE_4KB;
        
        // Modify the entire 4KB page at once
        memcpy(page_ptr, pattern, PAGE_SIZE_4KB);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        die("need MB of memory to allocate");

    size_t size = std::stoul(argv[1]) * MB;
    
    // Allocate memory aligned to 4KB boundary
    char* memory;
    if (posix_memalign((void**)&memory, PAGE_SIZE_4KB, size) != 0) {
        die("Failed to allocate aligned memory");
    }
    
    std::cout << "Will randomly access " << size/MB << " MB memory in 4KB pages\n";
    std::cout << "Total pages: " << size/PAGE_SIZE_4KB << "\n";

    // Initialize threads
    std::vector<std::thread> threads;
    size_t per_thread_size = size / NUM_THREADS;
    
    for(int i = 0; i < NUM_THREADS; ++i) {
        char* thread_mem = memory + i * per_thread_size;
        threads.emplace_back(random_page_access, thread_mem, per_thread_size);
    }

    for(auto& t : threads) {
        t.join();
    }

    free(memory);
    return 0;
}