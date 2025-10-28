#include "ADRNetwork.h" 
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <stdlib.h>     
#include <windows.h>    

using namespace std;

// --- DUMMY WINMAIN FIX (Resolves MinGW Linker Errors) ---
// Explicitly declare the C runtime argument counter and vector
extern int __argc;
extern char** __argv;

extern "C" int main(int, char**); 
extern "C" int __stdcall WinMainCRTStartup() {
    // This function satisfies the linker's demand for a custom entry point.
    return main(__argc, __argv); 
}
// ----------------------------------------------------------------


// --- Forward Declaration ---
ADRNetwork setup_initial_network();

// --- GLOBAL NETWORK OBJECT DECLARATION ---
ADRNetwork network; 

// --- 1. Network Setup Function Definition ---
ADRNetwork setup_initial_network() {
    ADRNetwork net;
    vector<tuple<string, string, int>> connections = {
        {"P1", "C1", 5}, {"H1", "C1", 3}, {"F1", "C1", 7},
        {"C1", "C2", 1}, {"C2", "P2", 4}, {"C2", "H2", 6},
        {"P1", "H1", 8}, {"H2", "F1", 2}, {"C1", "F2", 9},
        {"F2", "P2", 10}, {"H1", "P2", 5}
    };

    for (const auto& conn : connections) {
        net.add_connection(get<0>(conn), get<1>(conn), get<2>(conn));
    }
    net.initialize_resilience_tracker();
    
    // 💥 HARDCODED DISASTER FIX: The critical C1-C2 link is permanently failed.
    // This guarantees the Reroute function must be active upon execution.
    net.force_fail_connection("C1", "C2");
    
    // Re-initialize connectivity tracker to account for the forced failure
    net.initialize_resilience_tracker(); 

    return net;
}

// --- Find and REPLACE the entire current int main(...) function in main.cpp ---

int main(int argc, char* argv[]) {
    // CRITICAL FIX: Disable stream sync for stability
    ios_base::sync_with_stdio(false); 
    
    // Initialize the network globally once (with the hardcoded C1-C2 failure)
    network = setup_initial_network();
    
    // --- RESTORE ARGUMENT PARSING FOR NODE.JS ---
    if (argc > 1) {
        string command = argv[1];

        // This path handles the disaster command and the reroute command
        if (command == "disaster" && argc > 2) {
            try {
                int fail_count = stoi(argv[2]);
                network.simulate_disaster(fail_count); 
            } catch (const exception& e) {
                cerr << "Error: Invalid number provided for fail count." << endl;
                return 1;
            }
        } else if (command == "reroute" && argc > 3) {
            string start_node = argv[2];
            string end_node = argv[3];
            cout << network.find_shortest_path(start_node, end_node) << endl;
            cout.flush(); 
            return 0;
        }
    }
    
    // Default action (When Node.js asks for initial status):
    cout << "STATUS_NODES:" << network.get_nodes().size() << endl;
    cout << "STATUS_COMPONENTS:" << network.get_disconnected_components_count() << endl;
    for (const auto& conn : network.get_all_connections_data()) {
        cout << "EDGE:" << conn.u << "," << conn.v << "," << conn.cost << "," << conn.status << endl;
    }
    cout.flush(); 
    return 0;
}