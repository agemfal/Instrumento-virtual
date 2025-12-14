/*
==================================================================================
    SCRIPT.JS - Instrumento Virtual ESP32 - Versión con Pasos ADF4351 y AD9850
==================================================================================
*/

// ==========================================================
// VARIABLES GLOBALES Y ELEMENTOS DEL DOM
// ==========================================================
let websocket = null;
let isConnected = false;
let currentVfoMode = 'RX';

// Pasos predefinidos para ADF4351 (en Hz)
const ADF4351_STEPS = [
    10, 100, 1000, 10000, 100000, 1000000, 10000000
];
let currentAdf4351StepIndex = 2;

// Pasos predefinidos para AD9850 (en Hz)
const AD9850_STEPS = [
    1, 10, 100, 1000, 10000, 100000, 1000000
];
let currentAd9850StepIndex = 3;

// --- Lógica de Navegación: Generadores ---
const generatorSection = document.getElementById('generators-section');
const generatorModules = generatorSection.querySelectorAll('.instrument-module');
const genNameDisplay = document.getElementById('gen-name-display');
const genPrevBtn = document.getElementById('gen-prev-btn');
const genNextBtn = document.getElementById('gen-next-btn');
let currentGeneratorIndex = 0;

// --- Lógica de Navegación: Osciladores ---
const oscillatorSection = document.getElementById('oscillators-section');
const oscillatorModules = oscillatorSection.querySelectorAll('.instrument-module');
const oscNameDisplay = document.getElementById('osc-name-display');
const oscPrevBtn = document.getElementById('osc-prev-btn');
const oscNextBtn = document.getElementById('osc-next-btn');
let currentOscillatorIndex = 0;

// --- Interfaz General y Módulos ---
const connectBtn = document.getElementById('connect-btn'); 
const statusBar = document.getElementById('status-bar'); 
const responseLog = document.getElementById('response-log'); 
const esp32IpInput = document.getElementById('esp32-ip'); 
const scanBtn = document.getElementById('scan-i2c-btn'); 
const scanResultsDiv = document.getElementById('scan-results'); 

// Elementos VFO
const vfoFreqDisplay = document.getElementById('vfo-freq'); 
const vfoBandDisplay = document.getElementById('vfo-band'); 
const vfoStepDisplay = document.getElementById('vfo-step'); 
const vfoModeDisplay = document.getElementById('vfo-mode'); 
const vfoRxTxBtn = document.getElementById('vfo-rxtx-btn'); 

// Elementos AD9850
const ad9850FreqDisplay = document.getElementById('ad9850-freq-display'); 
const ad9850StepDisplay = document.getElementById('ad9850-step-display');
const ad9850StatusDisplay = document.getElementById('ad9850-status-display'); 
const ad9850FreqInput = document.getElementById('ad9850-freq-input'); 
const ad9850SetFreqBtn = document.getElementById('ad9850-set-freq-btn'); 
const ad9850EnableBtn = document.getElementById('ad9850-enable-btn'); 
const ad9850DisableBtn = document.getElementById('ad9850-disable-btn'); 
const ad9850FreqUpBtn = document.getElementById('ad9850-freq-up-btn');
const ad9850FreqDownBtn = document.getElementById('ad9850-freq-down-btn');
const ad9850StepBtn = document.getElementById('ad9850-step-btn');

// Elementos ADF4351
const adf4351FreqDisplay = document.getElementById('adf4351-freq-display'); 
const adf4351PowerDisplay = document.getElementById('adf4351-power-display'); 
const adf4351StepDisplay = document.getElementById('adf4351-step-display');
const adf4351StatusDisplay = document.getElementById('adf4351-status-display'); 
const adf4351FreqInput = document.getElementById('adf4351-freq-input'); 
const adf4351SetFreqBtn = document.getElementById('adf4351-set-freq-btn'); 
const adf4351PowerSelect = document.getElementById('adf4351-power-select'); 
const adf4351EnableBtn = document.getElementById('adf4351-enable-btn'); 
const adf4351DisableBtn = document.getElementById('adf4351-disable-btn');
const adf4351FreqUpBtn = document.getElementById('adf4351-freq-up-btn');
const adf4351FreqDownBtn = document.getElementById('adf4351-freq-down-btn');
const adf4351StepBtn = document.getElementById('adf4351-step-btn');

// ==========================================================
// LÓGICA DE CONEXIÓN Y MENSAJERÍA
// ==========================================================
function conectar() { 
    const ip = esp32IpInput.value.trim() || window.location.hostname; 
    if (!ip) { appendLog("⚠️ Por favor, ingresa una dirección IP."); return; } 
    const url = `ws://${ip}:81/`; 
    appendLog(`Intentando conectar a ${url}...`); 
    websocket = new WebSocket(url); 
    websocket.onopen = () => { isConnected = true; updateConnectionStatus(); appendLog("✅ Conectado al ESP32."); }; 
    websocket.onmessage = (event) => { procesarMensaje(event.data); }; 
    websocket.onerror = (error) => { appendLog(`❌ Error en la conexión: ${error.message || 'Error desconocido'}`); }; 
    websocket.onclose = () => { isConnected = false; updateConnectionStatus(); appendLog("🔌 Desconectado del ESP32"); }; 
}

function desconectar() { if (websocket) websocket.close(); }

function procesarMensaje(data) { 
    appendLog(`📨 Recibido: ${data}`); 
    try { 
        const msg = JSON.parse(data); 
        switch (msg.accion) { 
            case "respuesta_escaner": actualizarPantallaEscaner(msg); break; 
            case "respuesta_vfo": actualizarPantallaVFO(msg.datos); break; 
            case "respuesta_ad9850": actualizarPantallaAD9850(msg.datos); break; 
            case "respuesta_adf4351": actualizarPantallaADF4351(msg.datos); break; 
            case "respuesta_osc_select": appendLog(`✅ Confirmación: Switch RF activado para Oscilador ${msg.selected_id}.`); break; 
        } 
        if (msg.status === "error") { appendLog(`❌ Error desde ESP32: ${msg.mensaje}`); } 
    } catch (e) { 
        appendLog("⚠️ Mensaje recibido no es JSON: " + data); 
        console.error("Error al procesar JSON:", e); 
    } 
}

function enviarComando(data) { 
    console.log("DEBUG: La función 'enviarComando' fue llamada con:", data); // <--- DEBUG 1
    if (websocket && websocket.readyState === WebSocket.OPEN) { 
        const jsonString = JSON.stringify(data); 
        websocket.send(jsonString); 
        appendLog(`➡️ Enviado: ${jsonString}`); 
    } else { 
        appendLog("⚠️ No conectado. No se pudo enviar el comando."); 
    } 
}

// ==========================================================
// LÓGICA DE NAVEGACIÓN Y UI
// ==========================================================
function updateConnectionStatus() { statusBar.textContent = isConnected ? "Conectado" : "Desconectado"; statusBar.className = isConnected ? "status-connected" : "status-disconnected"; connectBtn.textContent = isConnected ? "Desconectar" : "Conectar"; }
function appendLog(msg) { responseLog.textContent += msg + "\n"; responseLog.scrollTop = responseLog.scrollHeight; }
function formatFrequency(hz) { if (!hz || isNaN(hz)) return "--"; const numHz = Number(hz); if (numHz >= 1000000000) return (numHz / 1000000000).toFixed(4) + ' GHz'; if (numHz >= 1000000) return (numHz / 1000000).toFixed(3) + ' MHz'; if (numHz >= 1000) return (numHz / 1000).toFixed(2) + ' kHz'; return numHz + ' Hz'; }
function formatStep(stepHz) { if (!stepHz || isNaN(stepHz)) return "--"; const numStep = Number(stepHz); if (numStep >= 1000000) return (numStep / 1000000).toFixed(0) + ' MHz'; if (numStep >= 1000) return (numStep / 1000).toFixed(0) + ' kHz'; return numStep + ' Hz'; }
function showGeneratorModule(index) { currentGeneratorIndex = index; generatorModules.forEach((module, i) => { module.style.display = (i === index) ? 'block' : 'none'; }); genNameDisplay.textContent = generatorModules[index].dataset.name || `Generador ${index + 1}`; }
function showOscillatorModule(index) { currentOscillatorIndex = index; oscillatorModules.forEach((module, i) => { module.style.display = (i === index) ? 'block' : 'none'; }); oscNameDisplay.textContent = oscillatorModules[index].dataset.name || `Oscilador ${index + 1}`; }
function actualizarPantallaEscaner(msg) { appendLog("📊 Procesando resultado del escaneo I2C..."); if (Array.isArray(msg.dispositivos) && msg.dispositivos.length > 0) { let html = "<ul>"; msg.dispositivos.forEach(addr => { const hexAddr = `0x${addr.toString(16).toUpperCase()}`; html += `<li>Dispositivo encontrado en <strong>${hexAddr}</strong> (Decimal: ${addr})</li>`; }); html += "</ul>"; scanResultsDiv.innerHTML = html; } else { scanResultsDiv.innerHTML = "<p>✅ Escaneo completo. No se detectaron dispositivos.</p>"; } }
function actualizarPantallaVFO(datos) { if (!datos) return; vfoFreqDisplay.textContent = (datos.frecuencia_hz / 1000000).toFixed(3); vfoStepDisplay.textContent = datos.paso_hz < 1000 ? `${datos.paso_hz} Hz` : `${datos.paso_hz / 1000} kHz`; vfoBandDisplay.textContent = datos.banda_nombre; vfoModeDisplay.textContent = datos.modo; currentVfoMode = datos.modo; vfoRxTxBtn.textContent = (currentVfoMode === 'RX') ? 'Cambiar a TX' : 'Cambiar a RX'; }
function actualizarPantallaAD9850(datos) { if (!datos) return; ad9850FreqDisplay.textContent = formatFrequency(datos.frecuencia_hz); ad9850StatusDisplay.textContent = datos.habilitado ? "Habilitado" : "Deshabilitado"; ad9850StatusDisplay.className = datos.habilitado ? 'status-on' : 'status-off'; if (datos.paso_hz) { ad9850StepDisplay.textContent = formatStep(datos.paso_hz); } }
function actualizarPantallaADF4351(datos) { if (!datos) return; adf4351FreqDisplay.textContent = formatFrequency(datos.frecuencia_hz); adf4351StatusDisplay.textContent = datos.habilitado ? "Habilitado" : "Deshabilitado"; adf4351StatusDisplay.className = datos.habilitado ? 'status-on' : 'status-off'; if (datos.paso_hz) { adf4351StepDisplay.textContent = formatStep(datos.paso_hz); } let powerText = ''; switch (datos.potencia) { case 0: powerText = '-4 dBm'; break; case 1: powerText = '-1 dBm'; break; case 2: powerText = '+2 dBm'; break; case 3: powerText = '+5 dBm'; break; default: powerText = 'Desconocido'; } adf4351PowerDisplay.textContent = powerText; adf4351PowerSelect.value = datos.potencia; }

// ==========================================================
// FUNCIONES PARA MANEJO DE PASOS ADF4351
// ==========================================================
function cambiarPasoADF4351() { currentAdf4351StepIndex = (currentAdf4351StepIndex + 1) % ADF4351_STEPS.length; const nuevoPaso = ADF4351_STEPS[currentAdf4351StepIndex]; adf4351StepDisplay.textContent = formatStep(nuevoPaso); enviarComando({ accion: "adf4351_command", sub_accion: "set_step", paso_hz: nuevoPaso }); appendLog(`🔧 Paso ADF4351 cambiado a: ${formatStep(nuevoPaso)}`); }
function ajustarFrecuenciaADF4351(direccion) { enviarComando({ accion: "adf4351_command", sub_accion: "change_freq", direccion: direccion }); const pasoActual = ADF4351_STEPS[currentAdf4351StepIndex]; appendLog(`🔧 Ajustando ADF4351: ${direccion === 'up' ? '+' : '-'}${formatStep(pasoActual)}`); }

// ==========================================================
// FUNCIONES PARA MANEJO DE PASOS AD9850
// ==========================================================
function cambiarPasoAD9850() { currentAd9850StepIndex = (currentAd9850StepIndex + 1) % AD9850_STEPS.length; const nuevoPaso = AD9850_STEPS[currentAd9850StepIndex]; ad9850StepDisplay.textContent = formatStep(nuevoPaso); enviarComando({ accion: "ad9850_command", sub_accion: "set_step", paso_hz: nuevoPaso }); appendLog(`🔧 Paso AD9850 cambiado a: ${formatStep(nuevoPaso)}`); }
function ajustarFrecuenciaAD9850(direccion) { enviarComando({ accion: "ad9850_command", sub_accion: "change_freq", direccion: direccion }); const pasoActual = AD9850_STEPS[currentAd9850StepIndex]; appendLog(`🔧 Ajustando AD9850: ${direccion === 'up' ? '+' : '-'}${formatStep(pasoActual)}`); }

// ==========================================================
// EVENT LISTENERS
// ==========================================================
connectBtn.addEventListener("click", () => { !isConnected ? conectar() : desconectar(); });
scanBtn.addEventListener("click", () => { enviarComando({ accion: "escanear_i2c" }); });

// Navegación
genNextBtn.addEventListener("click", () => { const nextIndex = (currentGeneratorIndex + 1) % generatorModules.length; showGeneratorModule(nextIndex); });
genPrevBtn.addEventListener("click", () => { const prevIndex = (currentGeneratorIndex - 1 + generatorModules.length) % generatorModules.length; showGeneratorModule(prevIndex); });
oscNextBtn.addEventListener("click", () => { const nextIndex = (currentOscillatorIndex + 1) % oscillatorModules.length; showOscillatorModule(nextIndex); });
oscPrevBtn.addEventListener("click", () => { const prevIndex = (currentOscillatorIndex - 1 + oscillatorModules.length) % oscillatorModules.length; showOscillatorModule(prevIndex); });

// Selección de Oscilador
const selectOscButtons = document.querySelectorAll('.select-osc-btn');
selectOscButtons.forEach(button => { button.addEventListener('click', (event) => { const oscId = parseInt(event.target.dataset.oscId, 10); enviarComando({ accion: "select_oscillator", id: oscId }); }); });

// Controles VFO
document.getElementById('vfo-freq-up-btn').addEventListener("click", () => enviarComando({ accion: "vfo_command", sub_accion: "change_freq", direccion: "up" })); 
document.getElementById('vfo-freq-down-btn').addEventListener("click", () => enviarComando({ accion: "vfo_command", sub_accion: "change_freq", direccion: "down" })); 
document.getElementById('vfo-step-btn').addEventListener("click", () => enviarComando({ accion: "vfo_command", sub_accion: "set_step" })); 
document.getElementById('vfo-band-btn').addEventListener("click", () => enviarComando({ accion: "vfo_command", sub_accion: "set_band" })); 
vfoRxTxBtn.addEventListener("click", () => { const nuevoModo = (currentVfoMode === 'RX') ? 'tx' : 'rx'; enviarComando({ accion: "vfo_command", sub_accion: "set_rxtx", modo: nuevoModo }); }); 

// Controles AD9850
ad9850SetFreqBtn.addEventListener("click", () => { const freqHz = parseInt(ad9850FreqInput.value); if (!isNaN(freqHz) && freqHz >= 0 && freqHz <= 40000000) { enviarComando({ accion: "ad9850_command", sub_accion: "set_freq", frecuencia_hz: freqHz }); } else { alert("Frecuencia para AD9850 debe estar entre 0 y 40,000,000 Hz."); } }); 
ad9850EnableBtn.addEventListener("click", () => enviarComando({ accion: "ad9850_command", sub_accion: "enable" })); 
ad9850DisableBtn.addEventListener("click", () => enviarComando({ accion: "ad9850_command", sub_accion: "disable" })); 

// Controles AD9850 con manejo de pasos --- ¡AQUÍ ESTÁ LA SECCIÓN CRÍTICA! ---
ad9850FreqUpBtn.addEventListener("click", () => {
    console.log("DEBUG: Botón 'AD9850 Freq >>' PRESIONADO."); // <--- DEBUG 2
    ajustarFrecuenciaAD9850('up');
});
ad9850FreqDownBtn.addEventListener("click", () => {
    console.log("DEBUG: Botón 'AD9850 Freq <<' PRESIONADO."); // <--- DEBUG 3
    ajustarFrecuenciaAD9850('down');
});
ad9850StepBtn.addEventListener("click", () => {
    cambiarPasoAD9850();
});

// Controles ADF4351
adf4351SetFreqBtn.addEventListener("click", () => { const freqHz = parseInt(adf4351FreqInput.value); if (!isNaN(freqHz) && freqHz >= 35000000 && freqHz <= 4400000000) { enviarComando({ accion: "adf4351_command", sub_accion: "set_freq", frecuencia_hz: freqHz }); } else { alert("Frecuencia para ADF4351 debe estar entre 35 MHz y 4400 MHz."); } }); 
adf4351EnableBtn.addEventListener("click", () => enviarComando({ accion: "adf4351_command", sub_accion: "enable" })); 
adf4351DisableBtn.addEventListener("click", () => enviarComando({ accion: "adf4351_command", sub_accion: "disable" })); 
adf4351PowerSelect.addEventListener("change", () => { const powerLevel = parseInt(adf4351PowerSelect.value); enviarComando({ accion: "adf4351_command", sub_accion: "set_power", potencia: powerLevel }); });
adf4351FreqUpBtn.addEventListener("click", () => { ajustarFrecuenciaADF4351('up'); });
adf4351FreqDownBtn.addEventListener("click", () => { ajustarFrecuenciaADF4351('down'); });
adf4351StepBtn.addEventListener("click", () => { cambiarPasoADF4351(); });
document.getElementById('btn-toggle-rf').addEventListener("click", () => { enviarComando({ accion: "adf4351_command", sub_accion: "toggle_rf" }); });

// ==========================================================
// INICIALIZACIÓN
// ==========================================================
document.addEventListener("DOMContentLoaded", () => {
    updateConnectionStatus();
    showGeneratorModule(0);
    showOscillatorModule(0);
    
    adf4351StepDisplay.textContent = formatStep(ADF4351_STEPS[currentAdf4351StepIndex]);
    ad9850StepDisplay.textContent = formatStep(AD9850_STEPS[currentAd9850StepIndex]);
    
    console.log("DEBUG: El script se ha cargado completamente. Los listeners están activos."); // <--- DEBUG 4
    appendLog("Interfaz lista. Ingresa la IP del ESP32 y presiona Conectar.");
});
