import numpy as np

values = []
with open("/mydata/cfm/ebpf_profiling/res_alloc_remote_page96019.txt", 'r') as file:
    values = [float(line.strip()) for line in file]

print(np.mean(values))