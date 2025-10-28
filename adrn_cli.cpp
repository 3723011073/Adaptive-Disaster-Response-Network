#include "UnionFind.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <ctime>
#include <climits>
#include <queue>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <tuple>

using namespace std;

// --- CRUCIAL: Structs must be defined here for global visibility ---

// Structure to hold edge data within the graph
struct EdgeData {
    int cost;
    string status; // "ACTIVE" or "FAILED"
};

// Structure for returning connection data for JSON generation (public interface)
struct ConnectionData {
    string u;
    string v;
    int cost;
    string status;
};

// Priority Queue item for Dijkstra's
struct PathItem {
    int cost;
    string node;
    vector<string> path;

    bool operator>(const PathItem& other) const {
        return cost > other.cost;
    }
};

class ADRNetwork {
private:
    // Adjacency List: {Node: {Neighbor: EdgeData}}
    map<string, map<string, EdgeData>> graph;
    vector<string> nodes;
    UnionFind* uf;

public:
    ADRNetwork() : uf(nullptr) {}
    ~ADRNetwork() { delete uf; }

    void add_connection(const string& u, const string& v, int cost) {
        if (find(nodes.begin(), nodes.end(), u) == nodes.end()) nodes.push_back(u);
        if (find(nodes.begin(), nodes.end(), v) == nodes.end()) nodes.push_back(v);

        graph[u][v] = {cost, "ACTIVE"};
        graph[v][u] = {cost, "ACTIVE"};
    }

    void initialize_resilience_tracker() {
        if (uf) delete uf;
        uf = new UnionFind(nodes);

        for (const auto& pair1 : graph) {
            const string& u = pair1.first;
            const map<string, EdgeData>& neighbors = pair1.second;
            
            for (const auto& pair2 : neighbors) {
                const string& v = pair2.first;
                const EdgeData& data = pair2.second;
                
                if (u < v && data.status == "ACTIVE") { 
                    uf->unite(u, v);
                }
            }
        }
    }

    // --- Essential Public Getters for CLI/JSON Output ---

    const vector<string>& get_nodes() const {
        return nodes;
    }

    vector<ConnectionData> get_all_connections_data() const {
        vector<ConnectionData> connections;
        for (const auto& pair1 : graph) {
            const string& u = pair1.first;
            const map<string, EdgeData>& neighbors = pair1.second;
            
            for (const auto& pair2 : neighbors) {
                const string& v = pair2.first;
                const EdgeData& data = pair2.second;
                
                if (u < v) { // Avoid duplicates
                    connections.push_back({u, v, data.cost, data.status});
                }
            }
        }
        return connections;
    }

    size_t get_disconnected_components_count() const {
        map<string, int> component_count;
        for (const auto& node : nodes) {
            string root = uf->find(node);
            if (!root.empty()) {
                component_count[root]++;
            }
        }
        return component_count.size();
    }
    
    // --- Core Logic Methods ---

    string find_shortest_path(const string& start_node, const string& end_node) {
        if (graph.find(start_node) == graph.end() || graph.find(end_node) == graph.end()) {
            return "❌ Reroute FAILED: Start or End node not found in network.";
        }
        if (uf->find(start_node) != uf->find(end_node)) {
            return "❌ Reroute FAILED: Nodes are in disconnected components.";
        }

        priority_queue<PathItem, vector<PathItem>, greater<PathItem>> pq;
        map<string, int> distances;

        for (const auto& node : nodes) {
            distances[node] = INT_MAX;
        }

        distances[start_node] = 0;
        pq.push({0, start_node, {start_node}});

        while (!pq.empty()) {
            PathItem current = pq.top();
            pq.pop();

            int current_cost = current.cost;
            string current_node = current.node;
            vector<string> current_path = current.path;

            if (current_cost > distances[current_node]) {
                continue;
            }

            if (current_node == end_node) {
                stringstream ss;
                ss << "✅ Reroute SUCCESS (Cost: " << current_cost << "): ";
                for (size_t i = 0; i < current_path.size(); ++i) {
                    ss << current_path[i] << (i == current_path.size() - 1 ? "" : " -> ");
                }
                return ss.str();
            }
            
            for (const auto& pair : graph[current_node]) {
                const string& neighbor = pair.first;
                const EdgeData& data = pair.second;

                if (data.status == "ACTIVE") {
                    int new_cost = current_cost + data.cost;
                    if (new_cost < distances[neighbor]) {
                        distances[neighbor] = new_cost;
                        vector<string> new_path = current_path;
                        new_path.push_back(neighbor);
                        pq.push({new_cost, neighbor, new_path});
                    }
                }
            }
        }
        return "❌ Reroute FAILED: Path not found (network error).";
    }
    
    string simulate_disaster(int fail_count) {
        vector<pair<string, string>> active_edges;
        
        for (const auto& pair1 : graph) {
            const string& u = pair1.first;
            const map<string, EdgeData>& neighbors = pair1.second;
            
            for (const auto& pair2 : neighbors) {
                const string& v = pair2.first;
                const EdgeData& data = pair2.second;
                
                if (u < v && data.status == "ACTIVE") { 
                    active_edges.push_back({u, v});
                }
            }
        }

        if (active_edges.empty()) return "Network has no active edges left to fail.";

        default_random_engine generator(chrono::system_clock::now().time_since_epoch().count()); 
        shuffle(active_edges.begin(), active_edges.end(), generator);

        size_t actual_fails = min(active_edges.size(), (size_t)fail_count);

        // This output is NOT JSON; it is diagnostic output captured by Node.js stderr
        cerr << "--- Simulating Disaster: Failing " << actual_fails << " connections ---\n";
        for (size_t i = 0; i < actual_fails; ++i) {
            const string& u = active_edges[i].first;
            const string& v = active_edges[i].second;
            
            graph[u][v].status = "FAILED";
            graph[v][u].status = "FAILED";
            cerr << "🚨 Connection FAILED: " << u << " <-> " << v << " (Original Cost: " << graph[u][v].cost << ")\n";
        }

        this->initialize_resilience_tracker();
        
        return "✅ Disaster simulation complete. Network state updated.";
    }

    void print_all_connections() const {
        // Diagnostic print (optional)
        // ... (body using getters)
    }
};