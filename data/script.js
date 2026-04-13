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