import numpy as np
import os

# 指定目录路径
directory = "/mydata/cfm/memcached/"

# 获取目录下所有txt文件
txt_files = [f for f in os.listdir(directory) if f.endswith('.txt')]

# 计算并打印每个txt文件的平均值
for file_name in txt_files:
    file_path = os.path.join(directory, file_name)
    try:
        with open(file_path, 'r') as file:
            values = [float(line.strip()) for line in file if line.strip()]
            if values:  # 确保文件不为空
                print(f"{file_name}: {np.mean(values)}")
            else:
                print(f"{file_name}: 文件为空")
    except Exception as e:
        print(f"处理文件 {file_name} 时出错: {str(e)}")