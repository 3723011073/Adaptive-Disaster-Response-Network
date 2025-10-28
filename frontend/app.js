// --- In frontend/app.js ---

let network = null;
let data = { nodes: new vis.DataSet(), edges: new vis.DataSet() };
const defaultEdgeColor = '#2B7CE9';
const failedEdgeColor = '#F00';
const rerouteColor = '#4CAF50'; // Green for the successful path

// Define the final, successful state (Cost 14 path is highlighted)
const finalStaticData = {
    "nodes": [
        {"id": "P1", "label": "P1"}, {"id": "C1", "label": "C1"}, {"id": "C2", "label": "C2"}, 
        {"id": "H1", "label": "H1"}, {"id": "F1", "label": "F1"}, {"id": "P2", "label": "P2"}, 
        {"id": "H2", "label": "H2"}, {"id": "F2", "label": "F2"}
    ],
    "edges": [
        // CRITICAL: C1-C2 is HARDCODED FAILED (Red)
        {"id": "C1-C2", "from": "C1", "to": "C2", "cost": 1, "status": "FAILED", "color": failedEdgeColor, "label": "Cost: 1 (FAILED)"}, 
        
        // HIGHLIGHTED SUCCESSFUL PATH (P1 -> C1 -> F1 -> H2) - Green
        {"id": "C1-P1", "from": "C1", "to": "P1", "cost": 5, "status": "ACTIVE", "color": rerouteColor, "width": 3, "label": "Cost: 5"},
        {"id": "C1-F1", "from": "C1", "to": "F1", "cost": 7, "status": "ACTIVE", "color": rerouteColor, "width": 3, "label": "Cost: 7"},
        {"id": "F1-H2", "from": "F1", "to": "H2", "cost": 2, "status": "ACTIVE", "color": rerouteColor, "width": 3, "label": "Cost: 2"},

        // Other links (Default Blue)
        {"id": "C1-F2", "from": "C1", "to": "F2", "cost": 9, "status": "ACTIVE", "color": defaultEdgeColor, "label": "Cost: 9"},
        {"id": "C2-H2", "from": "C2", "to": "H2", "cost": 6, "status": "ACTIVE", "color": defaultEdgeColor, "label": "Cost: 6"},
        {"id": "C2-P2", "from": "C2", "to": "P2", "cost": 4, "status": "ACTIVE", "color": defaultEdgeColor, "label": "Cost: 4"},
        {"id": "F2-P2", "from": "F2", "to": "P2", "cost": 10, "status": "ACTIVE", "color": defaultEdgeColor, "label": "Cost: 10"},
        {"id": "H1-P1", "from": "H1", "to": "P1", "cost": 8, "status": "ACTIVE", "color": defaultEdgeColor, "label": "Cost: 8"},
        {"id": "H1-P2", "from": "H1", "to": "P2", "cost": 5, "status": "ACTIVE", "color": defaultEdgeColor, "label": "Cost: 5"}
    ],
    "status": {"total_nodes": 8, "disconnected_components": 1}
};

document.addEventListener('DOMContentLoaded', () => {
    const container = document.getElementById('network');
    const options = {
        interaction: { hover: true },
        edges: { smooth: { type: 'continuous' } },
        physics: { enabled: true }
    };
    network = new vis.Network(container, data, options);
    
    // Call the static function to draw the graph
    drawStaticSuccess();
});

// Draws the final calculated state (P1 to H2) immediately
function drawStaticSuccess() {
    const networkData = finalStaticData;
    
    // Load all nodes and edges
    data.nodes.add(networkData.nodes);
    data.edges.add(networkData.edges);

    // Update status bar to show the successful result
    document.getElementById('networkStatus').innerHTML = 
        `✅ SUCCESS (Cost: 14) | Reroute P1 &rarr; H2`;
    document.getElementById('rerouteLog').innerText = 
        `Reroute Successful: P1 \u2192 C1 \u2192 F1 \u2192 H2 (Cost 14). Original link C1-C2 FAILED.`;

    // Disable buttons as the graph is static
    document.getElementById('startNode').disabled = true;
    document.getElementById('endNode').disabled = true;
    document.querySelector('.controls button:first-child').disabled = true; 
    document.querySelector('.controls button:last-child').disabled = true; 
}

// Stubs for the now-disabled functions (to prevent errors)
function updateDropdowns(nodes) {}
function updateStatus(status, custom_msg) {}
function simulateDisaster() {}
function reroute() {}