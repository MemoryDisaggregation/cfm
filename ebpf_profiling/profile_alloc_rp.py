from bcc import BPF
from time import sleep

func = "free_remote_page"
file_path_prex = "res_alloc_remote_page"
file_path_tail = ".txt"

# 定义BPF程序
bpf_program = """
#include <uapi/linux/ptrace.h>
#include <uapi/linux/bpf_perf_event.h>

// 定义用于存储时间戳的哈希表
BPF_HASH(start, u32, u64);

// 定义一个perf event来输出延迟数据
struct data_t {
    u32 pid;
    u64 delta;
};
BPF_PERF_OUTPUT(events);

// 记录函数开始时间的跟踪点
int trace_func_entry(struct pt_regs *ctx) {
    u32 pid = bpf_get_current_pid_tgid();
    u64 ts = bpf_ktime_get_ns();
    start.update(&pid, &ts);
    return 0;
}

// 记录函数结束时间并计算执行时间的跟踪点
int trace_func_return(struct pt_regs *ctx) {
    u32 pid = bpf_get_current_pid_tgid();
    u64 *tsp = start.lookup(&pid);
    if (tsp != 0) {
        u64 ts = bpf_ktime_get_ns();
        u64 delta = ts - *tsp;
        
        struct data_t data = {};
        data.pid = pid;
        data.delta = delta;
        
        events.perf_submit(ctx, &data, sizeof(data));
        
        start.delete(&pid);
    }
    return 0;
}
"""

# 加载BPF程序
b = BPF(text=bpf_program)

# 附加跟踪点到目标函数
b.attach_kprobe(event=func, fn_name="trace_func_entry")
b.attach_kretprobe(event=func, fn_name="trace_func_return")



# 定义处理函数
def print_event(cpu, data, size):
    event = b["events"].event(data)
    file_path = file_path_prex + str(event.pid) + file_path_tail
    f = open(file_path, 'a')
    print(f"PID {event.pid}: {event.delta} ns")
    f.write(f"{event.delta}\n")

# 绑定事件
b["events"].open_perf_buffer(print_event)

# 主循环
try:
    while True:
        b.perf_buffer_poll()
except KeyboardInterrupt:
    pass