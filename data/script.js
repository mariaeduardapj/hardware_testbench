const state = {
    catalog: null,
    devices: []
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

    // Show or hide the second GPIO field according to the selected device type.
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

function renderCatalog() {
    if (!state.catalog || !typeField) {
        return;
    }
    
    // Build the device-type selector from the firmware catalog instead of hardcoding options.
    typeField.innerHTML = state.catalog.deviceTypes
        .map((item) => `<option value="${item.id}">${item.label}</option>`)
        .join("");

    if (boardList) {
        boardList.textContent = `Tested support: ${state.catalog.supportedBoards.join(", ")}. Other ESP32 variants with equivalent GPIO behavior may work, but are not validated here yet.`;
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

    // Render one card per configured device instance stored in the ESP32.
    deviceList.className = "device-list";
    deviceList.innerHTML = state.devices
        .map((device) => {
            const pins = device.hasSecondPin
                ? `GPIOs ${device.pinA} / ${device.pinB}`
                : `GPIO ${device.pinA}`;

            return `
                <article class="device-card">
                    <div>
                        <p class="device-type">${device.typeLabel}</p>
                        <h3>${device.name}</h3>
                        <p class="device-pins">${pins}</p>
                    </div>
                    <div class="device-actions">
                        <button class="ghost-btn" type="button" onclick="runDeviceTest(${device.id})">Run test</button>
                        <button class="ghost-btn danger" type="button" onclick="removeDevice(${device.id})">Remove</button>
                    </div>
                </article>
            `;
        })
        .join("");
}

function getDeviceName(id) {
    return state.devices.find((device) => device.id === id)?.name || `Device ${id}`;
}

async function fetchCatalog() {
    const response = await fetch("/catalog");
    if (!response.ok) {
        throw new Error("Unable to load catalog.");
    }

    // The catalog tells the UI which device types are currently implemented in firmware.
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
        // The firmware keeps the source of truth, so registration always happens server-side.
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
        const response = await fetch(`/run?id=${id}`, { method: "POST" });
        const message = await response.text();

        if (!response.ok) {
            throw new Error(message);
        }

        setStatus(message, "success");
        addLog(`${name}: ${message}`);
    } catch (error) {
        setStatus(error.message, "error");
        addLog(`${name}: ${error.message}`);
    }
}

async function removeDevice(id) {
    const name = getDeviceName(id);
    try {
        // Tests run by device id, which lets multiple devices of the same type coexist.
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
        // Load connection state first, then hydrate the interface from firmware data.
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
window.removeDevice = removeDevice;

window.addEventListener("load", initializeInterface);
