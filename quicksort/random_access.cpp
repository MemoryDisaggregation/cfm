#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <atomic>
#include <random>
#include <thread>

const size_t MB = 1024 * 1024;
using namespace std::chrono;

void die(const char *msg, bool printErrno) {
	std::cerr << msg << "\n";
	exit(1);
}

void print_time_diff(time_point<high_resolution_clock> start,
	time_point<high_resolution_clock> end) {
	auto diff = end - start;
	std::cout << "time " << duration<double, std::nano> (diff).count() << "\n";
}

void print_time_diff_ms(time_point<high_resolution_clock> start,
	time_point<high_resolution_clock> end) {
	auto diff = end - start;
	std::cout << "time " << duration<double, std::milli> (diff).count() << " ms\n";
}

void random_add(long numInts) {
	auto atomic_vector = new std::vector<std::atomic<int>>(numInts);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();  
    std::mt19937 gen(seed);  
    std::uniform_int_distribution<> dis(0, atomic_vector->size() - 1); 
	int i = 0;
	uint64_t count = 0;
	uint64_t mid = atomic_vector->size() / 2;
	
    
    while(true) {
		if((count / 100) % 2 == 0) {
			for(uint32_t idx = 0; idx < mid; ++idx) {
				atomic_vector->at(idx).fetch_add(1);
			}
		} else {
			for(uint32_t idx = mid; idx < atomic_vector->size(); ++idx) {
				atomic_vector->at(idx).fetch_add(1);
			}
		}
		count++;
    }

}

int main(int argc, char *argv[]) {
	if (argc != 2)
		die("need MB of integers to sort", false);

	long size = std::stoi(argv[1]) * MB;
	long numInts = size / sizeof(std::atomic<int>);

	std::cout << "will random access " << numInts << " integers (" << size / MB << " MB)\n";
	//std::vector<std::atomic<int>> v(numInts);

	std::srand(std::time(0));
	time_point<high_resolution_clock> start, end;

	//std::generate(v.begin(), v.end(), std::rand);
	//start = high_resolution_clock::now();

	std::thread* threads[8];
	for(int i = 0; i < 8; ++i) {
		threads[i] = new std::thread(random_add, (numInts/8));
	}

	for(int i = 0; i < 8; ++i) {
		threads[i]->join();
	}

	//end = high_resolution_clock::now();
	//print_time_diff_ms(start, end);

	return 0;
}
