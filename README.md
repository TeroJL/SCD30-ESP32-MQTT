# SCD30-Wemos

ESP32 / Wemos D1 R32 -projekti Sensirion SCD30 -anturille.

Projektissa ESP32 lukee SCD30-anturilta:

* hiilidioksidipitoisuuden (CO₂)
* lämpötilan
* suhteellisen kosteuden

Mittaustiedot julkaistaan MQTT-brokerille JSON-muodossa.

Laitteessa on lisäksi **WiFi-asetusportaali**, jonka avulla WiFi- ja MQTT-asetukset voidaan määrittää ilman, että niitä tarvitsee kirjoittaa suoraan Arduino-ohjelmakoodiin.

Projektissa on kaksi versiota:

* **V5 Basic** – yksinkertainen ja helposti seurattava versio opiskeluun.
* **V6 Secure** – V5:n jatkokehitys, jossa tietoturvaa on parannettu.

---

## Projektin tavoite

Projektin tavoitteena on näyttää, miten ESP32-pohjainen IoT-laite voidaan rakentaa vaiheittain.

```text
V5 Basic
   │
   │ toimiva ja helposti ymmärrettävä
   │ perusratkaisu
   ▼
V6 Secure
   │
   │ tietoturvan parantaminen
   ▼
V7 ...
```

V5 toimii hyvänä lähtökohtana ESP32:n, WiFi:n, WebServerin, Preferences-muistin ja MQTT:n opiskeluun.

V6 näyttää, miten samaa toimivaa ohjelmaa voidaan kehittää turvallisemmaksi ilman, että koko ohjelmaa tarvitsee kirjoittaa uudelleen.

---

# Projektin rakenne

```text
SCD30-Wemos/
│
├── README.md
├── LICENSE
├── .gitignore
│
├── docs/
│   ├── version-5.md
│   └── version-6.md
│
└── examples/
    │
    ├── SCD30_WLAN_MQTT_v5/
    │   └── SCD30_WLAN_MQTT_v5.ino
    │
    └── SCD30_WLAN_MQTT_v6/
        └── SCD30_WLAN_MQTT_v6.ino
```

---

# Laitteisto

Projektin alkuperäinen laitteisto:

* **Wemos D1 R32 / ESP32**
* **Sensirion SCD30**
* USB-kaapeli
* WiFi-verkko
* MQTT-broker

SCD30 on kytketty ESP32:n I²C-väylään.

---

# Ohjelmisto

Ohjelmointi tehdään Arduino IDE:llä.

Tarvitaan:

* Arduino IDE
* ESP32-korttipaketti
* Adafruit SCD30 -kirjasto
* PubSubClient-kirjasto

ESP32:n ympäristö tarjoaa lisäksi muun muassa:

* `WiFi`
* `WiFiClientSecure`
* `WebServer`
* `DNSServer`
* `Preferences`

V6 käyttää lisäksi ESP32:n ajan synkronointiin ja järjestelmätoimintoihin liittyviä kirjastoja.

---

# V5 Basic

## V5:n tarkoitus

V5 on projektin yksinkertaisempi versio.

Se on tarkoitettu erityisesti:

* ESP32-ohjelmoinnin opiskeluun
* WiFi-yhteyden opiskeluun
* MQTT:n opiskeluun
* WebServerin opiskeluun
* Preferences/NVS-muistin opiskeluun
* WiFi-asetusportaalin toiminnan tutkimiseen
* V5:n ja V6:n tietoturvaerojen vertailuun

### V5-koodi

[→ Avaa V5-koodi](examples/SCD30_WLAN_MQTT_v5/SCD30_WLAN_MQTT_v5.ino)

---

## V5:n toiminta

V5:

1. käynnistää ESP32:n
2. lukee tallennetut WiFi- ja MQTT-asetukset
3. muodostaa WiFi-yhteyden
4. lukee SCD30-anturin mittaukset
5. muodostaa MQTT-yhteyden
6. julkaisee mittaustiedot MQTT-brokerille
7. tarjoaa WiFi-asetusportaalin asetusten muuttamista varten

---

## V5:n tietoturva

V5 on ennen kaikkea **oppimisversio**.

Sen tietoturvaa ei ole tarkoitettu tuotantotason IoT-laitteeseen.

Erityisesti V5:n MQTT TLS -yhteydessä käytetään `setInsecure()`-toimintoa. TLS-yhteys on salattu, mutta MQTT-palvelimen sertifikaattia ei tarkisteta samalla tavalla kuin V6:ssa.

V5:n asetussivun suojaus on myös yksinkertaisempi kuin V6:ssa.

**Jos laite tulee normaaliin käyttöön, suositeltava versio on V6.**

---

## V5:n AP-salasana

V5 käyttää esimerkkiasetuksena:

```text
asetukset
```

Tämä on tarkoituksella yksinkertainen oppimisesimerkki.

**Vaihda salasana ennen V5:n käyttämistä oikeassa ympäristössä.**

---

# V6 Secure

## V6:n tarkoitus

V6 on V5:n tietoturvallisempi jatkokehitys.

SCD30:n mittaus- ja MQTT-perustoiminta on pidetty mahdollisimman samanlaisena, mutta useita turvallisuuteen liittyviä kohtia on parannettu.

### V6-koodi

[→ Avaa V6-koodi](examples/SCD30_WLAN_MQTT_v6/SCD30_WLAN_MQTT_v6.ino)

---

# V6:n tietoturvaparannukset

## TLS-varmenteen tarkistus

V5 käyttää:

```cpp
setInsecure();
```

V6 käyttää CA-varmennetta:

```cpp
wifiClient.setCACert(ROOT_CA);
```

V6 tarkistaa näin MQTT-palvelimen TLS-varmenteen.

Tämä on yksi tärkeimmistä V6:n tietoturvaparannuksista.

---

## NTP-ajan synkronointi

TLS-varmenteiden tarkistaminen edellyttää luotettavaa kellonaikaa.

V6 synkronoi ESP32:n kellon NTP-palveluiden avulla ennen TLS-yhteyden muodostamista.

Käytössä ovat muun muassa:

```text
pool.ntp.org
time.nist.gov
time.google.com
```

Jos luotettavaa aikaa ei saada, TLS-yhteyttä ei aloiteta.

---

## Asetusportaalin rajoittaminen

V6 erottaa normaalin käyttötilan ja WiFi-asetustilan.

Asetusten muuttamiseen tarkoitetut toiminnot ovat käytettävissä vain asetustilan aikana.

Tämä vähentää riskiä, että laitteen WiFi- tai MQTT-asetuksia päästäisiin muuttamaan normaalin WLAN-yhteyden kautta.

---

## Asetustoimintojen token

V6 luo asetustilaa varten satunnaisen tokenin.

Token tarkistetaan asetusten tallentamiseen ja MQTT-yhteyden testaamiseen liittyvissä pyynnöissä.

Token ei ole käyttäjätunnus eikä varsinainen käyttäjän istuntohallinta.

Sen tarkoituksena on estää yksinkertaiset luvattomat pyynnöt asetustoimintoihin.

---

## Salasanoja ei palauteta HTML-sivulle

V5:n yksinkertaisemmassa toteutuksessa nykyinen salasana voitiin kirjoittaa HTML-lomakkeen `value`-kenttään.

V6 ei palauta nykyistä salasanaa HTML-lähteeseen.

Esimerkiksi:

```html
<input type="password" name="mqttPass">
```

Jos salasana jätetään tyhjäksi, nykyinen salasana säilytetään.

---

## HTML escaping

V6 käsittelee käyttäjän syöttämät tiedot HTML-escapingilla ennen niiden lisäämistä HTML-sivulle.

Tämä vähentää HTML- ja JavaScript-injektioiden riskiä.

---

## Syötteiden validointi

V6 tarkistaa palvelinpuolella muun muassa:

* SSID:n pituuden
* MQTT-palvelimen pituuden
* MQTT-portin
* MQTT-topicin pituuden
* muiden asetuskenttien pituuksia ja arvoja

Pelkkään selaimen suorittamaan HTML-validointiin ei luoteta.

---

## Yksilöllinen MQTT Client ID

V5 käyttää samaa Client ID:tä eri laitteilla.

V6 muodostaa Client ID:n laitteen MAC-osoitteen perusteella.

Esimerkiksi:

```text
ESP32-SCD30-A1B2C3D4E5F6
```

Tämän ansiosta useat SCD30-laitteet voivat käyttää samaa MQTT-brokeria ilman Client ID -ristiriitoja.

---

## HTTP security headers

V6 lisää HTTP-vastauksiin muun muassa:

```text
Cache-Control: no-store
Pragma: no-cache
X-Content-Type-Options: nosniff
X-Frame-Options: DENY
Referrer-Policy: no-referrer
Content-Security-Policy
```

Näillä rajoitetaan selaimen toimintaa ja vähennetään muun muassa tarpeetonta välimuistiin tallentamista ja sivun upottamista iframeen.

---

# V5 ja V6 vertailu

| Ominaisuus                         |  V5 Basic | V6 Secure |
| ---------------------------------- | :-------: | :-------: |
| SCD30                              |     ✅     |     ✅     |
| CO₂-mittaus                        |     ✅     |     ✅     |
| Lämpötila                          |     ✅     |     ✅     |
| Kosteus                            |     ✅     |     ✅     |
| WiFi                               |     ✅     |     ✅     |
| WiFi-asetusportaali                |     ✅     |     ✅     |
| DNS / captive portal               |     ✅     |     ✅     |
| Preferences / NVS                  |     ✅     |     ✅     |
| MQTT                               |     ✅     |     ✅     |
| MQTT TLS                           |     ⚠️    |     ✅     |
| TLS-varmenteen tarkistus           |     ❌     |     ✅     |
| NTP-ajan synkronointi              |     ❌     |     ✅     |
| Yhteinen AP-oletussalasana         |     ⚠️    |     ❌     |
| Laitekohtainen AP-salasana         |     ❌     |     ✅     |
| Asetusportaalin rajoitus           |     ❌     |     ✅     |
| Asetustoimintojen token            |     ❌     |     ✅     |
| Salasanojen palauttaminen HTML:ään |     ⚠️    |     ❌     |
| HTML escaping                      |     ❌     |     ✅     |
| Syötteiden validointi              | Perustaso |     ✅     |
| Yksilöllinen MQTT Client ID        |     ❌     |     ✅     |
| HTTP security headers              |     ❌     |     ✅     |

---

# WiFi-asetusportaali

Jos ESP32:lle ei ole tallennettu toimivaa WiFi-asetusta, laite käynnistää oman WiFi-asetusverkon.

## V5

V5:n verkon nimi on:

```text
SCD30-Setup
```

Salasana on oletuksena:

```text
asetukset
```

## V6

V6 käyttää laitekohtaista AP-verkon nimeä ja salasanaa.

AP-salasana muodostetaan laitteen MAC-osoitteen perusteella.

Tämä on parempi ratkaisu kuin kaikille laitteille yhteinen oletussalasana, mutta salasana ei ole kryptografisesti satunnainen salaisuus.

---

# MQTT

Projektissa käytetään MQTT:tä SCD30:n mittaustietojen lähettämiseen.

V6 on tarkoitettu käytettäväksi TLS-suojatun MQTT-yhteyden kanssa.

Tyypillinen MQTT TLS -portti on:

```text
8883
```

Jos käytät muuta MQTT-brokeria kuin projektissa käytettyä palvelua, tarkista brokerin TLS-varmenneketju ja käytettävä CA-varmenne.

**Älä koskaan lisää omia MQTT-tunnuksiasi GitHubiin.**

---

# MQTT-viestin rakenne

Mittaustiedot julkaistaan JSON-muodossa.

Esimerkiksi:

```json
{
  "co2": 561,
  "temperature": 27.68,
  "humidity": 35.95
}
```

Kentät:

| Kenttä        | Tyyppi | Selitys                |
| ------------- | ------ | ---------------------- |
| `co2`         | numero | CO₂-pitoisuus ppm      |
| `temperature` | numero | lämpötila °C           |
| `humidity`    | numero | suhteellinen kosteus % |

MQTT-topic määritetään laitteen asetussivulla.

---

# Firmwareen lataaminen ja PermissionError

## ⚠️ Jos firmwarea ei saada ladattua

ESP32:n ohjelmoinnissa voi joskus esiintyä Windowsissa virhe:

```text
PermissionError(13, 'Access is denied.')
```

tai:

```text
SerialException: could not open port 'COMx'
PermissionError(13, 'Access is denied.')
```

Tärkeää on ymmärtää, että **tämä ei välttämättä tarkoita, että Arduino-ohjelmassa tai firmware-koodissa olisi virhe**.

Ongelma voi liittyä esimerkiksi:

* Windowsin COM-porttiin
* USB-sarjaportin ajuriin
* toisen ohjelman käyttämään COM-porttiin
* USB-yhteyteen
* `esptool`-ohjelmaan
* ESP32:n korttipaketin ja `esptool`-version yhteensopivuuteen

### Espressifin virallinen issue

Espressifin virallisessa `esptool`-projektissa on dokumentoitu vastaava Windows-ympäristössä esiintyvä ongelma:

[Espressif esptool – PermissionError(13, 'Access is denied.') #682](https://github.com/espressif/esptool/issues/682)

Issue #682:ssa `esptool` ei pysty avaamaan Windowsin COM-porttia ja antaa `PermissionError(13)`-virheen. Tapauksessa ongelma esiintyi Windows 10 -ympäristössä ESP8266:n kanssa.

**Huom:** Issue #682 ei ole juuri Wemos D1 R32:n nykyinen ESP32-ongelmanratkaisu, vaan dokumentoi saman tyyppisen `esptool`/Windows/COM-portti -ongelman. Se on kuitenkin hyödyllinen lähtökohta, jos `PermissionError(13)` ilmestyy firmwarea ladattaessa.

---

## Mitä kannattaa kokeilla?

Jos Wemos D1 R32 näkyy Windowsissa oikealla COM-portilla, mutta firmwareen lataaminen epäonnistuu `PermissionError`-virheeseen:

1. Sulje Arduino IDE.
2. Irrota Wemos USB-kaapelista.
3. Varmista, ettei toinen ohjelma käytä samaa COM-porttia.
4. Liitä Wemos uudelleen.
5. Tarkista Windowsin Laitehallinnasta COM-portti.
6. Käynnistä Arduino IDE uudelleen.
7. Kokeile toista USB-porttia.
8. Tarkista USB-sarjaportin ajuri.
9. Kokeile toista USB-kaapelia.
10. Kokeile Arduino IDE:n upload-nopeudeksi esimerkiksi `115200`.
11. Jos ongelma jatkuu, tarkista ESP32-korttipaketin ja `esptool`-version toiminta.

### Tärkeä oppi

Jos käännös onnistuu:

```text
Sketch uses ...
Global variables use ...
```

mutta lataus epäonnistuu vasta:

```text
Connecting...
```

tai COM-portin avaamisen yhteydessä, ongelma ei välttämättä ole Arduino-koodissa.

Ajattele ongelmaa näin:

```text
Arduino-koodi
      │
      ▼
   Käännös
      │
      ├── ❌ Compile error
      │       → ongelma koodissa
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
              → ongelma voi olla
                latausympäristössä
```

Tämä on erityisen tärkeää aloittelijalle: **älä ala muuttamaan toimivaa Arduino-koodia vain siksi, että firmwarea ei saada ladattua ESP32:lle.**

---

# Tietoturva

## Älä julkaise tunnuksia GitHubiin

Lähdekoodissa ei saa olla oikeita:

* WiFi-salasanoja
* MQTT-salasanoja
* API-avaimia
* muita salaisia tunnuksia

Tämän projektin GitHub-versioissa ei ole tarkoituksella mukana käyttäjän todellisia WiFi- tai MQTT-tunnuksia.

---

## GitHub-historia

Jos oikea salasana on joskus päätynyt GitHubiin, sen poistaminen nykyisestä tiedostosta ei välttämättä riitä.

Salaisuus voi edelleen löytyä Git-historiasta.

Jos oikea salasana julkaistaan vahingossa:

1. vaihda salasana välittömästi
2. poista salaisuus nykyisestä koodista
3. tarkista commit-historia
4. poista salaisuus tarvittaessa myös Git-historiasta

---

## V6:n AP-salasana

V6 käyttää laitekohtaista AP-salasanaa, joka muodostetaan MAC-osoitteen perusteella.

Tämä estää kaikille laitteille yhteisen oletussalasanan käytön.

AP-salasana ei kuitenkaan ole kryptografisesti satunnainen.

Siksi sitä ei pidä pitää yhtä vahvana salaisuutena kuin satunnaisesti luotua salasanaa.

---

## Paikallinen HTTP-asetusportaali

WiFi-asetusportaali käyttää ESP32:n HTTP-palvelinta.

Asetusportaalin liikenne ei ole HTTPS-salattua.

Tämä on pienessä paikallisessa ESP32-asetusportaalissa tarkoituksellinen kompromissi.

Asetusportaalia ei tule altistaa suoraan Internetiin.

---

# Asennus

## 1. Asenna Arduino IDE

Asenna Arduino IDE ja ESP32-korttipaketti.

## 2. Valitse ESP32-kortti

Wemos D1 R32:n kanssa voidaan käyttää esimerkiksi:

```text
ESP32 Dev Module
```

## 3. Asenna kirjastot

Arduino IDE:n Library Managerista:

```text
Adafruit SCD30
PubSubClient
```

## 4. Kytke SCD30

Liitä SCD30 ESP32:n I²C-väylään.

## 5. Valitse ohjelmaversio

Opiskeluun:

```text
examples/SCD30_WLAN_MQTT_v5/
```

Normaaliin käyttöön:

```text
examples/SCD30_WLAN_MQTT_v6/
```

## 6. Lataa firmware

Käännä ohjelma ja lataa se Wemos D1 R32:lle.

Jos lataus epäonnistuu `PermissionError`-virheeseen, katso [Firmwareen lataaminen ja PermissionError](#firmwareen-lataaminen-ja-permissionerror).

---

# Käyttöönotto

Ensimmäisellä käynnistyksellä ESP32 käynnistää WiFi-asetusportaalin, jos toimivaa WiFi-asetusta ei ole tallennettu.

### 1. Etsi ESP32:n WiFi-verkko

Etsi tietokoneella tai puhelimella ESP32:n muodostama verkko.

### 2. Yhdistä verkkoon

Yhdistä ESP32:n AP-verkkoon.

### 3. Avaa asetussivu

Avaa selaimella ESP32:n asetussivu.

### 4. Syötä asetukset

Anna:

```text
WiFi SSID
WiFi password

MQTT server
MQTT port
MQTT username
MQTT password
MQTT topic
```

### 5. Tallenna

ESP32 tallentaa asetukset Preferences/NVS-muistiin ja käynnistyy uudelleen.

Tämän jälkeen ESP32 yrittää muodostaa yhteyden WiFi-verkkoon ja MQTT-brokeriin.

---

# Oppimispolku

Projektia voi käyttää ESP32-, WiFi-, MQTT- ja tietoturvaominaisuuksien opiskeluun.

Suositeltu etenemisjärjestys:

## 1. Aloita V5:stä

Tutustu:

* `setup()`
* `loop()`
* WiFi-yhteyteen
* WebServeriin
* Preferences-muistiin
* MQTT-yhteyteen
* SCD30:n lukemiseen

## 2. Testaa MQTT

Varmista, että SCD30:n mittaustiedot näkyvät MQTT-brokerissa.

## 3. Tutki V5:n rajoituksia

Erityisesti:

```cpp
setInsecure();
```

ja salasanojen käsittelyä.

## 4. Siirry V6:een

Vertaa V5:n ja V6:n toteutuksia.

Tutki erityisesti:

* TLS-varmenteen tarkistusta
* NTP-aikaa
* asetustoimintojen tokenia
* HTML escapingia
* syötteiden validointia
* MQTT Client ID:tä
* HTTP security headereita

## 5. Opettele myös firmwareen lataamisen ongelmat

Jos ohjelma ei siirry ESP32:lle, opettele erottamaan:

```text
Käännösongelma
      ≠
Firmwareen lataamisen ongelma
```

`PermissionError(13)` voi liittyä Windowsin COM-porttiin tai `esptool`-ympäristöön eikä itse Arduino-koodiin.

## 6. Kehitä oma V7

Mahdollisia jatkokehityskohteita:

* satunnaisesti luotu AP-salasana
* AP-salasanan turvallisempi tallennus
* OTA-päivitys
* MQTT Last Will
* laitteen online/offline-tila
* parempi virheenkäsittely
* sensorin tilan valvonta
* Web-käyttöliittymän kehittäminen
* MQTT:n uudelleenyhdistämisen parantaminen

---

# V5 vai V6?

## Valitse V5, jos:

* opettelet ESP32-ohjelmointia
* haluat mahdollisimman helposti seurattavan koodin
* opettelet WiFi-yhteyksiä
* opettelet MQTT:tä
* haluat nähdä tietoturvan kehityksen vaiheittain

## Valitse V6, jos:

* laite tulee oikeaan käyttöön
* haluat TLS-varmenteen tarkistuksen
* haluat paremmin suojatun asetussivun
* haluat yksilöllisen MQTT Client ID:n
* haluat HTML escapingin
* haluat syötteiden validointia
* haluat HTTP security headerit

**Suositus: käytä V6:ta varsinaisessa käytössä.**

---

# Tunnetut rajoitukset

V6 on huomattavasti V5:tä turvallisempi, mutta sitä ei ole suunniteltu Internetiin suoraan altistettavaksi Web-palvelimeksi.

Rajoituksia ovat muun muassa:

* WiFi-asetusportaali käyttää HTTP:tä eikä HTTPS:ää.
* AP-salasana perustuu MAC-osoitteeseen.
* ESP32:n rajalliset resurssit rajoittavat Web-palvelimen ominaisuuksia.
* MQTT TLS suojaa MQTT-yhteyden, mutta ei paikallista HTTP-asetusliikennettä.
* Laitetta ei tule asettaa suoraan Internetiin ilman erillistä suojausta.

V6 on tarkoitettu ensisijaisesti **luotettuun paikalliseen IoT-verkkoon**.

---

# Tuleva V7

V6:n jälkeen seuraava luonnollinen kehitysversio voisi olla V7.

Yksi tärkeimmistä parannuksista olisi AP-salasanan muuttaminen aidosti satunnaisesti generoiduksi salasanaksi.

Esimerkiksi:

```text
SSID:
SCD30-Setup-A1B2C3

Password:
K7#mP9xQ2$vL
```

Salasana voitaisiin luoda ensimmäisellä käynnistyskerralla ESP32:n satunnaislukugeneraattorilla ja tallentaa Preferences-muistiin.

Tällöin AP-salasanaa ei voisi päätellä laitteen MAC-osoitteesta.

---

# Lisenssi

Tämä projekti on julkaistu [MIT-lisenssillä](LICENSE).

Copyright © 2026 Tero Leinonen.
