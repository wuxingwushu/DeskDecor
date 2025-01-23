#include "Network.h"
#include "EepromString.h"
#include "DEV_Config.h"
#include "Debug.h"
#include <WiFi.h>

WebServer server(80);

NetworkCase ConnectWIFI()
{
  // 读取字符串
  String ssidConfig = readStringFromEEPROM(WifiNameAddr);
  String passwordConfig = readStringFromEEPROM(WifiPassAddr);

  Debug("链接：");
  Debug(ssidConfig);
  Debug(", ");
  Debug(passwordConfig);
  Debug("\n");

  // 初始化 12引脚口
  pinMode(12, INPUT_PULLUP);

  // 启用WiFi模块
  WiFi.mode(WIFI_STA);
  // 连接WiFi
  WiFi.begin(ssidConfig, passwordConfig);

  int cishu = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (digitalRead(12) == 0)
    {
      Debug("\n");
      Debug("Wed服务");
      return Network_Wed;
    }
    ++cishu;
    if (cishu > 50)
    { // 几次后没法连接判定为没有网络
      Debug("\n");
      Debug("连接失败");
      return Network_Not;
    }
    DEV_Delay_ms(100);
    Debug(".");
  }
  Debug("\n");
  Debug("连接成功");
  return Network_Ok;
}

String WebServerFun()
{
  // 确保首先断开了STA模式下的任何连接
  WiFi.disconnect(true);

  // 设置ESP32为AP模式并启动，不使用密码
  WiFi.mode(WIFI_AP);
  WiFi.softAP("一言墨水屏");

  // 定义根路径的回调函数
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/wifi/config", HTTP_POST, handleWifiConfig); // 提交Wi-Fi信息进行连接
  server.on("/set", handleSet);
  server.on("/set/config", HTTP_POST, handleSetConfig);
  server.on("/restart", handleRestart);

  // 启动Web服务器
  server.begin();

  return WiFi.softAPIP().toString();
}

// 根路径请求的处理函数
void handleRoot()
{
  server.send(200, "text/html", RootHtml);
}

void handleSet()
{
  const String SethtmlForm = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 设置界面</title>
  <style>
    body{
      height: 100vh;
      margin: 0;
      font-family: 'SF Mono', 'Roboto Mono', monospace;
      display: flex;
      justify-content: center;
      align-items: center;
      background-color: #24292e; /* 暗色背景 */
      color: #c9d1d9; /* 浅色文字 */
    }

    .form-input {
      width: calc(100% - 20px);
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #30363d;
      border-radius: 6px;
      box-sizing: border-box;
      background-color: #161b22;
      color: #c9d1d9;
      transition: border-color 0.3s ease;
    }

    .form-input:focus {
      border-color: #58a6ff;
    }

    .submit-button {
      width: 100%;
      padding: 10px;
      margin-top: 10px;
      background-color: #2ea043;
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    .submit-button:hover {
      background-color: #279f42;
    }

    .form-container {
      padding: 20px;
      background-color: #161b22;
      border-radius: 6px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    }
  </style>

  <script>
    function logFun(){
      console.log("0k");
    }
  </script>

</head>
<body>
  <form action="/set/config" method="POST" id="passwordForm" class="form-container">
    <p>更新時間(分钟):</p>
    <input type="number" id="TimeVal" name="TimeVal" class="form-input" value="
  )rawliteral";
  const String SethtmlForm2 = R"rawliteral(
" oninput="logFun">
    <button type="submit" class="submit-button">修改</button>
  </form>
</body>
</html>
  )rawliteral";
  int shu;
  EEPROM.get(SleepValueAddr, shu);
  server.send(200, "text/html", SethtmlForm + String(shu) + SethtmlForm2);
}

void handleWifi()
{ // HTML表单，供用户输入Wi-Fi信息
  const String htmlForm1 = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>WiFi 配置界面</title>
  <style>
    /* 样式 */
    body, html {
      height: 100vh;
      margin: 0;
      font-family: 'SF Mono', 'Roboto Mono', monospace;
      display: flex;
      justify-content: center;
      align-items: center;
      background-color: #24292e; /* 暗色背景 */
      color: #c9d1d9; /* 浅色文字 */
    }

    .wifi-container {
      width: 100vh;
      max-width: 50vh;
      background-color: #161b22;
      border-radius: 6px;
      overflow: hidden;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    }

    .wifi-item {      
      padding: 5vh;
      width: 100%;
      transition: background-color 0.3s ease;
      cursor: pointer;
      border-bottom: 1px solid #30363d;
      text-align: center;
    }

    .wifi-item:hover {
      background-color: #21262d;
    }

    .hidden {
      display: none;
    }

    .form-input {
      width: calc(100% - 20px);
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #30363d;
      border-radius: 6px;
      box-sizing: border-box;
      background-color: #161b22;
      color: #c9d1d9;
      transition: border-color 0.3s ease;
    }

    .form-input:focus {
      border-color: #58a6ff;
    }

    .submit-button {
      width: 100%;
      padding: 10px;
      margin-top: 10px;
      background-color: #2ea043;
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    .submit-button:hover {
      background-color: #279f42;
    }

    .form-container {
      padding: 20px;
      background-color: #161b22;
      border-radius: 6px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    }

    label {
      display: block;
      margin-bottom: 5px;
      color: #8b949e;
    }
  </style>
</head>
<body>
  <div id="liebiao" class="wifi-container">
    )rawliteral";
  // 扫描附近WiFi
  int n = WiFi.scanNetworks();
  String WifiNameS = "";
  for (size_t i = 0; i < n; i++)
  {
    WifiNameS += "<div class=\"wifi-item\" onclick=\"showPasswordInput(this)\">" + WiFi.SSID(i) + "</div>";
  }
  const String htmlForm2 = R"rawliteral(
</div>
  <form action="/wifi/config" method="POST" id="passwordForm" class="form-container hidden">
    <label for="ssid">WiFi名称:</label>
    <input type="text" id="ssid" placeholder="请输入WiFi名称" name="ssid" class="form-input" required>
    <label for="password">WiFi密码:</label>
    <input type="password" id="password" placeholder="请输入WiFi密码" name="password" class="form-input" required>
    <button type="submit" class="submit-button">连接</button>
    <button type="button" class="submit-button" onclick="Fh()">返回</button>
  </form>
</body>
<script>
  function showPasswordInput(element) {
    var le = document.getElementById("liebiao");
    le.classList.add("hidden");
    var form = document.getElementById("passwordForm");
    form.classList.remove("hidden"); // 显示
    form.querySelector("#ssid").value = element.textContent;
  }

  function Fh() {
    var le = document.getElementById("liebiao");
    le.classList.remove("hidden");
    var form = document.getElementById("passwordForm");
    form.classList.add("hidden");
  }
</script>
</html>
    )rawliteral";
  Debug("提供Wed服务\n");
  server.send(200, "text/html", htmlForm1 + WifiNameS + htmlForm2);
}

// 处理WiFi配置提交
void handleWifiConfig()
{
  String ssidConfig = server.arg("ssid");
  String passwordConfig = server.arg("password");

  Debug(ssidConfig);
  Debug("\n");
  Debug(passwordConfig);

  writeStringToEEPROM(WifiNameAddr, ssidConfig);
  writeStringToEEPROM(WifiPassAddr, passwordConfig);

  // 读取字符串
  ssidConfig = readStringFromEEPROM(WifiNameAddr);
  passwordConfig = readStringFromEEPROM(WifiPassAddr);

  server.send(200, "text/html", RootHtml);

  // 调用esp_restart()函数进行重启
  // esp_restart();
}

// 处理WiFi配置提交
void handleSetConfig()
{
  String timeConfig = server.arg("TimeVal");

  Debug(timeConfig.toInt());
  Debug("\n");

  // 读取字符串
  int shu = timeConfig.toInt();
  EEPROM.put(SleepValueAddr, shu);
  EEPROM.get(SleepValueAddr, shu);
  EEPROM.commit();
  Debug(shu);
  server.send(200, "text/html", RootHtml);

  // 调用esp_restart()函数进行重启
  // esp_restart();
}

void handleRestart()
{
  // 提示用户已提交WiFi信息
  String response = "<h1>重启中...</h1>";
  server.send(200, "text/html", response);
  // 调用esp_restart() 函数进行重启
  esp_restart();
}

const char *RootHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="UTF-8">
<title>主页</title>
<style>
    /* 设置容器为弹性布局 */
    .container {
        display: flex;
        justify-content: space-around;
        align-items: center;
        height: 100vh;
        padding: 0px;
        background-color: #24292e;
    }

    /* 样式化SVG图标 */
    .icon {
        width: 30vw;
        height: 30vh;
        fill: currentColor; /* 使用当前颜色以支持主题变化 */
        transition: transform 0.2s ease-in-out;
    }

    /* 悬停时放大效果 */
    .icon:hover {
        transform: scale(1.1);
    }

    /* 默认浅色模式 */
    body {
        color: #000;
        background-color: #fff;
    }

    /* 深色模式 */
    @media (prefers-color-scheme: dark) {
        body {
            color: #fff;
            background-color: #000;
        }
        .icon {
            fill: #fff; /* 深色模式下使用白色填充 */
        }
    }
</style>
</head>
<body>
    <div class="container">
        <!-- WiFi图标 -->
        <a href="/wifi" target="_self">
            <svg t="1737446876153" class="icon" viewBox="0 0 1122 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="2433" width="200" height="200"><path d="M561.152 910.336c-7.68 0-25.088-13.824-53.248-41.984-27.648-28.16-41.472-45.568-41.472-53.248 0-12.288 11.776-22.528 35.84-30.72s43.52-12.8 59.392-12.8c15.872 0 35.328 4.096 59.392 12.8s35.84 18.432 35.84 30.72c0 7.68-13.824 25.6-41.472 53.248-29.184 28.16-46.592 41.984-54.272 41.984z" fill="#1296DB" p-id="2434"></path><path d="M715.264 755.712c-0.512 0-8.192-4.608-23.04-14.336-14.336-9.728-33.792-18.944-57.856-28.672-24.064-9.728-48.64-14.336-73.216-14.336-24.576 0-49.152 4.608-73.216 14.336-24.064 9.728-43.52 18.944-57.856 28.672-14.336 9.728-22.016 14.336-23.04 14.336-6.656 0-24.576-14.336-53.248-43.008s-43.008-46.08-43.008-53.248c0-5.12 2.048-9.216 5.632-13.312 29.696-29.184 67.072-52.224 112.128-69.12s89.088-25.088 133.12-25.088c44.032 0 88.064 8.192 133.12 25.088 45.056 16.896 82.432 39.936 112.128 69.12 3.584 3.584 5.632 8.192 5.632 13.312 0 6.656-14.336 24.576-43.008 53.248-29.696 28.672-47.104 43.008-54.272 43.008z" fill="#19A3E0" p-id="2435"></path><path d="M871.424 600.064c-4.096 0-8.704-1.536-13.312-4.608-51.712-39.936-99.84-69.632-143.872-88.064s-95.232-28.16-153.088-28.16c-32.256 0-65.024 4.096-97.28 12.8S402.944 510.464 378.88 522.24c-24.064 11.776-45.568 23.552-65.024 35.328s-34.304 22.016-45.056 30.208c-11.264 8.192-16.896 12.8-17.92 12.8-6.656 0-24.064-14.336-52.736-43.008s-43.008-46.08-43.008-53.248c0-4.608 2.048-8.704 5.632-12.8 50.176-50.176 111.104-89.088 182.784-117.248 71.68-27.648 143.872-41.472 217.088-41.472s145.408 13.824 217.088 41.472c71.68 27.648 132.608 67.072 182.784 117.248 3.584 3.584 5.632 8.192 5.632 12.8 0 6.656-14.336 24.576-43.008 53.248s-45.056 42.496-51.712 42.496z" fill="#48B5E5" p-id="2436"></path><path d="M1026.048 445.44c-4.096 0-8.192-1.536-12.8-5.12-68.096-59.904-138.752-104.96-212.48-135.168-73.216-30.208-153.6-45.568-240.128-45.568-87.04 0-166.912 15.36-240.128 45.568S176.64 380.416 108.544 440.32c-4.096 3.584-8.192 5.12-12.8 5.12-6.656 0-24.064-14.336-52.736-43.008S0 356.352 0 349.184c0-5.12 2.048-9.216 5.632-13.312C76.8 265.216 161.792 210.432 260.096 171.52s198.656-58.368 301.056-58.368 202.752 19.456 301.056 58.368 183.296 93.696 254.464 164.352c3.584 3.584 5.632 8.192 5.632 13.312 0 6.656-14.336 24.576-43.008 53.248-28.672 28.672-46.592 43.008-53.248 43.008z" fill="#80D8FF" p-id="2437"></path></svg>
        </a>

        <!-- 设置图标 -->
        <a href="/set" target="_self">
<svg t="1737633885234" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="7193" width="200" height="200"><path d="M951.68 428.8a23.808 23.808 0 0 0-0.64-4.224v-0.768l-0.448-2.24c-7.04-34.56-30.016-56.896-58.496-56.896h-4.672c-48.64 0-88-39.552-88-88 0-11.2 5.12-27.072 7.36-32.64 13.824-32.256-0.896-68.928-35.008-87.68l-107.264-60.672-1.984-0.64c-8.064-2.624-17.28-5.76-27.712-5.76-19.392 0-41.216 8.96-54.72 22.528-16.896 16.64-51.2 41.6-71.616 41.6-20.288 0-54.656-24.832-71.616-41.6a78.528 78.528 0 0 0-54.656-22.528c-10.688 0-19.712 3.008-27.712 5.76l-1.792 0.64-112.512 60.928-0.64 0.384c-27.328 17.088-38.4 56.32-24.576 87.424l0.192 0.384 0.256 0.384c2.176 4.928 8.96 21.504 8.96 36.032 0 48.64-39.616 88-88 88h-4.672c-29.824 0-52.096 21.952-58.496 57.28l-0.448 1.984v0.704c0 1.024-0.384 2.432-0.64 4.224-2.56 15.104-8.512 50.688-8.512 79.808 0 29.056 5.888 64.64 8.448 79.808a24 24 0 0 0 0.704 4.16v0.832l0.448 2.176c7.04 34.56 29.952 56.96 58.496 56.96h2.368c48.64 0 88 39.552 88 87.936 0 11.2-5.184 27.136-7.36 32.704-13.312 30.272-0.704 69.184 28.672 88.832l0.832 0.384 105.984 59.008 1.984 0.64c8 2.624 17.088 5.76 27.52 5.76 22.208 0 42.24-8.512 54.656-22.528 1.28-0.896 2.432-2.112 3.84-3.264 12.8-11.2 47.168-40.832 69.888-40.832 16.896 0 45.184 17.728 73.728 46.208 14.4 14.208 34.24 22.464 54.656 22.464 13.824 0 24-3.776 35.584-9.472l0.448-0.192 108.672-60.16 0.384-0.32c27.328-17.152 38.4-56.32 24.512-87.424l-0.192-0.384-0.192-0.384c-0.192-0.128-8.704-17.856-7.104-33.728l0.192-1.024v-0.96c0-48.64 39.616-88 88-88h4.992c29.824 0 52.096-22.016 58.496-57.344l0.448-1.92v-0.768l0.64-3.584c2.624-14.72 8.64-49.024 8.64-80.384 0.192-28.992-5.76-64.512-8.32-79.616z m-440 222.4a139.2 139.2 0 1 1 0-278.4 139.2 139.2 0 0 1 0 278.4z" fill="#00BAAD" p-id="7194"></path></svg>        </a>

        <!-- 重启图标 -->
        <a href="/restart" target="_self">
            <svg t="1737633375421" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" p-id="1499" width="200" height="200"><path d="M512 0C229.239467 0 0 229.239467 0 512c0 282.760533 229.239467 512 512 512 282.760533 0 512-229.239467 512-512C1024 229.239467 794.760533 0 512 0z m0 786.005333c-141.380267 0-256-114.619733-256-256 0-141.380267 114.619733-256 256-256V200.874667l146.295467 109.704533L512 420.317867v-73.130667a182.869333 182.869333 0 1 0 182.869333 182.818133c0-43.434667 73.130667-44.970667 73.130667 0 0 141.380267-114.619733 256-256 256z" fill="#3498DA" p-id="1500"></path></svg>        </a>
    </div>
    
</body>
</html>
)rawliteral";
