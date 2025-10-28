#include <vector>
#include <numeric>
#include <map>
#include <string>

using namespace std; // Using namespace std

class UnionFind {
private:
    map<string, string> parent;
    map<string, int> rank;

public:
    // Initialize Union-Find with all nodes
    UnionFind(const vector<string>& nodes) {
        for (const auto& node : nodes) {
            parent[node] = node; // Each node is its own parent
            rank[node] = 0;
        }
    }

    // Find operation with Path Compression
    string find(const string& i) {
        if (parent[i] == i) {
            return i;
        }
        // Path Compression: make the node point directly to its root
        parent[i] = find(parent[i]);
        return parent[i];
    }

    // Union operation with Union by Rank
    bool unite(const string& i, const string& j) {
        string root_i = find(i);
        string root_j = find(j);

        if (root_i != root_j) {
            // Union by Rank: attach smaller tree to root of larger tree
            if (rank[root_i] < rank[root_j]) {
                parent[root_i] = root_j;
            } else if (rank[root_i] > rank[root_j]) {
                parent[root_j] = root_i;
            } else {
                parent[root_j] = root_i;
                rank[root_i]++; // Only increment if ranks were equal
            }
            return true; // Union happened
        }
        return false; // Already connected
    }
};