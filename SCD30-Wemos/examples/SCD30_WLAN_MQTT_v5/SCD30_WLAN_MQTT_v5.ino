#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h> //Nick O'Leary
#include <Wire.h>
#include <Adafruit_SCD30.h> //Adafruit
//wifi portal
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

//wifi portal
Preferences prefs;
WebServer server(80);
DNSServer dns;
String wifiSSID;
String wifiPassword;
String mqttServer;
int mqttPort;
String mqttUser;
String mqttPassword;
String mqttTopic;

//wifi accesspoint WPA2 salasana väh. 8 merkkiä
const char* AP_NAME = "SCD30-Setup";
const char* AP_PASSWORD = "asetukset";

//HTML-sivu
String htmlPage()
{
    Serial.println("htmlPage()");

    String page;

    page += "<!DOCTYPE html>";
    page += "<html>";
    page += "<head>";

    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";

    page += "<title>SCD30 Setup</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head>";

    page += "<body>";

    page += "<h2>SCD30 asetukset</h2>";

    page += "<form action='/save' method='POST'>";

    // WiFi
    page += "<h3>WiFi</h3>";

    page += "SSID:<br>";
    page += "<input name='ssid' value='";
    page += wifiSSID;
    page += "'><br><br>";

    page += "Password:<br>";
    page += "<input type='password' name='pass' value='";
    page += wifiPassword;
    page += "'><br><br>";

    // MQTT
    page += "<h3>MQTT</h3>";

    page += "Server:<br>";
    page += "<input name='mqttServer' value='";
    page += mqttServer;
    page += "'><br><br>";

    page += "Port:<br>";
    page += "<input type='number' name='mqttPort' value='";
    page += String(mqttPort);
    page += "'><br><br>";

    page += "Username:<br>";
    page += "<input name='mqttUser' value='";
    page += mqttUser;
    page += "'><br><br>";

    page += "Password:<br>";
    page += "<input type='password' name='mqttPass' value='";
    page += mqttPassword;
    page += "'><br><br>";

    page += "Topic:<br>";
    page += "<input name='mqttTopic' value='";
    page += mqttTopic;
    page += "'><br><br>";

    page += "<input type='submit' value='Save'>";

    page += "</form>";

 page += "<br>";

  page += "<button type='button' onclick='testMQTT()'>";
  page += "Testaa MQTT-yhteys";
  page += "</button>";

  page += "<script>";

  page += "function testMQTT() {";

  page += "const form = document.querySelector('form');";

  page += "const data = new URLSearchParams(new FormData(form));";

  page += "fetch('/mqtt-test', {";
  page += "method: 'POST',";
  page += "headers: {'Content-Type':'application/x-www-form-urlencoded'},";
  page += "body: data";
  page += "})";

  page += ".then(response => response.text())";
  page += ".then(html => {";
  page += "document.open();";
  page += "document.write(html);";
  page += "document.close();";
  page += "});";

  page += "}";

  page += "</script>";

    page += "<br>";
    page += "<a href='/'>Takaisin</a>";

    page += "</body>";
    page += "</html>";

    return page;
}

//HTML-sivun käsittelijä
void handleRoot()
{
  Serial.println("handleRoot()");

  String page;

  page += "<html><head>";
  page += "<meta charset='UTF-8'>";
  page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  page += "<title>SCD30</title>";
  page += "<link rel='stylesheet' href='/style.css'>";
  page += "</head><body>";

  page += "<h2>SCD30</h2>";

  page += "<p><b>WiFi:</b> OK</p>";

  page += "<p>SSID: ";
  page += WiFi.SSID();
  page += "</p>";

  page += "<p>IP: ";
  page += WiFi.localIP().toString();
  page += "</p>";

  page += "<p>RSSI: ";
  page += String(WiFi.RSSI());
  page += " dBm</p>";

  page += "<hr>";

  page += "<p><a href='/setup'>WiFi-asetukset</a></p>";

  page += "</body></html>";

  server.send(200, "text/html", page);
}

void handleSetup()
{
  Serial.println("handleSetup()");
  server.send(200, "text/html", htmlPage());
}

//handleMQTTTest
void handleMQTTTest()
{
    Serial.println("handleMQTTTest()");

    String testServer = server.arg("mqttServer");
    int testPort = server.arg("mqttPort").toInt();
    String testUser = server.arg("mqttUser");
    String testPassword = server.arg("mqttPass");

    Serial.println("MQTT-yhteyden testi");

    Serial.print("Server: ");
    Serial.println(testServer);

    Serial.print("Port: ");
    Serial.println(testPort);

    Serial.print("User: ");
    Serial.println(testUser);

    WiFiClientSecure testClient;
    testClient.setInsecure();

    PubSubClient testMQTT(testClient);

    testMQTT.setServer(testServer.c_str(), testPort);

    bool connected = testMQTT.connect(
        "ESP32-SCD30-TEST",
        testUser.c_str(),
        testPassword.c_str()
    );

    String page;

    page += "<!DOCTYPE html>";
    page += "<html>";
    page += "<head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<title>MQTT testi</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head>";
    page += "<body>";

    page += "<h2>MQTT-yhteystesti</h2>";

    if (connected)
    {
        Serial.println("MQTT testi: OK");

        page += "<p style='color:green;'>";
        page += "<b>✓ MQTT-yhteys onnistui!</b>";
        page += "</p>";

        testMQTT.disconnect();
    }
    else
    {
        Serial.print("MQTT testi epäonnistui. Virhe: ");
        Serial.println(testMQTT.state());

        page += "<p style='color:red;'>";
        page += "<b>✗ MQTT-yhteys epäonnistui</b>";
        page += "</p>";

        page += "<p>Virhekoodi: ";
        page += String(testMQTT.state());
        page += "</p>";

// MQTT virhekoodien taulukko

page += "<h3>MQTT virhekoodit</h3>";

page += "<table border='1' cellpadding='6' cellspacing='0'>";

page += "<tr>";
page += "<th>Koodi</th>";
page += "<th>Merkitys</th>";
page += "</tr>";

page += "<tr>";
page += "<td>0</td>";
page += "<td>Yhteys onnistui</td>";
page += "</tr>";

page += "<tr>";
page += "<td>-1</td>";
page += "<td>Yhteys katkaistu</td>";
page += "</tr>";

page += "<tr>";
page += "<td>-2</td>";
page += "<td>MQTT-yhteyden muodostaminen epäonnistui</td>";
page += "</tr>";

page += "<tr>";
page += "<td>-3</td>";
page += "<td>Yhteys katkaistiin</td>";
page += "</tr>";

page += "<tr>";
page += "<td>-4</td>";
page += "<td>Yhteyden aikakatkaisu</td>";
page += "</tr>";

page += "<tr>";
page += "<td>-5</td>";
page += "<td>Virheellinen käyttäjätunnus tai salasana</td>";
page += "</tr>";

page += "<tr>";
page += "<td>-6</td>";
page += "<td>Virheellinen MQTT-protokolla</td>";
page += "</tr>";

page += "</table>";

    }

    page += "<p><a href='/setup'>Takaisin asetuksiin</a></p>";

    page += "</body>";
    page += "</html>";

    server.send(200, "text/html", page);
}

void handleSave()
{
    Serial.println("handleSave()");

    prefs.begin("config", false);

    // WiFi
    prefs.putString("ssid", server.arg("ssid"));
    prefs.putString("pass", server.arg("pass"));

    // MQTT
    prefs.putString("mqttServer", server.arg("mqttServer"));
    prefs.putInt("mqttPort", server.arg("mqttPort").toInt());
    prefs.putString("mqttUser", server.arg("mqttUser"));
    prefs.putString("mqttPass", server.arg("mqttPass"));
    prefs.putString("mqttTopic", server.arg("mqttTopic"));

    Serial.println("Asetukset tallennettu");

    Serial.print("WiFi SSID: ");
    Serial.println(server.arg("ssid"));

    Serial.print("MQTT Server: ");
    Serial.println(server.arg("mqttServer"));

    Serial.print("MQTT Port: ");
    Serial.println(server.arg("mqttPort"));

    Serial.print("MQTT User: ");
    Serial.println(server.arg("mqttUser"));

    Serial.print("MQTT Topic: ");
    Serial.println(server.arg("mqttTopic"));

    prefs.end();

    server.send(
        200,
        "text/html",
        "<html><body>"
        "<h2>Asetukset tallennettu!</h2>"
        "<p>ESP32 käynnistyy uudelleen...</p>"
        "</body></html>"
    );

    delay(1500);

    ESP.restart();
}

void handleStyle()
{
    String css;

    css += "body {";
    css += "font-family: Arial, Helvetica, sans-serif;";
    css += "font-size: 16px;";
    css += "line-height: 1.5;";
    css += "margin: 20px;";
    css += "color: #222;";
    css += "}";

    css += "h1, h2, h3 {";
    css += "font-family: Arial, Helvetica, sans-serif;";
    css += "}";

    css += "input, button, select, textarea {";
    css += "font-family: Arial, Helvetica, sans-serif;";
    css += "font-size: 16px;";
    css += "}";

    css += "input[type='text'],";
    css += "input[type='password'],";
    css += "input[type='number'] {";
    css += "width: 100%;";
    css += "max-width: 450px;";
    css += "box-sizing: border-box;";
    css += "padding: 8px;";
    css += "margin-top: 4px;";
    css += "}";

    css += "button,";
    css += "input[type='submit'] {";
    css += "padding: 8px 14px;";
    css += "margin-top: 8px;";
    css += "cursor: pointer;";
    css += "}";

    css += "table {";
    css += "font-family: Arial, Helvetica, sans-serif;";
    css += "font-size: 15px;";
    css += "border-collapse: collapse;";
    css += "margin-top: 10px;";
    css += "}";

    css += "th, td {";
    css += "border: 1px solid #999;";
    css += "padding: 6px 10px;";
    css += "text-align: left;";
    css += "}";

    css += "th {";
    css += "font-weight: bold;";
    css += "}";

    css += "a {";
    css += "font-family: Arial, Helvetica, sans-serif;";
    css += "}";

    server.send(200, "text/css", css);
}

void startPortal()
{
    Serial.println("startPortal()");

    WiFi.disconnect(true);
    delay(500);

    WiFi.mode(WIFI_AP_STA);

    WiFi.softAP(AP_NAME, AP_PASSWORD);

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    dns.start(53, "*", WiFi.softAPIP());

    server.on("/", handleRoot);

    server.on("/setup", handleSetup);

    server.on("/save", HTTP_POST, handleSave);

    server.on("/mqtt-test", HTTP_POST, handleMQTTTest);

    server.on("/style.css", HTTP_GET, handleStyle);

    server.onNotFound([]()
    {
        Serial.println("NotFound");

        server.send(
            200,
            "text/html",
            htmlPage()
        );
    });

    Serial.println("server.begin()");

    server.begin();

    Serial.println("server valmis");
}

void startWebServer()
{
    Serial.println("startWebServer()");

    server.on("/", handleRoot);

    server.on("/setup", handleSetup);

    server.on("/save", HTTP_POST, handleSave);

    server.on("/mqtt-test", HTTP_POST, handleMQTTTest);

    server.on("/style.css", HTTP_GET, handleStyle);

    server.onNotFound([]()
    {
        Serial.println("NotFound");
        server.send(404, "text/plain", "404 - Not Found");
    });

    server.begin();

    Serial.println("WebServer valmis");
}

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

// ---------- SCD30 ----------
Adafruit_SCD30 scd30;

unsigned long lastPublish = 0;
const unsigned long publishInterval = 10000;   // 10 s

unsigned long lastMQTTAttempt = 0;
const unsigned long mqttRetryInterval = 10000;


//--------------------------------------------------

//wifi Asetusportaali
bool loadSettings()
{
    Serial.println("loadSettings()");

    prefs.begin("config", true);

    wifiSSID = prefs.getString("ssid", "");
    wifiPassword = prefs.getString("pass", "");

    mqttServer = prefs.getString("mqttServer", "");
    mqttPort = prefs.getInt("mqttPort", 8883);
    mqttUser = prefs.getString("mqttUser", "");
    mqttPassword = prefs.getString("mqttPass", "");
    mqttTopic = prefs.getString("mqttTopic", "scd30/data");

    prefs.end();

    Serial.println();
    Serial.println("Tallennetut asetukset");

    Serial.print("WiFi SSID: ");
    Serial.println(wifiSSID);

    Serial.print("MQTT Server: ");
    Serial.println(mqttServer);

    Serial.print("MQTT Port: ");
    Serial.println(mqttPort);

    Serial.print("MQTT User: ");
    Serial.println(mqttUser);

    Serial.print("MQTT Topic: ");
    Serial.println(mqttTopic);

    if (wifiSSID.length() == 0)
        return false;

    if (mqttServer.length() == 0)
        return false;

    return true;
}

//wifi Asetusportaali
bool connectWiFi()
{
    Serial.println("connectWiFi()");
    delay(250);
    WiFi.mode(WIFI_STA);
    delay(250);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    delay(250);
    Serial.print("Yhdistetään WiFiin");

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");

        if (millis() - start > 20000)
        {
            Serial.println();
            Serial.println("Aikakatkaisu");

            return false;
        }
    }

    Serial.println();
    Serial.println("WiFi OK");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("GW: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("RSSI: ");
    Serial.println(WiFi.RSSI());


    return true;
}


//--------------------------------------------------
bool connectMQTT()
{
    Serial.print("Yhdistetään MQTT... ");

    if (client.connected())
    {
        Serial.println("jo yhdistetty");
        return true;
    }

    if (client.connect("ESP32-SCD30",
                       mqttUser.c_str(),
                       mqttPassword.c_str()))
    {
        Serial.println("OK");
        return true;
    }

    Serial.print("Virhe ");
    Serial.println(client.state());

    return false;
}

//--------------------------------------------------

void setup() {

  Serial.begin(115200);

  Wire.begin(21,22);
  Wire.setClock(50000);

  if (!scd30.begin()) {

    Serial.println("SCD30 ei löytynyt!");

    while (1);
  }

  Serial.println("SCD30 OK");

  //korvataan tämä hetkeksi
  //connectWiFi();
  //wifi Asetusportaali
  if (!loadSettings())
    {
        Serial.println("Ei asetuksia.");

        startPortal();

        while (true)
        {
            dns.processNextRequest();
            server.handleClient();
            delay(1);
        }
    }

    if (!connectWiFi())
    {
        Serial.println("WiFi epäonnistui.");

        startPortal();

        while (true)
        {
            dns.processNextRequest();
            server.handleClient();
            delay(1);
        }
    }

  startWebServer();

  wifiClient.setInsecure();      // Testikäyttöön

  client.setServer(mqttServer.c_str(), mqttPort);

  connectMQTT();
}

//--------------------------------------------------
void loop()
{
    // --------------------------------
    // Web-palvelin
    // --------------------------------

    server.handleClient();


    // --------------------------------
    // WiFi
    // --------------------------------

    if (WiFi.status() != WL_CONNECTED)
    {
        if (!connectWiFi())
        {
            startPortal();

            while (true)
            {
                dns.processNextRequest();
                server.handleClient();
                delay(1);
            }
        }
    }


    // --------------------------------
    // MQTT
    // --------------------------------

    if (!client.connected())
    {
        if (millis() - lastMQTTAttempt >= mqttRetryInterval)
        {
            lastMQTTAttempt = millis();

            connectMQTT();
        }
    }

    client.loop();


    // --------------------------------
    // SCD30
    // --------------------------------

    if (millis() - lastPublish >= publishInterval)
    {
        lastPublish = millis();

        if (scd30.dataReady())
        {
            if (scd30.read())
            {
                float co2 = scd30.CO2;
                float temp = scd30.temperature;
                float hum = scd30.relative_humidity;

                Serial.println("--------------------");

                Serial.print("CO2 : ");
                Serial.print(co2);
                Serial.println(" ppm");

                Serial.print("Temp: ");
                Serial.print(temp);
                Serial.println(" C");

                Serial.print("RH  : ");
                Serial.print(hum);
                Serial.println(" %");


                // ----------------------------
                // MQTT JSON
                // ----------------------------

                char payload[128];

                snprintf(
                    payload,
                    sizeof(payload),
                    "{\"co2\":%.0f,\"temperature\":%.2f,\"humidity\":%.2f}",
                    co2,
                    temp,
                    hum
                );


                // ----------------------------
                // MQTT publish
                // ----------------------------

                if (client.connected())
                {
                    client.publish(
                        mqttTopic.c_str(),
                        payload
                    );

                    Serial.println("MQTT lähetetty:");
                    Serial.println(payload);
                }
                else
                {
                    Serial.println("MQTT ei ole yhteydessä - viestiä ei lähetetty.");
                }
            }
        }
    }
}