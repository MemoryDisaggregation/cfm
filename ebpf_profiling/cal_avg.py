import numpy as np

values = []
for i in range(25074, 25078):
    with open("/mydata/cfm/ebpf_profiling/res_swap_free{}.txt".format(i), 'r') as file:
        values = [float(line.strip()) for line in file]
        print(np.mean(values[1000:]))

