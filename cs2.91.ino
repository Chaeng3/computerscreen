#include <WiFi.h>
#include <HTTPClient.h>
#include <U8g2lib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

// WiFi 连接信息
const char *ssid = "XXX"; // WIFI名称
const char *password = "XXX"; // WIFI密码
const char *host = "XXX";//aida64API网址

// NTP 客户端设置
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 28800, 3600000); // 更新间隔设置为1小时（3600000毫秒）

int16_t cpu_usage;
int16_t ram_usage;
int16_t gpu_usage;
String date;
String current_time;

String previous_times[4]; // 用于存储最近四次获取的时间
int time_index = 0;       // 当前时间索引

bool clockMode = false;
unsigned long clockModeStartTime = 0;
unsigned long lastResponseTime = 0; // 上次接收到服务器响应的时间
unsigned long lastUpdateTime = 0;   // 上次更新时间的时间

const unsigned long timeoutDuration = 10000; // 10秒超时

// OLED 显示初始化
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/SCL, /* data=*/SDA);

// 本地时间变量
unsigned long currentEpochTime = 0;       // 当前的Unix时间戳（秒）
unsigned long previousMillis = 0;         // 用于跟踪上次增加秒的时间
const unsigned long interval = 1000;      // 1秒的间隔
unsigned long lastNtpSyncTime = 0;        // 上次与NTP同步的时间
const unsigned long ntpSyncInterval = 3600000; // 1小时的间隔

void setup()
{
    Serial.begin(115200);
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    u8g2.begin();          // 初始化OLED屏幕
    u8g2.setFlipMode(1);   // 旋转屏幕180度
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    // 初始化NTP客户端
    timeClient.begin();
    syncTimeWithNTP();     // 初始时间同步
    lastNtpSyncTime = millis();
    lastUpdateTime = millis(); // 初始化上次更新时间
}

void loop()
{
    unsigned long currentMillis = millis();

    // 每隔1小时同步一次NTP时间
    if (currentMillis - lastNtpSyncTime >= ntpSyncInterval)
    {
        syncTimeWithNTP();
        lastNtpSyncTime = currentMillis;
    }

    // 每隔1秒增加一次本地时间
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        currentEpochTime++;
    }

    // 检查串口命令
    if (Serial.available() > 0)
    {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command.equalsIgnoreCase("aclock"))
        {
            enterClockMode(); // 条件1：通过串口命令进入时钟模式
        }
    }

    // 检查是否需要进入时钟模式
    if (clockMode)
    {
        // 显示电子时钟，每秒更新一次
        if (currentMillis - lastUpdateTime >= 1000)
        {
            displayClock();
            lastUpdateTime = currentMillis; // 更新上次更新时间
        }

        // 检查是否已超过1分钟（仅适用于通过串口命令进入时钟模式）
        if (currentMillis - clockModeStartTime > 60000 && clockModeStartTime != 0)
        {
            clockMode = false;
            Serial.println("Exiting clock mode.");
        }

        return;
    }

    WiFiClient client;

    if (client.connect(host, 80))
    {
        String getUrl = "/sse";
        client.print(String("GET ") + getUrl + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        Serial.println("GET request sent");

        delay(100); // 增加延迟以等待服务器响应

        String line;
        bool hasContent = false;
        while (client.available())
        {
            line += client.readStringUntil('\n');
            hasContent = true;
        }

        if (hasContent && line.length() > 0)
        {
            lastResponseTime = millis(); // 更新上次接收到响应的时间
            Serial.println("Content:");
            Serial.println(line);

            // 解析日期
            parseAndPrintString(line, "Simple1|date ", date);
            // 解析时间
            String new_time;
            parseAndPrintString(line, "Simple2|time ", new_time);

            // 检查是否连续四次时间相同
            previous_times[time_index] = new_time;
            time_index = (time_index + 1) % 4;

            if (previous_times[0] == previous_times[1] && previous_times[1] == previous_times[2] && previous_times[2] == previous_times[3] && !previous_times[0].isEmpty())
            {
                Serial.println("Time is the same for four consecutive attempts, switching to clock mode.");
                enterClockMode(); // 条件2：服务器没有新内容时进入时钟模式
            }

            current_time = new_time;
            // 解析CPU使用率
            parseAndPrintUsage(line, "Simple3|CPU usage ", cpu_usage);
            // 解析内存使用率
            parseAndPrintUsage(line, "Simple4|MEM usage ", ram_usage);
            // 解析GPU使用率
            parseAndPrintUsage(line, "Simple5|GPU usage ", gpu_usage);

            // 显示数据在OLED屏幕上
            displayData();
        }
        else
        {
            Serial.println("No new content received from server, switching to clock mode.");
            enterClockMode(); // 条件2：服务器没有新内容时进入时钟模式
        }

        client.stop(); // 关闭连接
    }
    else
    {
        Serial.println("Failed to connect to host, switching to clock mode.");
        enterClockMode(); // 条件3：服务器连接失败时进入时钟模式
    }

    // 检查是否已超时
    if (millis() - lastResponseTime > timeoutDuration)
    {
        Serial.println("Response timeout, switching to clock mode.");
        enterClockMode(); // 条件4：响应超时时进入时钟模式
    }

    delay(500); // 确保不会太频繁地执行 loop()
}

void syncTimeWithNTP()
{
    if (timeClient.update())
    {
        currentEpochTime = timeClient.getEpochTime();
        Serial.print("NTP Time synchronized: ");
        Serial.println(currentEpochTime);
    }
    else
    {
        Serial.println("Failed to synchronize time with NTP server.");
    }
}

void enterClockMode()
{
    clockMode = true;
    clockModeStartTime = millis();
    lastUpdateTime = 0; // 重置上次更新时间
    Serial.println("Entering clock mode.");
}

void parseAndPrintUsage(const String &line, const char *label, int16_t &usage)
{
    int startIndex = line.indexOf(label) + strlen(label);
    int endIndex = line.indexOf("%", startIndex);
    if (startIndex > 0 && endIndex > startIndex)
    {
        String usageStr = line.substring(startIndex, endIndex);
        usage = usageStr.toInt();
    }

    Serial.print(label);
    Serial.print(": ");
    Serial.println(usage);
}

void parseAndPrintString(const String &line, const char *label, String &output)
{
    int startIndex = line.indexOf(label) + strlen(label);
    int endIndex = line.indexOf("{|}", startIndex);
    if (startIndex > 0 && endIndex > startIndex)
    {
        output = line.substring(startIndex, endIndex);
    }

    Serial.print(label);
    Serial.print(": ");
    Serial.println(output);
}

void drawHalfCircleBar(int x, int y, int radius, int percentage, const char *label)
{
    int angle = map(percentage, 0, 100, 0, 180);

    // 清除之前的绘制
    u8g2.setDrawColor(0); // 设置绘制颜色为黑色，覆盖之前绘制的部分
    u8g2.drawBox(x - radius - 10, y - radius - 10, 2 * radius + 20, radius + 20);
    u8g2.setDrawColor(1); // 设置绘制颜色为白色

    // 绘制外圈半圆刻度
    for (int i = 0; i <= 180; i += 10)
    {
        int x0 = x + radius * cos((i - 180) * PI / 180); // 逆时针旋转90度
        int y0 = y + radius * sin((i - 180) * PI / 180);
        int x1 = x + (radius - 3) * cos((i - 180) * PI / 180);
        int y1 = y + (radius - 3) * sin((i - 180) * PI / 180);
        u8g2.drawLine(x0, y0, x1, y1);
    }

    // 绘制指针
    int x0 = x + radius * cos((angle - 180) * PI / 180); // 逆时针旋转90度
    int y0 = y + radius * sin((angle - 180) * PI / 180);
    u8g2.drawLine(x, y, x0, y0);

    // 在中间显示文字，再往下移动6个像素
    u8g2.setFont(u8g2_font_5x7_tr); // 更小的字体
    int text_width = u8g2.getStrWidth(label);
    u8g2.setCursor(x - text_width / 2, y + 10); // 向下移动10像素
    u8g2.print(label);

    // 在标签文字下方显示百分比
    String percentageStr = String(percentage) + "%";
    text_width = u8g2.getStrWidth(percentageStr.c_str());
    u8g2.setCursor(x - text_width / 2, y + 19); // 向下移动19像素
    u8g2.print(percentageStr);
}

void displayData()
{
    u8g2.clearBuffer(); // 清除缓冲区

    // 显示日期和时间（不显示秒数，只显示小时和分钟）
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(5, 10);
    u8g2.print(date);

    if (!current_time.isEmpty())
    {
        int colonIndex = current_time.indexOf(':');
        if (colonIndex != -1)
        {
            String timeWithoutSeconds = current_time.substring(0, colonIndex + 3); // 提取小时和分钟
            u8g2.setCursor(100, 10);
            u8g2.print(timeWithoutSeconds);
        }
    }

    // 显示CPU使用率图形
    drawHalfCircleBar(25, 38, 15, cpu_usage, "CPU");

    // 显示内存使用率图形
    drawHalfCircleBar(65, 38, 15, ram_usage, "RAM");

    // 显示GPU使用率图形
    drawHalfCircleBar(105, 38, 15, gpu_usage, "GPU");

    u8g2.sendBuffer(); // 更新显示
}

void displayClock()
{
    u8g2.clearBuffer(); // 清除缓冲区

    // 使用本地维护的时间变量
    unsigned long epochTime = currentEpochTime;

    // 使用 time.h 库解析时间戳
    time_t rawTime = epochTime;
    struct tm timeInfo;
    localtime_r(&rawTime, &timeInfo); // 使用 localtime_r 确保线程安全

    int monthDay = timeInfo.tm_mday;
    int currentMonth = timeInfo.tm_mon + 1;
    int currentYear = timeInfo.tm_year + 1900;
    int currentHour = timeInfo.tm_hour;
    int currentMinute = timeInfo.tm_min;
    int currentSecond = timeInfo.tm_sec;
    int currentWeekDay = timeInfo.tm_wday;

    const char *weekDays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    const char *weekDay = weekDays[currentWeekDay];

    char dateBuffer[20];
    sprintf(dateBuffer, "%04d/%02d/%02d", currentYear, currentMonth, monthDay);

    char timeBuffer[10];
    sprintf(timeBuffer, "%02d:%02d:%02d", currentHour, currentMinute, currentSecond);

    // 显示日期
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(0, 10);
    u8g2.print(dateBuffer);

    // 显示星期
    u8g2.setCursor(100, 10);
    u8g2.print(weekDay);

    // 显示时间，使用更大的字体
    u8g2.setFont(u8g2_font_ncenB24_tr); // 更大的字体
    u8g2.setCursor(-3, 50);             // 调整位置
    u8g2.print(timeBuffer);

    u8g2.sendBuffer(); // 更新显示
}
