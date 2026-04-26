const state = {
    catalog: null,
    devices: [],
    pollingTimers: {}
};

const typeField = document.getElementById("device-type");
const nameField = document.getElementById("device-name");
const pinAField = document.getElementById("pin-a");
const pinBField = document.getElementById("pin-b");
const pinAInfo = document.getElementById("pin-a-label");
const pinBInfo = document.getElementById("pin-b-label");
const pinBWrapper = document.getElementById("pin-b-wrapper");
const deviceList = document.getElementById("device-list");
const boardList = document.getElementById("board-list");
const statusBanner = document.getElementById("status-banner");
const eventLog = document.getElementById("event-log");

function addLog(message) {
    const timestamp = new Date().toLocaleTimeString("en-GB");
    const entry = document.createElement("li");
    entry.innerHTML = `<strong>[${timestamp}]</strong> ${message}`;
    eventLog.prepend(entry);

    while (eventLog.children.length > 12) {
        eventLog.removeChild(eventLog.lastChild);
    }
}

function setStatus(message, tone = "neutral") {
    statusBanner.textContent = message;
    statusBanner.dataset.tone = tone;
}

function escapeHtml(value) {
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;");
}

function getSelectedTypeMeta() {
    return state.catalog?.deviceTypes.find((item) => item.id === typeField.value) || null;
}

function updatePinFields() {
    const selected = getSelectedTypeMeta();
    if (!selected) {
        return;
    }

    pinAInfo.textContent = selected.pinLabels[0] || "Primary GPIO";
    nameField.placeholder = `Example: ${selected.label} #1`;

    if (selected.pins > 1) {
        pinBWrapper.classList.remove("hidden");
        pinBField.required = true;
        pinBInfo.textContent = selected.pinLabels[1] || "Secondary GPIO";
    } else {
        pinBWrapper.classList.add("hidden");
        pinBField.required = false;
        pinBField.value = "";
    }
}

function getResultTone(value) {
    if (value === "PASS") {
        return "pass";
    }
    if (value === "FAIL") {
        return "fail";
    }
    return "pending";
}

function formatTimestamp(timestampMs) {
    if (!timestampMs) {
        return "Not available yet";
    }
    return `${timestampMs} ms since ESP32 boot`;
}

function buildActionButtons(device) {
    const test = device.test;
    const disableRun = test.status === "Running" ? "disabled" : "";
    const finalizeButtons = test.status === "Completed" && test.userCanOverride
        ? `
            <button class="ghost-btn" type="button" onclick="finalizeTest(${device.id}, '${test.suggestedResult}')">
                Confirm ${test.suggestedResult}
            </button>
            <button class="ghost-btn danger" type="button" onclick="finalizeTest(${device.id}, 'FAIL')">
                Force FAIL
            </button>
            <button class="ghost-btn" type="button" onclick="finalizeTest(${device.id}, 'PASS')">
                Force PASS
            </button>
        `
        : "";

    return `
        <button class="ghost-btn" type="button" onclick="runDeviceTest(${device.id})" ${disableRun}>Run test</button>
        ${finalizeButtons}
        <button class="ghost-btn danger" type="button" onclick="removeDevice(${device.id})">Remove</button>
    `;
}

function renderCatalog() {
    if (!state.catalog || !typeField) {
        return;
    }

    typeField.innerHTML = state.catalog.deviceTypes
        .map((item) => `<option value="${item.id}">${item.label}</option>`)
        .join("");

    if (boardList) {
        boardList.textContent = `Tested support: ${state.catalog.supportedBoards.join(", ")}.`;
    }

    document.getElementById("compatibility-note").textContent = state.catalog.compatibility;
    updatePinFields();
}

function renderDevices() {
    if (!state.devices.length) {
        deviceList.className = "device-list empty-state";
        deviceList.textContent = "No devices registered yet.";
        return;
    }

    deviceList.className = "device-list";
    deviceList.innerHTML = state.devices
        .map((device) => {
            const pins = device.hasSecondPin
                ? `GPIOs ${device.pinA} / ${device.pinB}`
                : `GPIO ${device.pinA}`;
            const test = device.test;
            const suggestedTone = getResultTone(test.suggestedResult);
            const finalTone = getResultTone(test.finalResult);

            return `
                <article class="device-card">
                    <div class="device-main">
                        <p class="device-type">${escapeHtml(device.typeLabel)}</p>
                        <h3>${escapeHtml(device.name)}</h3>
                        <p class="device-pins">${pins}</p>

                        <div class="test-grid">
                            <div class="test-pill">
                                <span class="meta-label">Status</span>
                                <strong>${escapeHtml(test.status)}</strong>
                            </div>
                            <div class="test-pill result-${suggestedTone}">
                                <span class="meta-label">Suggested</span>
                                <strong>${escapeHtml(test.suggestedResult)}</strong>
                            </div>
                            <div class="test-pill result-${finalTone}">
                                <span class="meta-label">Final</span>
                                <strong>${escapeHtml(test.finalResult)}</strong>
                            </div>
                        </div>

                        <p class="test-message">${escapeHtml(test.message)}</p>
                        <p class="test-timestamp">Timestamp: ${escapeHtml(formatTimestamp(test.timestampMs))}</p>
                    </div>

                    <div class="device-actions">
                        ${buildActionButtons(device)}
                    </div>
                </article>
            `;
        })
        .join("");
}

function getDeviceName(id) {
    return state.devices.find((device) => device.id === id)?.name || `Device ${id}`;
}

function updateDeviceTestState(id, testState) {
    const device = state.devices.find((item) => item.id === id);
    if (!device) {
        return;
    }

    device.test = testState;
    renderDevices();
}

function stopPolling(id) {
    if (state.pollingTimers[id]) {
        clearInterval(state.pollingTimers[id]);
        delete state.pollingTimers[id];
    }
}

function startPolling(id) {
    stopPolling(id);

    state.pollingTimers[id] = setInterval(async () => {
        try {
            const response = await fetch(`/run/status?id=${id}`);
            if (!response.ok) {
                throw new Error(await response.text());
            }

            const testState = await response.json();
            updateDeviceTestState(id, testState);

            if (testState.status === "Completed") {
                stopPolling(id);

                const tone = testState.suggestedResult === "FAIL" ? "error" : "success";
                setStatus(`${getDeviceName(id)}: ${testState.message}`, tone);
                addLog(`${getDeviceName(id)} completed with suggested result ${testState.suggestedResult}.`);
            }
        } catch (error) {
            stopPolling(id);
            setStatus(error.message, "error");
            addLog(`Polling failed for ${getDeviceName(id)}: ${error.message}`);
        }
    }, 700);
}

async function fetchCatalog() {
    const response = await fetch("/catalog");
    if (!response.ok) {
        throw new Error("Unable to load catalog.");
    }

    state.catalog = await response.json();
    renderCatalog();
}

async function fetchDevices() {
    const response = await fetch("/devices");
    if (!response.ok) {
        throw new Error("Unable to load configured devices.");
    }

    state.devices = await response.json();
    renderDevices();
}

async function fetchSystemInfo() {
    try {
        const response = await fetch("/info");
        const info = await response.text();
        document.getElementById("system-version").textContent = info;
        document.getElementById("conn-status").textContent = "ONLINE";
        document.getElementById("conn-status").classList.remove("offline");
    } catch (error) {
        document.getElementById("conn-status").textContent = "OFFLINE";
        document.getElementById("conn-status").classList.add("offline");
        throw error;
    }
}

async function registerDevice(event) {
    event.preventDefault();

    const selected = getSelectedTypeMeta();
    const params = new URLSearchParams({
        type: typeField.value,
        name: nameField.value.trim(),
        pinA: pinAField.value
    });

    if (selected?.pins > 1) {
        params.append("pinB", pinBField.value);
    }

    try {
        const response = await fetch(`/devices/add?${params.toString()}`, {
            method: "POST"
        });
        const message = await response.text();

        if (!response.ok) {
            throw new Error(message);
        }

        setStatus(message, "success");
        addLog(`Registered ${nameField.value.trim()} (${selected.label}).`);
        event.target.reset();
        updatePinFields();
        await fetchDevices();
    } catch (error) {
        setStatus(error.message, "error");
        addLog(`Registration failed: ${error.message}`);
    }
}

async function runDeviceTest(id) {
    const name = getDeviceName(id);
    setStatus(`Running ${name}...`);
    addLog(`Started test for ${name}.`);

    try {
        const response = await fetch(`/run/start?id=${id}`, { method: "POST" });
        if (!response.ok) {
            throw new Error(await response.text());
        }

        const testState = await response.json();
        updateDeviceTestState(id, testState);
        startPolling(id);
    } catch (error) {
        setStatus(error.message, "error");
        addLog(`${name}: ${error.message}`);
    }
}

async function finalizeTest(id, result) {
    const name = getDeviceName(id);

    try {
        const response = await fetch(`/run/finalize?id=${id}&result=${result}`, { method: "POST" });
        if (!response.ok) {
            throw new Error(await response.text());
        }

        const testState = await response.json();
        updateDeviceTestState(id, testState);

        const tone = result === "FAIL" ? "error" : "success";
        setStatus(`${name}: final result ${result}.`, tone);
        addLog(`${name}: final result set to ${result}.`);
    } catch (error) {
        setStatus(error.message, "error");
        addLog(`${name}: finalize failed: ${error.message}`);
    }
}

async function removeDevice(id) {
    const name = getDeviceName(id);
    stopPolling(id);

    try {
        const response = await fetch(`/devices/remove?id=${id}`, { method: "POST" });
        const message = await response.text();

        if (!response.ok) {
            throw new Error(message);
        }

        setStatus(`${name} removed.`, "neutral");
        addLog(`${name} removed from bench.`);
        await fetchDevices();
    } catch (error) {
        setStatus(error.message, "error");
        addLog(`Removal failed: ${error.message}`);
    }
}

async function initializeInterface() {
    try {
        await fetchSystemInfo();
        await fetchCatalog();
        await fetchDevices();
        setStatus("Ready to register devices and run tests.");
        addLog("Catalog and device registry loaded.");
    } catch (error) {
        setStatus("Unable to reach the ESP32 web interface.", "error");
        addLog("Connection to firmware failed.");
    }
}

document.getElementById("device-form").addEventListener("submit", registerDevice);
typeField.addEventListener("change", updatePinFields);
window.runDeviceTest = runDeviceTest;
window.finalizeTest = finalizeTest;
window.removeDevice = removeDevice;

window.addEventListener("load", initializeInterface);
