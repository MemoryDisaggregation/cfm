from bcc import BPF
from time import sleep
import struct

func = "shrink_page_list"
file_path_prex = "res_shrink_page_list_fastswap"
file_path_tail = ".txt"

# 定义BPF程序
bpf_program = """
#include <uapi/linux/ptrace.h>
#include <uapi/linux/bpf_perf_event.h>

// 定义一个64位原子整数来存储页数
BPF_ARRAY(directswap_pages_count, u64, 1);

// 记录函数开始时间的跟踪点
int trace_func_entry(struct pt_regs *ctx) {
    return 0;
}

// 记录函数结束时间并更新原子计数器的跟踪点
int trace_func_return(struct pt_regs *ctx) {
    int zero = 0;
    u64 *count = directswap_pages_count.lookup(&zero);
    u64 pages = PT_REGS_RC(ctx);
    if (count != NULL && pages > 0) {
        *(count) += pages;
    }
    return 0;
}
"""

# 加载BPF程序
b = BPF(text=bpf_program)

# 附加跟踪点到目标函数
b.attach_kprobe(event=func, fn_name="trace_func_entry")
b.attach_kretprobe(event=func, fn_name="trace_func_return")

# 定义每秒打印的逻辑
def print_pages_count():
    prev_count = 0
    while True:
        # 获取当前原子计数器的值
        count_bytes  = b["directswap_pages_count"][0]
        count, = struct.unpack("Q", count_bytes)
        diff = count - prev_count
        prev_count = count
        print(f"Total pages swapped: {diff}")
        sleep(1)

# 启动打印逻辑的线程
import threading
print_thread = threading.Thread(target=print_pages_count)
print_thread.daemon = True
print_thread.start()

# 主循环，监听 eBPF 程序的输出
try:
    while True:
        sleep(1)
except KeyboardInterrupt:
    pass
