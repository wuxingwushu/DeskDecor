#include "Network.h"
#include "EepromString.h"
#include "DEV_Config.h"
#include "Debug.h"
#include <WiFi.h>

extern WebServer server;

// 根路径请求的处理函数
void handleRoot()
{ // HTML表单，供用户输入Wi-Fi信息
    const String htmlForm1 = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>WiFi 配置界面</title>
  <style>
    /* 样式 */
    body, html {
      height: 100%;
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
      max-width: 500px;
      background-color: #161b22;
      border-radius: 6px;
      overflow: hidden;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    }

    .wifi-item {
      width: 100%;
      padding: 15px;
      transition: background-color 0.3s ease;
      cursor: pointer;
      border-bottom: 1px solid #a4c8f0;
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
  <form action="/config" method="POST" id="passwordForm" class="form-container hidden">
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
    server.send(200, "text/html", htmlForm1 + WifiNameS + htmlForm2);
}

// 处理WiFi配置提交
void handleConfig()
{
    String ssidConfig = server.arg("ssid");
    String passwordConfig = server.arg("password");

    Debug(ssidConfig);
    Debug("\n");
    Debug(passwordConfig);

    writeStringToEEPROM(4, ssidConfig);
    writeStringToEEPROM(400, passwordConfig);

    // 读取字符串
    ssidConfig = readStringFromEEPROM(4);
    passwordConfig = readStringFromEEPROM(400);

    // 提示用户已提交WiFi信息
    String response = "<h1>正在连接到WiFi网络...</h1>";
    response += "<p>WiFi名称: " + ssidConfig + "</p>";

    server.send(200, "text/html", response);

    // 调用esp_restart()函数进行重启
    esp_restart();
}















NetworkCase ConnectWIFI()
{
    // 读取字符串
    String ssidConfig = readStringFromEEPROM(4);
    String passwordConfig = readStringFromEEPROM(400);

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

void WedServerFun()
{
    // 确保首先断开了STA模式下的任何连接
    WiFi.disconnect(true);

    // 设置ESP32为AP模式并启动，不使用密码
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32-001");
    String StringAPIP = WiFi.softAPIP().toString();
    StringAPIP += String('\0');
    Debug(StringAPIP);


    return;
}

void NetworkhandleClient(){
  server.handleClient();
}

