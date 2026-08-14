#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SCD30.h>
#include "esp_system.h"

#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

// ==================================================
// SCD30 / WiFi / MQTT
// ==================================================

Adafruit_SCD30 scd30;

Preferences prefs;
WebServer server(80);
DNSServer dns;

String wifiSSID;
String wifiPassword;
String mqttServer;
int mqttPort = 8883;
String mqttUser;
String mqttPassword;
String mqttTopic = "scd30/data";

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

// ==================================================
// WiFi setup portal
// ==================================================

bool portalMode = false;
String portalToken;
String apName;
String apPassword;

// ==================================================
// Timing
// ==================================================

uint32_t bootCount = 0;

unsigned long lastPublish = 0;
const unsigned long publishInterval = 10000;

unsigned long lastMQTTAttempt = 0;
const unsigned long mqttRetryInterval = 10000;

// ==================================================
// Let's Encrypt / ISRG Root X1
// ==================================================
//
// V6 verifies the MQTT server certificate.
// This CA is intended for a Let's Encrypt certificate chain.
// If you use another broker / CA, replace this certificate.
//
// Official source:
// https://letsencrypt.org/certificates/
// ==================================================

const char* ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// ==================================================
// Helpers
// ==================================================

void sendSecurityHeaders()
{
    server.sendHeader("Cache-Control", "no-store");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("X-Content-Type-Options", "nosniff");
    server.sendHeader("X-Frame-Options", "DENY");
    server.sendHeader("Referrer-Policy", "no-referrer");
    server.sendHeader(
        "Content-Security-Policy",
        "default-src 'self'; style-src 'self'; script-src 'none'; object-src 'none'; base-uri 'none'; frame-ancestors 'none'"
    );
}

void sendHtml(int code, const String& page)
{
    sendSecurityHeaders();
    server.send(code, "text/html; charset=UTF-8", page);
}

String htmlEscape(const String& input)
{
    String output;
    output.reserve(input.length() + 16);

    for (size_t i = 0; i < input.length(); i++)
    {
        char c = input[i];

        switch (c)
        {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\'': output += "&#39;"; break;
            default: output += c; break;
        }
    }

    return output;
}

bool validLength(const String& value, size_t maxLength)
{
    return value.length() <= maxLength;
}

bool validPort(int port)
{
    return port >= 1 && port <= 65535;
}

String makeRandomHex(uint32_t value)
{
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%08lX", (unsigned long)value);
    return String(buffer);
}

String makePortalToken()
{
    return makeRandomHex(esp_random()) + makeRandomHex(esp_random());
}

String makeRandomAPPassword()
{
    // 16 hexadecimal characters = 64 bits of random data.
    return makeRandomHex(esp_random()) + makeRandomHex(esp_random());
}

bool tokenOK()
{
    return portalMode &&
           portalToken.length() > 0 &&
           server.hasArg("token") &&
           server.arg("token") == portalToken;
}

// ==================================================
// HTML
// ==================================================

String htmlPage()
{
    String page;

    page += "<!DOCTYPE html>";
    page += "<html><head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<meta name='robots' content='noindex,nofollow'>";
    page += "<title>SCD30 Setup</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head><body>";

    page += "<h2>SCD30 asetukset</h2>";

    page += "<form action='/save' method='POST'>";

    page += "<input type='hidden' name='token' value='";
    page += htmlEscape(portalToken);
    page += "'>";

    page += "<h3>WiFi</h3>";

    page += "SSID:<br>";
    page += "<input name='ssid' maxlength='64' value='";
    page += htmlEscape(wifiSSID);
    page += "'><br><br>";

    page += "Password:<br>";
    page += "<input type='password' name='pass' maxlength='128' value=''";
    page += " placeholder='Jätä tyhjäksi, jos salasanaa ei muuteta'>";
    page += "<br><br>";

    page += "<h3>MQTT</h3>";

    page += "Server:<br>";
    page += "<input name='mqttServer' maxlength='128' value='";
    page += htmlEscape(mqttServer);
    page += "'><br><br>";

    page += "Port:<br>";
    page += "<input type='number' name='mqttPort' min='1' max='65535' value='";
    page += String(mqttPort);
    page += "'><br><br>";

    page += "Username:<br>";
    page += "<input name='mqttUser' maxlength='128' value='";
    page += htmlEscape(mqttUser);
    page += "'><br><br>";

    page += "Password:<br>";
    page += "<input type='password' name='mqttPass' maxlength='128' value=''";
    page += " placeholder='Jätä tyhjäksi, jos salasanaa ei muuteta'>";
    page += "<br><br>";

    page += "Topic:<br>";
    page += "<input name='mqttTopic' maxlength='128' value='";
    page += htmlEscape(mqttTopic);
    page += "'><br><br>";

    page += "<input type='submit' value='Tallenna asetukset'>";

    page += "<button type='submit' formaction='/mqtt-test' formmethod='POST'>";
    page += "Testaa MQTT-yhteys";
    page += "</button>";

    page += "</form>";

    page += "<br><p><a href='/'>Takaisin</a></p>";

    page += "</body></html>";

    return page;
}

String errorPage(const String& message)
{
    String page;

    page += "<!DOCTYPE html><html><head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<title>SCD30</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head><body>";

    page += "<h2>Virhe</h2>";
    page += "<p>";
    page += htmlEscape(message);
    page += "</p>";
    page += "<p><a href='/setup'>Takaisin asetuksiin</a></p>";

    page += "</body></html>";

    return page;
}

// ==================================================
// Root page
// ==================================================

void handleRoot()
{
    String page;

    page += "<!DOCTYPE html><html><head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<meta name='robots' content='noindex,nofollow'>";
    page += "<title>SCD30</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head><body>";

    page += "<h2>SCD30</h2>";

    if (portalMode)
    {
        page += "<p><b>WiFi-asetustila</b></p>";
        page += "<p>AP: ";
        page += htmlEscape(apName);
        page += "</p>";

        page += "<p><a href='/setup'>WiFi- ja MQTT-asetukset</a></p>";
    }
    else
    {
        page += "<p><b>WiFi:</b> OK</p>";

        page += "<p>SSID: ";
        page += htmlEscape(WiFi.SSID());
        page += "</p>";

        page += "<p>IP: ";
        page += htmlEscape(WiFi.localIP().toString());
        page += "</p>";

        page += "<p>RSSI: ";
        page += String(WiFi.RSSI());
        page += " dBm</p>";
    }

    page += "<p><a href='/board-info'>Device Information</a></p>";

    page += "</body></html>";

    sendHtml(200, page);
}

// ==================================================
// Setup page
// ==================================================

void handleSetup()
{
    if (!portalMode)
    {
        sendHtml(403, errorPage("Asetusportaali ei ole käytössä normaalissa WiFi-tilassa."));
        return;
    }

    sendHtml(200, htmlPage());
}

// ==================================================
// Device information
// ==================================================

String getResetReason()
{
    switch (esp_reset_reason())
    {
        case ESP_RST_UNKNOWN:    return "Unknown";
        case ESP_RST_POWERON:    return "Power-on";
        case ESP_RST_EXT:        return "External reset";
        case ESP_RST_SW:         return "Software reset";
        case ESP_RST_PANIC:      return "Panic / crash";
        case ESP_RST_INT_WDT:    return "Interrupt watchdog";
        case ESP_RST_TASK_WDT:   return "Task watchdog";
        case ESP_RST_WDT:        return "Watchdog";
        case ESP_RST_DEEPSLEEP:  return "Deep sleep";
        case ESP_RST_BROWNOUT:   return "Brownout";
        case ESP_RST_SDIO:       return "SDIO reset";
        default:                 return "Other";
    }
}

void handleBoardInfo()
{
    String page;

    page += "<!DOCTYPE html><html><head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<meta name='robots' content='noindex,nofollow'>";
    page += "<title>Device Information</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head><body>";

    page += "<h2>Device Information</h2>";

    page += "<h3>ESP32</h3><table>";

    page += "<tr><th>Chip</th><td>";
    page += htmlEscape(ESP.getChipModel());
    page += "</td></tr>";

    page += "<tr><th>Chip revision</th><td>";
    page += String(ESP.getChipRevision());
    page += "</td></tr>";

    page += "<tr><th>CPU cores</th><td>";
    page += String(ESP.getChipCores());
    page += "</td></tr>";

    page += "<tr><th>CPU frequency</th><td>";
    page += String(ESP.getCpuFreqMHz());
    page += " MHz</td></tr>";

    page += "<tr><th>Flash size</th><td>";
    page += String(ESP.getFlashChipSize() / (1024 * 1024));
    page += " MB</td></tr>";

    page += "<tr><th>Free heap</th><td>";
    page += String(ESP.getFreeHeap() / 1024);
    page += " KB</td></tr>";

    page += "<tr><th>Minimum free heap</th><td>";
    page += String(ESP.getMinFreeHeap() / 1024);
    page += " KB</td></tr>";

    page += "<tr><th>Sketch size</th><td>";
    page += String(ESP.getSketchSize() / 1024);
    page += " KB</td></tr>";

    page += "<tr><th>Free sketch space</th><td>";
    page += String(ESP.getFreeSketchSpace() / 1024);
    page += " KB</td></tr>";

    page += "</table>";

    page += "<h3>Software</h3><table>";

    page += "<tr><th>Arduino Core</th><td>";
    page += htmlEscape(ESP.getCoreVersion());
    page += "</td></tr>";

    page += "<tr><th>ESP-IDF</th><td>";
    page += htmlEscape(ESP.getSdkVersion());
    page += "</td></tr>";

    page += "</table>";

    page += "<h3>Network</h3><table>";

    page += "<tr><th>MAC address</th><td>";
    page += htmlEscape(WiFi.macAddress());
    page += "</td></tr>";

    page += "<tr><th>IP address</th><td>";
    page += htmlEscape(WiFi.localIP().toString());
    page += "</td></tr>";

    page += "<tr><th>Gateway</th><td>";
    page += htmlEscape(WiFi.gatewayIP().toString());
    page += "</td></tr>";

    page += "<tr><th>DNS</th><td>";
    page += htmlEscape(WiFi.dnsIP().toString());
    page += "</td></tr>";

    page += "<tr><th>RSSI</th><td>";
    page += String(WiFi.RSSI());
    page += " dBm</td></tr>";

    page += "<tr><th>Boot count</th><td>";
    page += String(bootCount);
    page += "</td></tr>";

    page += "<tr><th>Reset reason</th><td>";
    page += htmlEscape(getResetReason());
    page += " (";
    page += String((int)esp_reset_reason());
    page += ")</td></tr>";

    page += "<tr><th>Uptime</th><td>";
    page += String(millis() / 1000);
    page += " s</td></tr>";

    page += "</table>";

    page += "<h3>SCD30</h3><table>";
    page += "<tr><th>I²C address</th><td>0x61</td></tr>";
    page += "</table>";

    page += "<br><p><a href='/'>← Back to SCD30</a></p>";

    page += "</body></html>";

    sendHtml(200, page);
}

// ==================================================
// MQTT test
// ==================================================

void handleMQTTTest()
{
    if (!tokenOK())
    {
        sendHtml(403, errorPage("Virheellinen tai puuttuva portal token."));
        return;
    }

    String testServer = server.arg("mqttServer");
    int testPort = server.arg("mqttPort").toInt();
    String testUser = server.arg("mqttUser");
    String testPassword = server.arg("mqttPass");

    // Empty password means: use stored password.
    if (testPassword.length() == 0)
        testPassword = mqttPassword;

    if (!validLength(testServer, 128) ||
        !validLength(testUser, 128) ||
        !validLength(testPassword, 128) ||
        !validPort(testPort))
    {
        sendHtml(400, errorPage("MQTT-asetukset eivät täytä sallittuja arvoja."));
        return;
    }

    WiFiClientSecure testClient;
    testClient.setCACert(ROOT_CA);

    PubSubClient testMQTT(testClient);
    testMQTT.setServer(testServer.c_str(), testPort);

    String testClientId = "ESP32-SCD30-TEST-" + WiFi.macAddress();

    bool connected = testMQTT.connect(
        testClientId.c_str(),
        testUser.c_str(),
        testPassword.c_str()
    );

    String page;

    page += "<!DOCTYPE html><html><head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<title>MQTT testi</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head><body>";

    page += "<h2>MQTT-yhteystesti</h2>";

    if (connected)
    {
        page += "<p><b>✓ MQTT-yhteys onnistui!</b></p>";
        testMQTT.disconnect();
    }
    else
    {
        page += "<p><b>✗ MQTT-yhteys epäonnistui</b></p>";
        page += "<p>Virhekoodi: ";
        page += String(testMQTT.state());
        page += "</p>";
    }

    page += "<p><a href='/setup'>Takaisin asetuksiin</a></p>";
    page += "</body></html>";

    sendHtml(200, page);
}

// ==================================================
// Save settings
// ==================================================

void handleSave()
{
    if (!tokenOK())
    {
        sendHtml(403, errorPage("Virheellinen tai puuttuva portal token."));
        return;
    }

    String newSSID = server.arg("ssid");
    String newWiFiPassword = server.arg("pass");
    String newMQTTServer = server.arg("mqttServer");
    int newMQTTPort = server.arg("mqttPort").toInt();
    String newMQTTUser = server.arg("mqttUser");
    String newMQTTPassword = server.arg("mqttPass");
    String newMQTTTopic = server.arg("mqttTopic");

    if (!validLength(newSSID, 64) ||
        !validLength(newWiFiPassword, 128) ||
        !validLength(newMQTTServer, 128) ||
        !validLength(newMQTTUser, 128) ||
        !validLength(newMQTTPassword, 128) ||
        !validLength(newMQTTTopic, 128) ||
        !validPort(newMQTTPort))
    {
        sendHtml(400, errorPage("Asetuksissa on liian pitkä kenttä tai virheellinen MQTT-portti."));
        return;
    }

    if (newSSID.length() == 0 || newMQTTServer.length() == 0 || newMQTTTopic.length() == 0)
    {
        sendHtml(400, errorPage("SSID, MQTT Server ja Topic ovat pakollisia."));
        return;
    }

    prefs.begin("config", false);

    prefs.putString("ssid", newSSID);

    // Empty password = keep the existing password.
    if (newWiFiPassword.length() > 0)
        prefs.putString("pass", newWiFiPassword);

    prefs.putString("mqttServer", newMQTTServer);
    prefs.putInt("mqttPort", newMQTTPort);
    prefs.putString("mqttUser", newMQTTUser);

    if (newMQTTPassword.length() > 0)
        prefs.putString("mqttPass", newMQTTPassword);

    prefs.putString("mqttTopic", newMQTTTopic);

    prefs.end();

    String page;
    page += "<!DOCTYPE html><html><head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<title>SCD30</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head><body>";
    page += "<h2>Asetukset tallennettu!</h2>";
    page += "<p>ESP32 käynnistyy uudelleen...</p>";
    page += "</body></html>";

    sendHtml(200, page);

    delay(1200);
    ESP.restart();
}

// ==================================================
// CSS
// ==================================================

void handleStyle()
{
    String css;

    css += "body{font-family:Arial,Helvetica,sans-serif;font-size:16px;line-height:1.5;margin:20px;color:#222;}";
    css += "h1,h2,h3{font-family:Arial,Helvetica,sans-serif;}";
    css += "input,button,select,textarea{font-family:Arial,Helvetica,sans-serif;font-size:16px;}";
    css += "input[type=text],input[type=password],input[type=number]{width:100%;max-width:450px;box-sizing:border-box;padding:8px;margin-top:4px;}";
    css += "button,input[type=submit]{padding:8px 14px;margin-top:8px;cursor:pointer;}";
    css += "table{font-family:Arial,Helvetica,sans-serif;font-size:15px;border-collapse:collapse;margin-top:10px;}";
    css += "th,td{border:1px solid #999;padding:6px 10px;text-align:left;}";
    css += "th{font-weight:bold;}";
    css += "a{font-family:Arial,Helvetica,sans-serif;}";

    sendSecurityHeaders();
    server.send(200, "text/css", css);
}

// ==================================================
// Portal
// ==================================================

void startPortal()
{
    portalMode = true;

    WiFi.disconnect(true);
    delay(500);

    WiFi.mode(WIFI_AP_STA);

    String mac = WiFi.macAddress();
    mac.replace(":", "");

    apName = "SCD30-Setup-" + mac.substring(mac.length() - 6);
    apPassword = makeRandomAPPassword();
    portalToken = makePortalToken();

    WiFi.softAP(apName.c_str(), apPassword.c_str());

    Serial.println();
    Serial.println("================================");
    Serial.println("SCD30 WiFi-asetustila");
    Serial.println("================================");
    Serial.print("AP name: ");
    Serial.println(apName);
    Serial.print("AP password: ");
    Serial.println(apPassword);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("================================");

    dns.start(53, "*", WiFi.softAPIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/setup", HTTP_GET, handleSetup);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/mqtt-test", HTTP_POST, handleMQTTTest);
    server.on("/style.css", HTTP_GET, handleStyle);
    server.on("/board-info", HTTP_GET, handleBoardInfo);

    server.onNotFound([]()
    {
        sendHtml(404, errorPage("404 - Not Found"));
    });

    server.begin();
}

// ==================================================
// Normal WebServer
// ==================================================

void startWebServer()
{
    portalMode = false;

    server.on("/", HTTP_GET, handleRoot);
    server.on("/setup", HTTP_GET, handleSetup);
    server.on("/style.css", HTTP_GET, handleStyle);
    server.on("/board-info", HTTP_GET, handleBoardInfo);

    // Configuration-changing endpoints are intentionally NOT registered
    // in normal STA mode.

    server.onNotFound([]()
    {
        sendHtml(404, errorPage("404 - Not Found"));
    });

    server.begin();

    Serial.println("WebServer valmis");
}

// ==================================================
// Settings
// ==================================================

bool loadSettings()
{
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
    Serial.println("Salasanoja ei tulosteta.");

    if (wifiSSID.length() == 0)
        return false;

    if (mqttServer.length() == 0)
        return false;

    if (!validPort(mqttPort))
        return false;

    return true;
}

bool connectWiFi()
{
    Serial.println("connectWiFi()");

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

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

// ==================================================
// MQTT
// ==================================================

String mqttClientId()
{
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    return "ESP32-SCD30-" + mac;
}

bool connectMQTT()
{
    Serial.print("Yhdistetään MQTT... ");

    if (client.connected())
    {
        Serial.println("jo yhdistetty");
        return true;
    }

    String clientId = mqttClientId();

    if (client.connect(
            clientId.c_str(),
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

// ==================================================
// Setup
// ==================================================

void setup()
{
    Serial.begin(115200);
    delay(200);

    // Boot count
    prefs.begin("config", false);
    bootCount = prefs.getUInt("bootCount", 0);
    bootCount++;
    prefs.putUInt("bootCount", bootCount);
    prefs.end();

    // Board-specific I2C pins:
    // Seeed Studio XIAO ESP32-S3
    Wire.begin(5, 6);

    Wire.setClock(50000);

    if (!scd30.begin())
    {
        Serial.println("SCD30 ei löytynyt!");

        while (1)
            delay(1000);
    }

    Serial.println("SCD30 OK");

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

    // IMPORTANT:
    // V6 verifies the MQTT server certificate.
    // Never replace this with setInsecure() in production.
    wifiClient.setCACert(ROOT_CA);

    client.setServer(mqttServer.c_str(), mqttPort);

    connectMQTT();
}

// ==================================================
// Main loop
// ==================================================

void loop()
{
    server.handleClient();

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

    if (!client.connected())
    {
        if (millis() - lastMQTTAttempt >= mqttRetryInterval)
        {
            lastMQTTAttempt = millis();
            connectMQTT();
        }
    }

    client.loop();

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

                char payload[128];

                snprintf(
                    payload,
                    sizeof(payload),
                    "{\"co2\":%.0f,\"temperature\":%.2f,\"humidity\":%.2f}",
                    co2,
                    temp,
                    hum
                );

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
                    Serial.println(
                        "MQTT ei ole yhteydessä - viestiä ei lähetetty."
                    );
                }
            }
        }
    }
}
