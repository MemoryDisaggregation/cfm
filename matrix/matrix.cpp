#include <iostream>
#include <vector>
#include <thread>
#include <cstdlib>  // For std::atoi
#include <ctime>    // For measuring execution time
#include <cmath>    // For std::sqrt

// 矩阵乘法函数，用于多线程计算
void multiply_matrices(const std::vector<std::vector<uint64_t>>& A, 
                       const std::vector<std::vector<uint64_t>>& B, 
                       std::vector<std::vector<uint64_t>>& C, 
                       int start_row, int end_row) {
    int n = A.size();  
    int m = B[0].size(); 

    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < m; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < n; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <num_threads> <memory_limit_MB>" << std::endl;
        return 1;
    }

    int num_threads = std::atoi(argv[1]);  // 使用的线程数
    int memory_limit_MB = std::atoi(argv[2]);  // 设置期望的内存限制，单位MB

    // 每个矩阵的内存占用，单位为字节
    const int uint64_size = sizeof(uint64_t);  

    // 计算矩阵的最大尺寸
    // 期望使用的内存为3个矩阵：A, B, C，每个矩阵大小为size * size
    // 因此所需内存：3 * size^2 * sizeof(uint64_t) <= memory_limit_MB * 1024 * 1024
    long long max_memory_bytes = static_cast<long long>(memory_limit_MB) * 1024 * 1024;
    long long required_memory_per_matrix = 3 * (long long)(uint64_size) * (long long)(1000000);  // 每个矩阵需要 `size^2` 个 uint64_t 元素

    // 计算合适的矩阵大小
    long long size = std::sqrt(max_memory_bytes / (3 * uint64_size));
    std::cout << "Using matrix size: " << size << " x " << size << std::endl;

    // 动态分配矩阵
    std::vector<std::vector<uint64_t>> A(size, std::vector<uint64_t>(size));
    std::vector<std::vector<uint64_t>> B(size, std::vector<uint64_t>(size));
    std::vector<std::vector<uint64_t>> C(size, std::vector<uint64_t>(size));

    // 初始化矩阵A和B
    std::srand(std::time(0));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            A[i][j] = std::rand() % 100;  // 随机生成[0, 99]之间的整数
            B[i][j] = std::rand() % 100;  // 随机生成[0, 99]之间的整数
        }
    }

    // 矩阵乘法并行化
    std::vector<std::thread> threads;
    int rows_per_thread = size / num_threads;

    // 记录开始时间
    auto start = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < num_threads; ++t) {
        int start_row = t * rows_per_thread;
        int end_row = (t == num_threads - 1) ? size : (t + 1) * rows_per_thread;
        threads.push_back(std::thread(multiply_matrices, std::ref(A), std::ref(B), std::ref(C), start_row, end_row));
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 记录结束时间
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Matrix multiplication completed in " << duration.count() << " seconds." << std::endl;

    // 可选：输出结果矩阵C的前几行，避免输出过长
    std::cout << "Result (first 5 rows of C):" << std::endl;
    for (int i = 0; i < std::min((long long)5, size); ++i) {
        for (int j = 0; j < std::min((long long)5, size); ++j) {
            std::cout << C[i][j] << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
