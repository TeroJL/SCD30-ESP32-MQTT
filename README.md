# SCD30-ESP32-MQTT

ESP32-pohjainen **Sensirion SCD30 CO₂-, lämpötila- ja kosteudenmittausprojekti**, jossa sama sovellus on toteutettu kahdelle eri ESP32-mikrokontrollerialustalle:

- **Wemos D1 R32 / ESP32**
- **Seeed Studio XIAO ESP32-S3**

Molemmat versiot lukevat SCD30-anturilta:

- CO₂-pitoisuuden
- lämpötilan
- suhteellisen kosteuden

Mittaustiedot julkaistaan MQTT-brokerille JSON-muodossa.

Projektissa on lisäksi **WiFi-asetusportaali**, jonka avulla WiFi- ja MQTT-asetukset voidaan määrittää ilman tunnusten kirjoittamista lähdekoodiin.

---

## Projektin tarkoitus

Projektin tarkoituksena on toimia sekä toimivana IoT-mittauslaitteena että oppimisprojektina.

Projektissa voidaan harjoitella muun muassa:

- ESP32-ohjelmointia
- I²C-kommunikaatiota
- Sensirion SCD30 -anturin käyttöä
- WiFi-yhteyksiä
- WebServer-palvelinta
- Preferences/NVS-muistia
- MQTT-protokollaa
- JSON-dataa
- TLS-salausta
- HTTP-tietoturvaa
- käyttäjän syötteiden validointia
- ohjelmiston tietoturvan parantamista

---

# Versiot

## V5 Basic

V5 on projektin helposti seurattava **oppimisversio**.

Se soveltuu erityisesti ESP32-, WiFi-, WebServer-, Preferences- ja MQTT-ohjelmoinnin opiskeluun.

V5:n tarkoituksena ei ole olla projektin turvallisin mahdollinen toteutus, vaan tarjota mahdollisimman selkeä lähtökohta V6:n tietoturvaparannusten ymmärtämiseen.

## V6 Secure – Tested

V6 perustuu vastaavaan V5-versioon, mutta sisältää useita tietoturvaparannuksia.

**Molemmat V6-versiot on testattu toimiviksi oikealla laitteistolla:**

- Wemos D1 R32 / ESP32 ✅
- Seeed Studio XIAO ESP32-S3 ✅

V6 on tämän projektin suositeltu versio varsinaiseen käyttöön.

---

# Projektin rakenne

```text
SCD30-ESP32-MQTT/
│
├── README.md
│
├── images/
│   ├── scd30-wemos.jpg
│   ├── scd30-sensor.jpg
│   ├── scd30-wemos-wiring.jpg
│   ├── scd30-speedstudio-1.jpg
│   └── scd30-speedstudio-2.jpg
│
└── examples/
    ├── SCD30-Wemos-D1-R32-V5/
    │   └── SCD30-Wemos-D1-R32-V5.ino
    │
    ├── SCD30-Wemos-D1-R32-V6/
    │   └── SCD30-Wemos-D1-R32-V6.ino
    │
    ├── SCD30-XIAO-ESP32-S3-V5/
    │   └── SCD30-XIAO-ESP32-S3-V5.ino
    │
    └── SCD30-XIAO-ESP32-S3-V6/
        └── SCD30-XIAO-ESP32-S3-V6.ino
```

Kansio- ja tiedostonimet on tarkoituksella nimetty niin, että käytetty SCD30-anturi, mikrokontrollerialusta ja ohjelmistoversio näkyvät suoraan nimestä.

---

# Mikrokontrollerit

| Ominaisuus | Wemos D1 R32 | XIAO ESP32-S3 |
|---|---|---|
| MCU | ESP32 | ESP32-S3 |
| SCD30 I²C SDA | GPIO21 | GPIO5 |
| SCD30 I²C SCL | GPIO22 | GPIO6 |
| WiFi | 2,4 GHz | 2,4 GHz |
| MQTT | Kyllä | Kyllä |
| WiFi-asetusportaali | Kyllä | Kyllä |
| V6 TLS | Kyllä | Kyllä |

Sama sovelluslogiikka on sovitettu kahdelle eri ESP32-alustalle. Tärkein alustakohtainen ero tässä projektissa on SCD30:n I²C-pinnien määritys.

---

# Kytkentä

## Wemos D1 R32

| SCD30 | Wemos D1 R32 | Tarkoitus |
|---|---|---|
| VIN | 3V3 | Käyttöjännite |
| GND | GND | Maa |
| SDA | GPIO21 / SDA | I²C-data |
| SCL | GPIO22 / SCL | I²C-kello |

Arduino-koodissa:

```cpp
Wire.begin(21, 22);
```

## XIAO ESP32-S3

| SCD30 | XIAO ESP32-S3 | Tarkoitus |
|---|---|---|
| VIN | 3V3 | Käyttöjännite |
| GND | GND | Maa |
| SDA | GPIO5 | I²C-data |
| SCL | GPIO6 | I²C-kello |

Arduino-koodissa:

```cpp
Wire.begin(5, 6);
```

SCD30:n I²C-osoite on:

```text
0x61
```

---

# Kuvat

## Koko laitteisto

![Wemos D1 R32 ja SCD30](images/scd30-wemos.jpg)

## SCD30-anturi

![SCD30-anturi](images/scd30-sensor.jpg)

## Käytännön kytkentä

![SCD30 ja Wemos](images/scd30-wemos-wiring.jpg)

## SCD30-anturi – SpeedStudio

![SCD30-anturi SpeedStudio 1](images/scd30-speedstudio-1.jpg)

![SCD30-anturi SpeedStudio 2](images/scd30-speedstudio-2.jpg)

---

# V5 → V6

V6:n lähtökohtana ovat vastaavat V5-ohjelmat:

```text
SCD30-Wemos-D1-R32-V5/
└── SCD30-Wemos-D1-R32-V5.ino

SCD30-XIAO-ESP32-S3-V5/
└── SCD30-XIAO-ESP32-S3-V5.ino
```

V6-versiot ovat:

```text
SCD30-Wemos-D1-R32-V6/
└── SCD30-Wemos-D1-R32-V6.ino

SCD30-XIAO-ESP32-S3-V6/
└── SCD30-XIAO-ESP32-S3-V6.ino
```

Molemmissa V5-ohjelmissa on sama perusrakenne.

Wemos käyttää:

```cpp
Wire.begin(21, 22);
```

ja XIAO ESP32-S3:

```cpp
Wire.begin(5, 6);
```

Muu WiFi-, MQTT-, WebServer- ja SCD30-toiminta on rakennettu samalla periaatteella.

---

# V6:n tietoturvaparannukset

## 1. MQTT TLS -varmenteen tarkistus

V5 käyttää:

```cpp
wifiClient.setInsecure();
```

Tämä tarkoittaa, ettei MQTT-palvelimen TLS-varmennetta tarkisteta.

V6 käyttää:

```cpp
wifiClient.setCACert(ROOT_CA);
```

V6:ssa on mukana ISRG Root X1 -juurivarmenne Let's Encrypt -sertifikaattiketjuja varten.

Jos käytät muuta MQTT-brokeria tai muuta CA:ta, `ROOT_CA` tulee vaihtaa vastaamaan käytetyn brokerin sertifikaattiketjua.

> Älä muuta V6:ssa `setCACert()`-kutsua takaisin `setInsecure()`-kutsuksi, jos tavoitteena on TLS-varmenteen oikea tarkistus.

## 2. NTP-aika

TLS-varmenteen tarkistus tarvitsee oikean kellonajan.

V6:ssa TLS-varmennetta ei ohiteta `setInsecure()`-ratkaisulla. ESP32:n tulee synkronoida kellonaika ennen TLS-yhteyden muodostamista.

## 3. Salasanoja ei palauteta HTML-lomakkeeseen

V5:n toteutuksessa nykyinen salasana voidaan sijoittaa HTML:n `value`-kenttään.

V6:ssa salasanoja ei palauteta selaimelle.

Salasanakenttä jätetään tyhjäksi, ja jos käyttäjä jättää sen tyhjäksi, nykyinen salasana säilytetään.

## 4. Salasanoja ei tulosteta Serial Monitoriin

V6 ei tulosta tallennettuja WiFi- tai MQTT-salasanoja Serial Monitoriin.

## 5. Portal token

Kun WiFi-asetusportaali käynnistyy, V6 muodostaa `esp_random()`-funktiolla satunnaisen tokenin.

Token vaaditaan asetusten tallennukseen ja MQTT-yhteystestiin.

## 6. Asetusten muuttaminen vain portal-tilassa

V5:n `/save`- ja `/mqtt-test`-endpointit ovat käytettävissä laajemmin.

V6:ssa asetusten muuttamiseen tarkoitetut endpointit rekisteröidään vain WiFi-asetusportaalin aikana.

## 7. Satunnainen AP-salasana

V6 muodostaa WiFi-asetusportaalille uuden satunnaisen AP-salasanan jokaisella portal-tilan käynnistyskerralla.

AP:n nimi on muotoa:

```text
SCD30-Setup-XXXXXX
```

Salasana muodostetaan satunnaisesti ja tulostetaan Serial Monitoriin asetustilan aikana.

## 8. HTML escaping

Käyttäjän syöttämät tiedot käsitellään `htmlEscape()`-funktiolla ennen niiden lisäämistä HTML-sivulle.

Tämä estää esimerkiksi `<`, `>`, `"` ja `'` -merkkien päätymisen suoraan HTML-rakenteeseen.

## 9. Syötteiden validointi

V6 tarkistaa palvelinpuolella muun muassa:

- SSID:n enimmäispituuden
- WiFi-salasanan enimmäispituuden
- MQTT-palvelimen enimmäispituuden
- MQTT-käyttäjänimen enimmäispituuden
- MQTT-salasanan enimmäispituuden
- MQTT-topicin enimmäispituuden
- MQTT-portin alueen 1–65535
- pakolliset kentät

## 10. Yksilöllinen MQTT Client ID

V5 käyttää kaikille laitteille samaa Client ID:tä.

V6 muodostaa Client ID:n laitteen MAC-osoitteen perusteella.

Esimerkiksi:

```text
ESP32-SCD30-A1B2C3D4E5F6
```

Tämä on tärkeää, jos samassa MQTT-brokerissa käytetään useita SCD30-laitteita.

## 11. HTTP security headers

V6 lisää HTTP-vastauksiin muun muassa:

```text
Cache-Control: no-store
Pragma: no-cache
X-Content-Type-Options: nosniff
X-Frame-Options: DENY
Referrer-Policy: no-referrer
Content-Security-Policy
```

---

# V5 ja V6

| Ominaisuus | V5 Basic | V6 Secure |
|---|---:|---:|
| SCD30 | ✅ | ✅ |
| WiFi | ✅ | ✅ |
| WiFi-asetusportaali | ✅ | ✅ |
| MQTT | ✅ | ✅ |
| MQTT-yhteystesti | ✅ | ✅ |
| MQTT TLS | ⚠️ | ✅ |
| TLS-varmenteen tarkistus | ❌ | ✅ |
| Salasanojen palauttaminen HTML:ään | ⚠️ | ❌ |
| Salasanojen tulostaminen Serial Monitoriin | ⚠️ | ❌ |
| Portal token | ❌ | ✅ |
| Asetusendpointit vain portal-tilassa | ❌ | ✅ |
| Satunnainen AP-salasana | ❌ | ✅ |
| HTML escaping | ❌ | ✅ |
| Syötteiden validointi | Perustaso | ✅ |
| Yksilöllinen MQTT Client ID | ❌ | ✅ |
| HTTP security headers | ❌ | ✅ |
| Boot count | ✅ | ✅ |
| Reset reason | ✅ | ✅ |
| Device Information | ✅ | ✅ |
| Uptime | – | ✅ |
| Fyysisesti testattu | – | **✅** |

---

# MQTT

Molemmat mikrokontrolleriversiot lähettävät saman JSON-rakenteen:

```json
{
  "co2": 561,
  "temperature": 27.68,
  "humidity": 35.95
}
```

| Kenttä | Selitys |
|---|---|
| `co2` | CO₂-pitoisuus ppm |
| `temperature` | lämpötila °C |
| `humidity` | suhteellinen kosteus % |

Oletustopic:

```text
scd30/data
```

Oletusportti:

```text
8883
```

Portti 8883 on tarkoitettu MQTT TLS -yhteydelle.

---

# WiFi-asetusportaali

Jos WiFi-asetuksia ei ole tallennettu tai WiFi-yhteyden muodostaminen epäonnistuu, laite käynnistää oman WiFi-asetusverkon.

V6:n verkko on muotoa:

```text
SCD30-Setup-XXXXXX
```

Salasana muodostetaan satunnaisesti laitteen käynnistyessä.

Asetustilassa käyttäjä voi määrittää:

- WiFi SSID:n
- WiFi-salasanan
- MQTT-palvelimen
- MQTT-portin
- MQTT-käyttäjänimen
- MQTT-salasanan
- MQTT-topicin

Asetussivulla voidaan myös testata MQTT-yhteys ennen asetusten tallentamista.

---

# Device Information

Device Information -sivulla näytetään muun muassa:

- ESP32-malli
- chip revision
- CPU-ytimien määrä
- CPU-taajuus
- flash-muistin koko
- vapaa heap-muisti
- sketchin koko
- vapaa sketch-tila
- Arduino Core -versio
- ESP-IDF-versio
- MAC-osoite
- IP-osoite
- gateway
- DNS
- RSSI
- boot count
- reset reason
- reset reasonin numeerinen arvo
- laitteen uptime
- SCD30:n I²C-osoite

Reset reason näytetään esimerkiksi muodossa:

```text
Software reset (3)
```

Uptime kertoo, kuinka kauan laite on ollut käynnissä viimeisimmän resetin jälkeen.

---

# PermissionError Windowsissa

ESP32-firmwarea ladattaessa voi esiintyä esimerkiksi:

```text
PermissionError(13, 'Access is denied.')
```

tai:

```text
SerialException: could not open port 'COMx'
PermissionError(13, 'Access is denied.')
```

Tämä ei välttämättä tarkoita, että Arduino-koodissa on virhe.

Ongelma voi liittyä esimerkiksi:

- Windowsin COM-porttiin
- USB-sarjaportin ajuriin
- toisen ohjelman käyttämään COM-porttiin
- USB-kaapeliin
- USB-porttiin
- `esptool`-ohjelmaan
- ESP32-korttipakettiin

## Espressif esptool issue #682

Espressifin `esptool`-projektissa on dokumentoitu vastaava Windows/COM-portti/`PermissionError(13)` -ongelma:

https://github.com/espressif/esptool/issues/682

Issue #682 liittyy alkuperäisesti ESP8266/Windows/esptool-ympäristöön, mutta se on hyödyllinen viite samantyyppisen COM-porttiongelman selvittämiseen.

## Kokeile

1. Sulje Arduino IDE.
2. Irrota kortti USB-kaapelista.
3. Varmista, ettei toinen ohjelma käytä COM-porttia.
4. Liitä kortti uudelleen.
5. Tarkista COM-portti Windowsin Laitehallinnasta.
6. Käynnistä Arduino IDE uudelleen.
7. Kokeile toista USB-porttia.
8. Tarkista USB-sarjaportin ajuri.
9. Kokeile toista USB-kaapelia.
10. Kokeile upload-nopeudeksi esimerkiksi `115200`.

### Aloittelijalle tärkeä ero

```text
Arduino-koodi
      │
      ▼
   Käännös
      │
      ├── ❌ Compile error
      │       → koodiongelma
      │
      ▼
 Firmware valmis
      │
      ▼
   esptool
      │
      ▼
 Windows / USB / COM
      │
      └── ❌ PermissionError
              → latausympäristön ongelma
```

Jos ohjelma kääntyy onnistuneesti, älä muuta toimivaa Arduino-koodia vain siksi, että firmwareen lataaminen epäonnistuu.

---

# Arduino IDE

Tarvitset:

- Arduino IDE:n
- ESP32-korttipaketin
- CH340 USB-sarjaporttiajurin, jos käyttämäsi Wemos-kortti sitä tarvitsee
- Adafruit SCD30 -kirjaston
- PubSubClient-kirjaston

## Wemos D1 R32

Valitse Arduino IDE:ssä esimerkiksi:

```text
ESP32 Dev Module
```

SCD30:

```cpp
Wire.begin(21, 22);
```

## XIAO ESP32-S3

Valitse Arduino IDE:ssä XIAO ESP32-S3:n mukainen kortti.

SCD30:

```cpp
Wire.begin(5, 6);
```

---

# Suositeltu oppimispolku

1. Aloita Wemos D1 R32:n V5-versiosta.
2. Opettele SCD30:n I²C-kommunikaatio.
3. Opettele ESP32:n WiFi.
4. Opettele WebServer.
5. Opettele Preferences/NVS.
6. Opettele MQTT.
7. Tutki V5:n tietoturvarajoituksia.
8. Vertaa Wemos V5 → V6.
9. Vertaa XIAO V5 → V6.
10. Tutki, mitkä osat ovat alustariippumattomia ja mitkä liittyvät I²C-pinneihin.
11. Tutki MQTT TLS -yhteyttä.
12. Tutustu V6:n tietoturvaratkaisuihin.
13. Kehitä oma V7.

Tämä tekee projektista hyvän harjoituksen myös ohjelmakoodin uudelleenkäytöstä: sama sovelluslogiikka voidaan siirtää toiselle ESP32-alustalle vaihtamalla tarvittavat laitteistokohtaiset osat.

---

# Tietoturvahuomio

V6 on selvästi V5:tä turvallisempi, mutta sitä ei ole suunniteltu suoraan Internetiin altistettavaksi Web-palvelimeksi.

Rajoituksia:

- WiFi-asetusportaali käyttää HTTP:tä, ei HTTPS:ää.
- MQTT TLS suojaa MQTT-yhteyden, mutta ei paikallista HTTP-asetusliikennettä.
- AP-salasana tulostetaan Serial Monitoriin asetustilan aikana.
- ESP32:n resurssit rajoittavat WebServerin ominaisuuksia.
- Laitetta ei tule asettaa suoraan Internetiin ilman erillistä suojausta.

V6 on tarkoitettu ensisijaisesti luotettuun paikalliseen IoT-verkkoon.

---

# Tuleva V7

Mahdollisia jatkokehityskohteita:

- HTTPS-asetusportaali
- OTA-päivitys
- MQTT Last Will
- online/offline-tila
- parempi virheenkäsittely
- sensorin tilan valvonta
- sertifikaatin vaihtaminen asetussivulta
- sertifikaattien automaattinen päivitys
- salaisuuksien parempi suojaus NVS-muistissa
- yhteinen alustariippumaton kirjasto Wemos- ja XIAO-versioille

---

# Lisenssi

Tämä projekti on julkaistu MIT-lisenssillä.

Copyright © 2026 Tero Leinonen.
