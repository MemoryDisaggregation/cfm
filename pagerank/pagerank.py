import networkx as nx
import numpy as np
import random
import time

def generate_large_graph(num_nodes, avg_degree):
    """
    生成一个大规模稀疏图
    :param num_nodes: 图中节点数
    :param avg_degree: 每个节点的平均度
    :return: 生成的图对象
    """
    G = nx.DiGraph()
    
    # 添加节点
    G.add_nodes_from(range(num_nodes))
    
    # 随机添加边（稀疏连接）
    for node in range(num_nodes):
        for _ in range(avg_degree):
            target_node = random.choice(range(num_nodes))
            if target_node != node:  # 避免自环
                G.add_edge(node, target_node)
    
    return G

def calculate_pagerank(G, damping_factor=0.85):
    """
    计算PageRank
    :param G: NetworkX 图对象
    :param damping_factor: 阻尼因子
    :return: PageRank 值字典
    """
    return nx.pagerank(G, alpha=damping_factor)

if __name__ == "__main__":
    num_nodes = 10**6  # 节点数量，100万节点
    avg_degree = 20    # 每个节点的平均度

    start_time = time.time()
    
    # 生成大规模稀疏图
    G = generate_large_graph(num_nodes, avg_degree)
    
    print(f"Graph generated with {num_nodes} nodes and {len(G.edges())} edges.")
    
    # 计算 PageRank
    pagerank_values = calculate_pagerank(G)
    
    print(f"PageRank calculated for {len(pagerank_values)} nodes.")
    print("Time taken:", time.time() - start_time, "seconds.")
