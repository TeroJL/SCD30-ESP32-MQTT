#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SCD30.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <time.h>
#include <esp_system.h>

// ============================================================
// SCD30 + ESP32 + WiFi-asetusportaali + MQTT
// Version 6 - turvallisempi versio
// ============================================================
//
// Tärkeimmät muutokset versioon 5:
// 1. MQTT TLS tarkistaa palvelimen sertifikaatin (ei setInsecure()).
// 2. Käytössä HiveMQ Cloudin käyttämä ISRG Root X1 -juurivarmenne.
// 3. TLS:n vaatima kellonaika haetaan NTP-palvelimelta.
// 4. Asetusportaali on käytettävissä vain AP-tilassa.
// 5. /save ja /mqtt-test suojataan istuntotunnisteella.
// 6. Salasanoja ei tulosteta Serial Monitoriin.
// 7. Salasanoja ei palauteta HTML:n value-kenttiin.
// 8. Tyhjä salasanakenttä tarkoittaa "säilytä nykyinen salasana".
// 9. MQTT Client ID muodostetaan ESP32:n MAC-osoitteesta.
// 10. HTML-erikoismerkit escapetaan.
// 11. Syötteille tehdään perustason validointi.
// 12. HTTP-vastauksille lisätään turvallisuusheaderit ja no-store.
//
// HUOMIO:
// Tämä CA-varmenne on ISRG Root X1 ja sopii HiveMQ Cloudin
// Let's Encrypt -sertifikaattiketjuun. Jos käytät muuta MQTT-\n// brokeria, CA-varmenne voi olla vaihdettava.
// ============================================================

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

// Asetusportaali
bool portalMode = false;
String portalToken;
String apName;
String apPassword;

// WiFi access point
const char* AP_NAME_PREFIX = "SCD30-Setup-";

// ------------------------------------------------------------
// HiveMQ Cloud / Let's Encrypt - ISRG Root X1
// ------------------------------------------------------------
// Lähde: Let's Encrypt ISRG Root X1.
// HiveMQ Cloud käyttää julkisilla palveluilla TLS-varmenteita,
// joiden ketju voi päätyä ISRG Root X1 -juurivarmenteeseen.
// ------------------------------------------------------------
const char* ROOT_CA =
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n";

// ------------------------------------------------------------
// MQTT / SCD30
// ------------------------------------------------------------
WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);
Adafruit_SCD30 scd30;

unsigned long lastPublish = 0;
const unsigned long publishInterval = 10000;

unsigned long lastMQTTAttempt = 0;
const unsigned long mqttRetryInterval = 10000;

// ------------------------------------------------------------
// HTML-apufunktiot
// ------------------------------------------------------------

String htmlEscape(const String& input)
{
    String output = input;
    output.replace("&", "&amp;");
    output.replace("<", "&lt;");
    output.replace(">", "&gt;");
    output.replace("\"", "&quot;");
    output.replace("'", "&#39;");
    return output;
}

void securityHeaders()
{
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("X-Content-Type-Options", "nosniff");
    server.sendHeader("X-Frame-Options", "DENY");
    server.sendHeader("Referrer-Policy", "no-referrer");
    server.sendHeader("Content-Security-Policy", "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'");
}

void sendHtml(int code, const String& page)
{
    securityHeaders();
    server.send(code, "text/html; charset=UTF-8", page);
}

bool portalAuthorized()
{
    if (!portalMode)
        return false;

    if (!server.hasArg("token"))
        return false;

    return server.arg("token") == portalToken;
}

String mqttClientId()
{
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    return "ESP32-SCD30-" + mac;
}

String createAPPassword()
{
    String mac = WiFi.macAddress();
    mac.replace(":", "");

    // Viimeiset 8 heksamerkkiä + etuliite = 14 merkkiä.
    if (mac.length() >= 8)
        return "SCD30-" + mac.substring(mac.length() - 8);

    return "SCD30-Setup-8K4P";
}

// ------------------------------------------------------------
// Asetussivu
// ------------------------------------------------------------
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
    page += "<p><b>Huom:</b> salasanoja ei näytetä tällä sivulla. Tyhjä salasana säilyttää nykyisen salasanan.</p>";

    page += "<form action='/save' method='POST' autocomplete='off'>";
    page += "<input type='hidden' name='token' value='" + htmlEscape(portalToken) + "'>";

    // WiFi
    page += "<h3>WiFi</h3>";
    page += "SSID:<br>";
    page += "<input name='ssid' maxlength='32' required value='" + htmlEscape(wifiSSID) + "'><br><br>";

    page += "Password:<br>";
    page += "<input type='password' name='pass' maxlength='64' autocomplete='new-password'><br><br>";

    // MQTT
    page += "<h3>MQTT</h3>";

    page += "Server:<br>";
    page += "<input name='mqttServer' maxlength='255' required value='" + htmlEscape(mqttServer) + "'><br><br>";

    page += "Port:<br>";
    page += "<input type='number' name='mqttPort' min='1' max='65535' value='" + String(mqttPort) + "' required><br><br>";

    page += "Username:<br>";
    page += "<input name='mqttUser' maxlength='128' value='" + htmlEscape(mqttUser) + "'><br><br>";

    page += "Password:<br>";
    page += "<input type='password' name='mqttPass' maxlength='128' autocomplete='new-password'><br><br>";

    page += "Topic:<br>";
    page += "<input name='mqttTopic' maxlength='255' required value='" + htmlEscape(mqttTopic) + "'><br><br>";

    page += "<input type='submit' value='Tallenna'>";
    page += "</form>";

    page += "<br>";
    page += "<button type='button' onclick='testMQTT()'>Testaa MQTT-yhteys</button>";

    page += "<script>";
    page += "function testMQTT(){";
    page += "const form=document.querySelector('form');";
    page += "const data=new URLSearchParams(new FormData(form));";
    page += "fetch('/mqtt-test',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:data})";
    page += ".then(r=>r.text()).then(h=>{document.open();document.write(h);document.close();})";
    page += ".catch(e=>alert('MQTT-testi epäonnistui: '+e));";
    page += "}";
    page += "</script>";

    page += "<br><br>";
    page += "<a href='/'>Takaisin</a>";
    page += "</body></html>";

    return page;
}

// ------------------------------------------------------------
// Root
// ------------------------------------------------------------
void handleRoot()
{
    if (portalMode)
    {
        sendHtml(200, htmlPage());
        return;
    }

    String page;

    page += "<!DOCTYPE html><html><head>";
    page += "<meta charset='UTF-8'>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<meta name='robots' content='noindex,nofollow'>";
    page += "<title>SCD30</title>";
    page += "<link rel='stylesheet' href='/style.css'>";
    page += "</head><body>";

    page += "<h2>SCD30</h2>";
    page += "<p><b>WiFi:</b> OK</p>";
    page += "<p>SSID: " + htmlEscape(WiFi.SSID()) + "</p>";
    page += "<p>IP: " + htmlEscape(WiFi.localIP().toString()) + "</p>";
    page += "<p>RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
    page += "<p>MQTT: " + String(client.connected() ? "Yhdistetty" : "Ei yhteyttä") + "</p>";
    page += "<hr>";
    page += "<p>Asetusportaali ei ole käytettävissä normaalissa WiFi-tilassa.</p>";

    page += "</body></html>";

    sendHtml(200, page);
}

void handleSetup()
{
    if (!portalMode)
    {
        sendHtml(403, "<html><body><h2>403 - Ei sallittu</h2><p>Asetukset ovat käytettävissä vain SCD30-Setup-tukiaseman kautta.</p></body></html>");
        return;
    }

    sendHtml(200, htmlPage());
}

// ------------------------------------------------------------
// MQTT-test
// ------------------------------------------------------------
void handleMQTTTest()
{
    if (!portalAuthorized())
    {
        sendHtml(403, "<html><body><h2>403 - Ei sallittu</h2></body></html>");
        return;
    }

    String testServer = server.arg("mqttServer");
    int testPort = server.arg("mqttPort").toInt();
    String testUser = server.arg("mqttUser");
    String testPassword = server.arg("mqttPass");

    // Tyhjä salasana tarkoittaa nykyisen tallennetun salasanan käyttöä.
    if (testPassword.length() == 0)
        testPassword = mqttPassword;

    if (testServer.length() == 0 || testPort < 1 || testPort > 65535)
    {
        sendHtml(400, "<html><body><h2>Virhe</h2><p>MQTT-palvelin tai portti on virheellinen.</p><p><a href='/setup'>Takaisin asetuksiin</a></p></body></html>");
        return;
    }

    Serial.println("MQTT-yhteyden testi");
    Serial.print("Server: ");
    Serial.println(testServer);
    Serial.print("Port: ");
    Serial.println(testPort);
    Serial.print("User: ");
    Serial.println(testUser.length() > 0 ? "(asetettu)" : "(ei asetettu)");

    WiFiClientSecure testClient;
    testClient.setCACert(ROOT_CA);

    PubSubClient testMQTT(testClient);
    testMQTT.setServer(testServer.c_str(), testPort);

    String testClientId = mqttClientId() + "-TEST";

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
        Serial.println("MQTT testi: OK");

        page += "<p class='ok'><b>✓ MQTT-yhteys onnistui!</b></p>";
        page += "<p>TLS-palvelinsertifikaatti hyväksyttiin.</p>";

        testMQTT.disconnect();
    }
    else
    {
        int state = testMQTT.state();

        Serial.print("MQTT testi epäonnistui. Virhe: ");
        Serial.println(state);

        page += "<p class='error'><b>✗ MQTT-yhteys epäonnistui</b></p>";
        page += "<p>Virhekoodi: " + String(state) + "</p>";
        page += "<p>Jos käytössä on HiveMQ Cloud, tarkista palvelimen nimi, portti 8883, tunnukset ja että laitteen kellonaika on oikea.</p>";
    }

    page += "<h3>MQTT virhekoodit</h3>";
    page += "<table><tr><th>Koodi</th><th>Merkitys</th></tr>";
    page += "<tr><td>0</td><td>Yhteys onnistui</td></tr>";
    page += "<tr><td>-1</td><td>Yhteys katkaistu</td></tr>";
    page += "<tr><td>-2</td><td>Yhteyden muodostaminen epäonnistui</td></tr>";
    page += "<tr><td>-3</td><td>Yhteys katkaistiin</td></tr>";
    page += "<tr><td>-4</td><td>Yhteyden aikakatkaisu</td></tr>";
    page += "<tr><td>-5</td><td>Virheellinen käyttäjätunnus tai salasana</td></tr>";
    page += "<tr><td>-6</td><td>Virheellinen MQTT-protokolla</td></tr>";
    page += "</table>";

    page += "<p><a href='/setup'>Takaisin asetuksiin</a></p>";
    page += "</body></html>";

    sendHtml(200, page);
}

// ------------------------------------------------------------
// Asetusten tallennus
// ------------------------------------------------------------
void handleSave()
{
    if (!portalAuthorized())
    {
        sendHtml(403, "<html><body><h2>403 - Ei sallittu</h2></body></html>");
        return;
    }

    String newSSID = server.arg("ssid");
    String newWiFiPassword = server.arg("pass");
    String newMQTTServer = server.arg("mqttServer");
    String newMQTTUser = server.arg("mqttUser");
    String newMQTTPassword = server.arg("mqttPass");
    String newMQTTTopic = server.arg("mqttTopic");
    int newMQTTPort = server.arg("mqttPort").toInt();

    // Perustason validointi.
    if (newSSID.length() == 0 || newSSID.length() > 32 ||
        newMQTTServer.length() == 0 || newMQTTServer.length() > 255 ||
        newMQTTPort < 1 || newMQTTPort > 65535 ||
        newMQTTTopic.length() == 0 || newMQTTTopic.length() > 255)
    {
        sendHtml(400, "<html><body><h2>Virhe</h2><p>Yksi tai useampi asetus on virheellinen.</p><p><a href='/setup'>Takaisin</a></p></body></html>");
        return;
    }

    // Tyhjä salasana säilyttää nykyisen.
    if (newWiFiPassword.length() == 0)
        newWiFiPassword = wifiPassword;

    if (newMQTTPassword.length() == 0)
        newMQTTPassword = mqttPassword;

    prefs.begin("config", false);

    prefs.putString("ssid", newSSID);
    prefs.putString("pass", newWiFiPassword);
    prefs.putString("mqttServer", newMQTTServer);
    prefs.putInt("mqttPort", newMQTTPort);
    prefs.putString("mqttUser", newMQTTUser);
    prefs.putString("mqttPass", newMQTTPassword);
    prefs.putString("mqttTopic", newMQTTTopic);

    prefs.end();

    Serial.println("Asetukset tallennettu.");
    Serial.print("WiFi SSID: ");
    Serial.println(newSSID);
    Serial.print("MQTT Server: ");
    Serial.println(newMQTTServer);
    Serial.print("MQTT Port: ");
    Serial.println(newMQTTPort);
    Serial.print("MQTT User: ");
    Serial.println(newMQTTUser.length() > 0 ? "(asetettu)" : "(ei asetettu)");
    Serial.print("MQTT Topic: ");
    Serial.println(newMQTTTopic);
    Serial.println("Salasanoja ei tulosteta.");

    sendHtml(
        200,
        "<html><head><meta charset='UTF-8'></head><body>"
        "<h2>Asetukset tallennettu!</h2>"
        "<p>ESP32 käynnistyy uudelleen...</p>"
        "</body></html>"
    );

    delay(1500);
    ESP.restart();
}

// ------------------------------------------------------------
// CSS
// ------------------------------------------------------------
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

    css += "h1,h2,h3 {font-family: Arial, Helvetica, sans-serif;}";

    css += "input,button,select,textarea {";
    css += "font-family: Arial, Helvetica, sans-serif;";
    css += "font-size: 16px;";
    css += "}";

    css += "input[type='text'],input[type='password'],input[type='number'],input:not([type]) {";
    css += "width:100%;max-width:450px;box-sizing:border-box;padding:8px;margin-top:4px;";
    css += "}";

    css += "button,input[type='submit'] {padding:8px 14px;margin-top:8px;cursor:pointer;}";
    css += "table {font-size:15px;border-collapse:collapse;margin-top:10px;}";
    css += "th,td {border:1px solid #999;padding:6px 10px;text-align:left;}";
    css += "th {font-weight:bold;}";
    css += ".ok {color:green;} .error {color:red;}";
    css += "a {font-family:Arial,Helvetica,sans-serif;}";

    securityHeaders();
    server.send(200, "text/css; charset=UTF-8", css);
}

// ------------------------------------------------------------
// WebServer
// ------------------------------------------------------------
void configureWebServer()
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/setup", HTTP_GET, handleSetup);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/mqtt-test", HTTP_POST, handleMQTTTest);
    server.on("/style.css", HTTP_GET, handleStyle);

    server.onNotFound([]()
    {
        if (portalMode)
        {
            // Captive portal: kaikki tuntemattomat osoitteet tuovat asetussivun.
            sendHtml(200, htmlPage());
        }
        else
        {
            securityHeaders();
            server.send(404, "text/plain; charset=UTF-8", "404 - Not Found");
        }
    });
}

void startPortal()
{
    Serial.println("startPortal()");

    portalMode = true;

    WiFi.disconnect(true);
    delay(500);

    WiFi.mode(WIFI_AP_STA);

    apName = String(AP_NAME_PREFIX) + WiFi.macAddress().substring(12);
    apName.replace(":", "");

    // Yksilöllinen AP-salasana laitteen MAC-osoitteen perusteella.
    apPassword = createAPPassword();

    WiFi.softAP(apName.c_str(), apPassword.c_str());

    Serial.print("AP SSID: ");
    Serial.println(apName);
    Serial.print("AP password: ");
    Serial.println(apPassword);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    // Satunnainen token asetussivun POST-pyyntöihin.
    portalToken = String((uint32_t)esp_random(), HEX) + String((uint32_t)esp_random(), HEX);

    dns.start(53, "*", WiFi.softAPIP());

    server.stop();
    server.begin();

    Serial.println("WebServer AP-tilassa valmis");
}

void startWebServer()
{
    Serial.println("startWebServer()");

    portalMode = false;

    server.stop();
    server.begin();

    Serial.println("WebServer normaalissa STA-tilassa");
}

// ------------------------------------------------------------
// Asetusten lataus
// ------------------------------------------------------------
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
    Serial.println(mqttUser.length() > 0 ? "(asetettu)" : "(ei asetettu)");
    Serial.print("MQTT Topic: ");
    Serial.println(mqttTopic);
    Serial.println("Salasanoja ei tulosteta.");

    if (wifiSSID.length() == 0)
        return false;

    if (mqttServer.length() == 0)
        return false;

    if (mqttPort < 1 || mqttPort > 65535)
        return false;

    if (mqttTopic.length() == 0)
        return false;

    return true;
}

// ------------------------------------------------------------
// WiFi
// ------------------------------------------------------------
bool connectWiFi()
{
    Serial.println("connectWiFi()");

    WiFi.mode(WIFI_STA);
    delay(250);

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
            Serial.println("WiFi-yhteyden aikakatkaisu");
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

// ------------------------------------------------------------
// Ajan synkronointi TLS:ää varten
// ------------------------------------------------------------
bool syncTime()
{
    Serial.println("Synkronoidaan kellonaika TLS:ää varten...");

    configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");

    struct tm timeinfo;

    for (int i = 0; i < 20; i++)
    {
        if (getLocalTime(&timeinfo, 1000))
        {
            Serial.println("Aika OK");
            Serial.printf("UTC: %04d-%02d-%02d %02d:%02d:%02d\n",
                          timeinfo.tm_year + 1900,
                          timeinfo.tm_mon + 1,
                          timeinfo.tm_mday,
                          timeinfo.tm_hour,
                          timeinfo.tm_min,
                          timeinfo.tm_sec);
            return true;
        }

        Serial.print(".");
    }

    Serial.println();
    Serial.println("Kellonajan synkronointi epäonnistui.");
    return false;
}

// ------------------------------------------------------------
// MQTT
// ------------------------------------------------------------
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

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
    delay(200);

    Wire.begin(21, 22);
    Wire.setClock(50000);

    if (!scd30.begin())
    {
        Serial.println("SCD30 ei löytynyt!");
        while (1)
            delay(1000);
    }

    Serial.println("SCD30 OK");

    configureWebServer();

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

    // TLS-varmenteen tarkistus.
    wifiClient.setCACert(ROOT_CA);

    // HiveMQ Cloud käyttää TLS:ää portissa 8883.
    client.setServer(mqttServer.c_str(), mqttPort);

    // Sertifikaatin voimassaolo tarkistetaan laitteen kellonaikaa vasten.
    if (!syncTime())
    {
        Serial.println("TLS-yhteyttä ei aloiteta ilman luotettavaa kellonaikaa.");
    }
    else
    {
        connectMQTT();
    }
}

// ------------------------------------------------------------
// Loop
// ------------------------------------------------------------
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

        // WiFi-yhteyden palautumisen jälkeen päivitetään TLS:n aika.
        syncTime();
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
                    if (client.publish(mqttTopic.c_str(), payload))
                    {
                        Serial.println("MQTT lähetetty:");
                        Serial.println(payload);
                    }
                    else
                    {
                        Serial.println("MQTT publish epäonnistui.");
                    }
                }
                else
                {
                    Serial.println("MQTT ei ole yhteydessä - viestiä ei lähetetty.");
                }
            }
        }
    }
}
