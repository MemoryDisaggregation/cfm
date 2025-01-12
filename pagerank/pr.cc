#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/page_rank.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace boost;
using namespace std;

typedef adjacency_list<vecS, vecS, directedS> Graph;

Graph loadGraph(const string& filename) {
    Graph g;
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        istringstream ss(line);
        int from, to;
        ss >> from >> to;
        add_edge(from, to, g);
    }
    return g;
}

int main() {
    string filename = "/mydata/dataset/pagelinks_v4.csv";
    Graph g = loadGraph(filename);
    
    vector<double> ranks(num_vertices(g), 1.0 / num_vertices(g));
    page_rank(g, make_iterator_property_map(ranks.begin(), get(vertex_index, g)));

    cout << "PageRank results:" << endl;
    for (size_t i = 0; i < ranks.size(); ++i) {
        cout << "Page " << i << ": " << ranks[i] << endl;
        if (i >= 10) break;  // Print first 10 results
    }

    return 0;
}
