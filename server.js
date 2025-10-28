const express = require('express');
const { exec } = require('child_process');
const path = require('path');
const app = express();
const PORT = 8081;

const CXX_EXEC_PATH = path.resolve(__dirname, 'adrn.exe');

// --- Function to execute C++ CLI tool and parse text output ---
function runCXXCommand(commandArgs) {
    return new Promise((resolve, reject) => {
        const fullCommand = `${CXX_EXEC_PATH} ${commandArgs.join(' ')}`;
        
        console.log(`Executing C++: ${fullCommand}`);
        
        // Execute the command and capture stdout/stderr
        exec(fullCommand, { cwd: __dirname }, (error, stdout, stderr) => {
            if (stderr) {
                 console.log(`C++ Stderr: ${stderr.trim()}`);
                 // Note: We don't reject immediately on stderr because C++ uses it for diagnostic prints (like disaster warnings).
            }
            if (error && error.code !== 0) {
                return reject({ status: 500, message: `C++ runtime error (Code ${error.code}): See terminal for full log.` });
            }
            
            // Output parsing logic
            const lines = stdout.trim().split('\n').filter(line => line.length > 0);
            let data = { nodes: [], edges: [], status: {} };
            let nodeSet = new Set();
            
            lines.forEach(line => {
                if (line.startsWith('STATUS_NODES:')) {
                    data.status.total_nodes = parseInt(line.split(':')[1]);
                } else if (line.startsWith('STATUS_COMPONENTS:')) {
                    data.status.disconnected_components = parseInt(line.split(':')[1]);
                } else if (line.startsWith('EDGE:')) {
                    const parts = line.split(':').pop().split(',');
                    const [u, v, cost, status] = parts;
                    
                    if (!nodeSet.has(u)) { data.nodes.push({ id: u, label: u }); nodeSet.add(u); }
                    if (!nodeSet.has(v)) { data.nodes.push({ id: v, label: v }); nodeSet.add(v); }

                    data.edges.push({
                        id: `${u}-${v}`, from: u, to: v, cost: parseInt(cost), status: status
                    });
                }
            });

            // If it's a reroute, the output is the raw path string (e.g., "SUCCESS|18|P1 -> ...")
            if (commandArgs[0] === 'reroute') {
                resolve({ raw_message: stdout.trim(), ...data });
            } else {
                resolve(data);
            }
        });
    });
}

// --- API ENDPOINT 1: GET /network/status ---
app.get('/network/status', async (req, res) => {
    try {
        const data = await runCXXCommand([]);
        res.json(data);
    } catch (err) {
        res.status(err.status || 500).send(err.message);
    }
});

// --- API ENDPOINT 2: POST /network/disaster/<int> ---
app.post('/network/disaster/:fail_count', async (req, res) => {
    try {
        const failCount = req.params.fail_count;
        await runCXXCommand(['disaster', failCount]);
        // Get the updated state
        const data = await runCXXCommand([]); 
        res.json(data);
    } catch (err) {
        res.status(err.status || 500).send(err.message);
    }
});

// --- API ENDPOINT 3: GET /reroute?start=X&end=Y ---
app.get('/reroute', async (req, res) => {
    try {
        const { start, end } = req.query;
        if (!start || !end) return res.status(400).send("Start and end nodes are required.");
        
        const data = await runCXXCommand(['reroute', start, end]);
        
        const pathMessage = data.raw_message; 
        const parts = pathMessage.split('|');
        
        let pathNodes = [];
        let cost = 'N/A';
        let status = 'Error';
        let resultMsg = 'Reroute FAILED: Unknown error.';

        if (parts.length === 3) {
            status = parts[0];
            cost = parts[1];
            
            if (status === 'SUCCESS') {
                pathNodes = parts[2].split(' -> ').filter(n => n.length > 0);
                resultMsg = `✅ Reroute SUCCESS (Cost: ${cost}): ${parts[2]}`;
            } else {
                resultMsg = `❌ Reroute FAILED: ${parts[2]}`;
            }
        }
        
        // Fetch the current network state again (if needed, though 'data' already has it)
        const networkState = await runCXXCommand([]);

        res.json({
            result_message: resultMsg,
            path_nodes: pathNodes,
            ...networkState
        });

    } catch (err) {
        res.status(err.status || 500).send(err.message);
    }
});

// Serve the frontend files statically
app.use(express.static(path.join(__dirname, 'frontend')));

app.listen(PORT, () => {
    console.log(`Node.js Proxy running at http://localhost:${PORT}`);
});