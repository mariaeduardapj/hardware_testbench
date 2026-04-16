function runTest(testName) {
    const log = document.getElementById('log');
    const ultrasonicSection = document.getElementById('ultrasonic-section');

    // If test is ultrasonic, show the distance section and wait for user to click read
    if (testName === 'ultrasonic') {
        ultrasonicSection.classList.remove('hidden');
        log.innerText = "Ultrasonic sensor active. Click below to read.";
    } else {
        // For other tests, hide the ultrasonic section and run the test immediately
        ultrasonicSection.classList.add('hidden');
        log.innerText = "Init " + testName + "...";
        
        fetch(`/run?test=${testName}`)
            .then(response => response.text())
            .then(data => {
                log.innerText = data;
            });
    }
}

function getDistance() {
    const display = document.getElementById('dist-value');
    const log = document.getElementById('log');
    display.innerText = "Reading...";

    fetch('/ultrasonic')
        .then(response => response.text())
        .then(data => {
            display.innerText = data; 
            // The test finish if the distance is read
            log.innerText = "Test ultrasonic completed!";
        })
        .catch(err => {
            display.innerText = "Error";
            log.innerText = "Connection error";
        });
}


function getSystemInfo() {
    fetch('/info')
        .then(response => response.text())
        .then(data => {
            document.getElementById('system-version').innerText = "Firmware: " + data;
            document.getElementById('conn-status').innerText = "ONLINE";
            document.getElementById('conn-status').style.color = "#4caf50";
        })
        .catch(() => {
            document.getElementById('conn-status').innerText = "OFFLINE";
            document.getElementById('conn-status').style.color = "#f44336";
        });
}

window.onload = getSystemInfo;

function addLog(message) {
    const logList = document.getElementById('event-log');
    const now = new Date();
    const time = now.getHours().toString().padStart(2, '0') + ":" + 
                 now.getMinutes().toString().padStart(2, '0') + ":" + 
                 now.getSeconds().toString().padStart(2, '0');


    const newEntry = document.createElement('li');
    newEntry.innerHTML = `<strong>[${time}]</strong> ${message}`;
    

    logList.insertBefore(newEntry, logList.firstChild);


    if (logList.children.length > 10) {
        logList.removeChild(logList.lastChild);
    }
}


function runTest(testName) {
    addLog(`Initiated: ${testName}`);
    
    fetch(`/run?test=${testName}`)
        .then(response => response.text())
        .then(data => {
            addLog(`Result: ${data}`);
        });
}