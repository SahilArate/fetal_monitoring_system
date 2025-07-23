#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <MPU6050.h>

MPU6050 mpu;

const char* ssid = "NARZO 70 Pro 5G";        //  Suyog_sk
const char* password = "87654321";    //87654321

ESP8266WebServer server(80);

int16_t ax, ay, az, gx, gy, gz;
int16_t baseAx = 0, baseAy = 0, baseAz = 0;
int16_t prevAx = 0, prevAy = 0, prevAz = 0;

bool isCalibrated = false;
int calibrationSamples = 50;
int calibrationCount = 0;
long calibrationSumX = 0, calibrationSumY = 0, calibrationSumZ = 0;

unsigned int movementCount = 0;
unsigned int kickCount = 0;

const int movementThreshold = 1500;
const int kickThreshold = 6000;
const unsigned long movementCooldown = 800;
const unsigned long kickCooldown = 1500;
unsigned long lastMovementTime = 0;
unsigned long lastKickTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(4, 5);

  mpu.initialize();
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setDLPFMode(MPU6050_DLPF_BW_20);

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  if (!isCalibrated) {
    if (calibrationCount < calibrationSamples) {
      calibrationSumX += ax;
      calibrationSumY += ay;
      calibrationSumZ += az;
      calibrationCount++;
      delay(20);
      return;
    } else {
      baseAx = calibrationSumX / calibrationSamples;
      baseAy = calibrationSumY / calibrationSamples;
      baseAz = calibrationSumZ / calibrationSamples;
      isCalibrated = true;
    }
  }

  int16_t relativeAx = ax - baseAx;
  int16_t relativeAy = ay - baseAy;
  int16_t relativeAz = az - baseAz;

  int16_t axDelta = abs(relativeAx - prevAx);
  int16_t ayDelta = abs(relativeAy - prevAy);
  int16_t azDelta = abs(relativeAz - prevAz);

  bool significantDeviation = (abs(relativeAx) > movementThreshold ||
                               abs(relativeAy) > movementThreshold ||
                               abs(relativeAz) > movementThreshold);
  bool significantChange = (axDelta > movementThreshold / 2 ||
                            ayDelta > movementThreshold / 2 ||
                            azDelta > movementThreshold / 2);

  if ((significantDeviation || significantChange) &&
      (millis() - lastMovementTime > movementCooldown)) {
    movementCount++;
    lastMovementTime = millis();
  }

  int totalDelta = axDelta + ayDelta + (azDelta * 2);
  if (totalDelta > kickThreshold &&
      (millis() - lastKickTime > kickCooldown)) {
    kickCount++;
    lastKickTime = millis();
  }

  prevAx = relativeAx;
  prevAy = relativeAy;
  prevAz = relativeAz;

  delay(50);
}

void handleData() {
  String json = "{";
  json += "\"ax\":" + String((float)(ax - baseAx) / 1000.0, 2) + ",";
  json += "\"ay\":" + String((float)(ay - baseAy) / 1000.0, 2) + ",";
  json += "\"az\":" + String((float)(az - baseAz) / 1000.0, 2) + ",";
  json += "\"movementCount\":" + String(movementCount) + ",";
  json += "\"kickCount\":" + String(kickCount);
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  String html = R"rawliteral(
 <!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Mother's Movement Monitor</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      background: linear-gradient(135deg, #fff0f3 0%, #ffccd5 100%);
      font-family: 'Poppins', sans-serif;
      overflow-x: hidden;
      min-height: 100vh;
      color: #333;
    }

    /* Improved Floating Bubble Animation */
    .bubble-container {
      position: fixed;
      width: 100%;
      height: 100%;
      top: 0;
      left: 0;
      pointer-events: none;
      z-index: 0;
      overflow: hidden;
    }

    .bubble {
      position: absolute;
      border-radius: 50%;
      background: radial-gradient(circle at 30% 30%, rgba(255, 255, 255, 0.9), rgba(255, 255, 255, 0.4));
      box-shadow: 0 4px 20px rgba(255, 182, 193, 0.3);
      animation: float 15s infinite ease-in-out;
      opacity: 0;
    }

    @keyframes float {
      0% {
        transform: translate(0, 100vh) scale(0.2);
        opacity: 0;
      }
      5% {
        opacity: 0.8;
      }
      90% {
        opacity: 0.6;
      }
      100% {
        transform: translate(var(--tx), -100vh) scale(1);
        opacity: 0;
      }
    }

    header {
      background: linear-gradient(135deg, #ff8fab 0%, #ff5d8f 100%);
      padding: 2.5rem 1rem;
      text-align: center;
      border-bottom-left-radius: 40px;
      border-bottom-right-radius: 40px;
      box-shadow: 0 10px 30px rgba(255, 93, 143, 0.3);
      position: relative;
      z-index: 2;
      margin-bottom: 2rem;
    }

    h1 {
      margin: 0;
      font-size: 2.5rem;
      color: #fff;
      font-weight: 700;
      letter-spacing: 1px;
      text-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    }

    header p {
      color: #fff;
      margin-top: 0.5rem;
      font-size: 1.1rem;
      opacity: 0.9;
    }

    .heart-icon {
      display: inline-block;
      animation: heartbeat 1.5s infinite ease-in-out;
      transform-origin: center;
    }

    @keyframes heartbeat {
      0% { transform: scale(1); }
      14% { transform: scale(1.3); }
      28% { transform: scale(1); }
      42% { transform: scale(1.3); }
      70% { transform: scale(1); }
    }

    .container {
      max-width: 1200px;
      margin: 0 auto;
      padding: 0 1.5rem 3rem;
      position: relative;
      z-index: 1;
    }

    .dashboard {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 1.5rem;
      margin-bottom: 2rem;
    }

    .card {
      background: rgba(255, 255, 255, 0.9);
      backdrop-filter: blur(10px);
      border-radius: 16px;
      overflow: hidden;
      box-shadow: 0 8px 30px rgba(0, 0, 0, 0.1);
      transition: all 0.3s ease;
    }

    .card:hover {
      transform: translateY(-8px);
      box-shadow: 0 15px 40px rgba(255, 93, 143, 0.3);
    }

    .chart-card {
      padding: 1.5rem;
      margin-top: 2rem;
    }

    .stats-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
      gap: 1.5rem;
    }

    .stat-block {
      padding: 1.8rem 1rem;
      text-align: center;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      transition: transform 0.3s ease;
      position: relative;
      border-top: 5px solid transparent;
    }

    .stat-block:hover {
      transform: translateY(-5px);
    }

    .stat-block:nth-child(1) {
      border-top-color: #0077b6;
    }

    .stat-block:nth-child(2) {
      border-top-color: #00b37a;
    }

    .stat-block:nth-child(3) {
      border-top-color: #d62828;
    }

    .stat-block:nth-child(4) {
      border-top-color: #ff9e00;
    }

    .stat-block:nth-child(5) {
      border-top-color: #ff0054;
    }

    .stat-icon {
      font-size: 1.8rem;
      margin-bottom: 0.5rem;
      width: 50px;
      height: 50px;
      display: flex;
      align-items: center;
      justify-content: center;
      border-radius: 50%;
      background-color: #f8f9fa;
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
    }

    .stat-block:nth-child(1) .stat-icon {
      color: #0077b6;
    }

    .stat-block:nth-child(2) .stat-icon {
      color: #00b37a;
    }

    .stat-block:nth-child(3) .stat-icon {
      color: #d62828;
    }

    .stat-block:nth-child(4) .stat-icon {
      color: #ff9e00;
    }

    .stat-block:nth-child(5) .stat-icon {
      color: #ff0054;
    }

    .stat-block h3 {
      margin: 0.8rem 0 0.3rem;
      font-size: 0.9rem;
      font-weight: 500;
      color: #666;
      text-transform: uppercase;
      letter-spacing: 1px;
    }

    .stat-block span {
      font-size: 1.8rem;
      font-weight: 600;
      line-height: 1.2;
      display: block;
      margin-bottom: 0.3rem;
    }

    .stat-block:nth-child(1) span {
      color: #0077b6;
    }

    .stat-block:nth-child(2) span {
      color: #00b37a;
    }

    .stat-block:nth-child(3) span {
      color: #d62828;
    }

    .stat-block:nth-child(4) span {
      color: #ff9e00;
    }

    .stat-block:nth-child(5) span {
      color: #ff0054;
    }

    .stat-block p {
      font-size: 0.8rem;
      color: #888;
    }

    .chart-container {
      position: relative;
      margin: 0.5rem 0 1rem;
      height: 400px;
    }

    canvas {
      border-radius: 8px;
    }

    .btn-container {
      text-align: center;
      margin-top: 2rem;
    }

    .btn {
      background: linear-gradient(135deg, #ff8fab 0%, #ff5d8f 100%);
      border: none;
      padding: 12px 28px;
      color: white;
      font-size: 1rem;
      font-weight: 500;
      border-radius: 30px;
      cursor: pointer;
      box-shadow: 0 4px 15px rgba(255, 93, 143, 0.4);
      transition: all 0.3s ease;
      display: inline-flex;
      align-items: center;
      gap: 8px;
    }

    .btn:hover {
      transform: translateY(-3px);
      box-shadow: 0 8px 25px rgba(255, 93, 143, 0.5);
    }

    .btn:active {
      transform: translateY(0);
    }

    .section-title {
      font-size: 1.5rem;
      color: #333;
      margin-bottom: 1.5rem;
      text-align: center;
      position: relative;
      padding-bottom: 0.8rem;
    }

    .section-title::after {
      content: "";
      position: absolute;
      bottom: 0;
      left: 50%;
      transform: translateX(-50%);
      width: 80px;
      height: 3px;
      background: linear-gradient(to right, #ff8fab, #ff5d8f);
      border-radius: 3px;
    }

    .status-indicator {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      margin-top: 1rem;
      font-size: 0.9rem;
      color: #555;
    }

    .indicator-dot {
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background-color: #4CAF50;
      display: inline-block;
      animation: blink 2s infinite;
    }

    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.4; }
      100% { opacity: 1; }
    }

    @media screen and (max-width: 768px) {
      h1 {
        font-size: 2rem;
      }

      .dashboard {
        grid-template-columns: 1fr;
      }

      .stats-grid {
        grid-template-columns: repeat(auto-fit, minmax(120px, 1fr));
      }

      .stat-block span {
        font-size: 1.5rem;
      }

      .chart-container {
        height: 300px;
      }
    }
  </style>
</head>
<body>
  <div class="bubble-container">
    <!-- Bubbles will be created by JavaScript -->
  </div>

  <header>
    <h1>Baby's Kick & Movement Monitor</h1>
    <p>Your baby's activity, monitored with care <span class="heart-icon">💖</span></p>
  </header>

  <div class="container">
    <h2 class="section-title">Real-Time Measurements</h2>
    
    <div class="dashboard">
      <div class="card">
        <div class="stats-grid">
          <div class="stat-block">
            <div class="stat-icon">X</div>
            <h3>X-Axis</h3>
            <span id="ax">--</span>
            <p>g-force</p>
          </div>
          <div class="stat-block">
            <div class="stat-icon">Y</div>
            <h3>Y-Axis</h3>
            <span id="ay">--</span>
            <p>g-force</p>
          </div>
          <div class="stat-block">
            <div class="stat-icon">Z</div>
            <h3>Z-Axis</h3>
            <span id="az">--</span>
            <p>g-force</p>
          </div>
          <div class="stat-block">
            <div class="stat-icon">👶</div>
            <h3>Movements</h3>
            <span id="movement">--</span>
            <p>detected</p>
          </div>
          <div class="stat-block">
            <div class="stat-icon">👣</div>
            <h3>Kicks</h3>
            <span id="kick">--</span>
            <p>detected</p>
          </div>
        </div>
      </div>
    </div>

    <div class="status-indicator">
      <span class="indicator-dot"></span>
      <span>Monitoring active</span>
    </div>

    <div class="card chart-card">
      <h2 class="section-title">Activity Graph</h2>
      <div class="chart-container">
        <canvas id="azChart"></canvas>
      </div>
      <div class="btn-container">
        <button class="btn" onclick="downloadCSV()">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path>
            <polyline points="7 10 12 15 17 10"></polyline>
            <line x1="12" y1="15" x2="12" y2="3"></line>
          </svg>
          Download Report
        </button>
      </div>
    </div>
  </div>

  <script>
    // Create animated bubbles
    const bubbleContainer = document.querySelector('.bubble-container');
    const bubbleCount = 15;
    
    for (let i = 0; i < bubbleCount; i++) {
      const size = Math.random() * 100 + 30;
      const bubble = document.createElement('div');
      bubble.className = 'bubble';
      bubble.style.width = `${size}px`;
      bubble.style.height = `${size}px`;
      bubble.style.left = `${Math.random() * 100}%`;
      bubble.style.animationDelay = `${Math.random() * 15}s`;
      bubble.style.animationDuration = `${15 + Math.random() * 15}s`;
      bubble.style.setProperty('--tx', `${(Math.random() - 0.5) * 50}vw`);
      bubbleContainer.appendChild(bubble);
    }

    let labels = [], axData = [], ayData = [], azData = [], kickData = [];
    let movementLog = [], kickLog = [];

    const ctx = document.getElementById('azChart').getContext('2d');
    
    Chart.defaults.font.family = "'Poppins', sans-serif";
    Chart.defaults.color = '#555';
    
    const azChart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: labels,
        datasets: [
          {
            label: 'X-Axis',
            data: axData,
            borderColor: '#0077b6',
            backgroundColor: 'rgba(0, 119, 182, 0.1)',
            fill: true,
            tension: 0.4,
            borderWidth: 2
          },
          {
            label: 'Y-Axis',
            data: ayData,
            borderColor: '#00b37a',
            backgroundColor: 'rgba(0, 179, 122, 0.1)',
            fill: true,
            tension: 0.4,
            borderWidth: 2
          },
          {
            label: 'Z-Axis',
            data: azData,
            borderColor: '#d62828',
            backgroundColor: 'rgba(214, 40, 40, 0.1)',
            fill: true,
            tension: 0.4,
            borderWidth: 2
          },
          {
            label: 'Kicks',
            data: kickData,
            borderColor: '#ff0054',
            backgroundColor: 'rgba(255, 0, 84, 0.5)',
            borderDash: [5, 5],
            fill: false,
            showLine: false,
            pointRadius: 8,
            pointStyle: 'star'
          }
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        interaction: {
          mode: 'index',
          intersect: false
        },
        scales: {
          y: {
            beginAtZero: true,
            grid: {
              color: 'rgba(0, 0, 0, 0.05)'
            }
          },
          x: {
            grid: {
              display: false
            }
          }
        },
        plugins: {
          legend: {
            position: 'top',
            labels: {
              boxWidth: 15,
              usePointStyle: true,
              padding: 20
            }
          },
          tooltip: {
            backgroundColor: 'rgba(255, 255, 255, 0.9)',
            titleColor: '#333',
            bodyColor: '#666',
            bodyFont: {
              size: 13
            },
            titleFont: {
              size: 14,
              weight: 'bold'
            },
            borderColor: 'rgba(0, 0, 0, 0.1)',
            borderWidth: 1,
            cornerRadius: 8,
            padding: 10,
            displayColors: true,
            usePointStyle: true,
            boxWidth: 8
          }
        }
      }
    });

    function fetchData() {
      fetch("/data")
        .then(res => res.json())
        .then(data => {
          const time = new Date().toLocaleTimeString();
          const ax = data.ax;
          const ay = data.ay;
          const az = data.az;
          const move = data.movementCount;
          const kick = data.kickCount;

          document.getElementById("ax").textContent = ax;
          document.getElementById("ay").textContent = ay;
          document.getElementById("az").textContent = az;
          document.getElementById("movement").textContent = move;
          document.getElementById("kick").textContent = kick;

          // Keep the last 40 data points for the chart
          if (labels.length > 40) {
            labels.shift(); axData.shift(); ayData.shift(); azData.shift(); kickData.shift();
          }

          labels.push(time);
          axData.push(ax);
          ayData.push(ay);
          azData.push(az);
          
          // Add star markers for kicks
          const lastKickValue = kickLog.length > 0 ? kickLog[kickLog.length - 1].value : 0;
          kickData.push(kick > lastKickValue ? Math.max(ax, ay, az, 3) : null);

          movementLog.push({ time, value: move });
          kickLog.push({ time, value: kick });

          azChart.update();
        })
        .catch(err => {
          console.error("Error fetching data:", err);
        });
    }

    setInterval(fetchData, 2000);

    function downloadCSV() {
      let csv = "Time,X,Y,Z,Movements,Kicks\n";
      
      for (let i = 0; i < labels.length; i++) {
        let time = labels[i] || '';
        let x = axData[i] || '';
        let y = ayData[i] || '';
        let z = azData[i] || '';
        let move = i < movementLog.length ? movementLog[i].value : '';
        let kick = i < kickLog.length ? kickLog[i].value : '';
        
        csv += `${time},${x},${y},${z},${move},${kick}\n`;
      }
      
      const blob = new Blob([csv], { type: 'text/csv' });
      const link = document.createElement("a");
      link.href = URL.createObjectURL(blob);
      link.download = "movement_data_" + new Date().toISOString().slice(0,10) + ".csv";
      
      // Animation for download button
      const btn = document.querySelector('.btn');
      btn.innerHTML = "Downloading...";
      
      setTimeout(() => {
        link.click();
        btn.innerHTML = `<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path>
          <polyline points="7 10 12 15 17 10"></polyline>
          <line x1="12" y1="15" x2="12" y2="3"></line>
        </svg> Download Report`;
      }, 500);
    }
  </script>
</body>
</html>
  )rawliteral";

  server.send(200, "text/html", html);
}



 