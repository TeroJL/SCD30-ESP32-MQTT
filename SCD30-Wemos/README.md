# SCD30 Wemos D1 R32 – WLAN, MQTT ja WiFi-asetusportaali

ESP32 / Wemos D1 R32 -projekti Sensirion SCD30 -anturille.

Projekti sisältää kaksi versiota samasta perusratkaisusta:

- **V5 Basic** – yksinkertaisempi ja aloittelijalle helpompi versio.
- **V6 Secure** – sama perusidea, mutta tietoturvaa on parannettu.

## Projektin rakenne

```text
SCD30-Wemos/
├── README.md
├── docs/
│   ├── version-5.md
│   └── version-6.md
├── examples/
│   ├── SCD30_WLAN_MQTT_v5/
│   │   └── SCD30_WLAN_MQTT_v5.ino
│   └── SCD30_WLAN_MQTT_v6/
│       └── SCD30_WLAN_MQTT_v6.ino
├── .gitignore
└── LICENSE
```

## Laitteisto

- Wemos D1 R32 / ESP32
- Sensirion SCD30
- WiFi-yhteys
- MQTT-brokeri, esimerkiksi HiveMQ Cloud

## V5 – Basic

V5 on tarkoitettu erityisesti opiskeluun ja ESP32:n perusasioiden harjoitteluun.

Se näyttää selkeästi, miten:

- WiFi-yhteys muodostetaan
- ESP32 käynnistää oman WiFi-asetusportaalin
- asetukset tallennetaan Preferences/NVS-muistiin
- WebServer käsittelee lomakkeita
- SCD30:n mittaukset luetaan
- MQTT-yhteys muodostetaan
- mittaustiedot julkaistaan MQTT:llä

### Tietoturvahuomio

V5 ei ole tämän projektin turvallisin versio. Se on tarkoituksella yksinkertaisempi oppimisversio.

Erityisesti MQTT TLS -yhteyden sertifikaatin tarkistus ei ole samalla tasolla kuin V6:ssa.

Älä käytä V5:tä sellaisenaan ympäristössä, jossa tietoturva on kriittinen.

## V6 – Secure

V6 säilyttää V5:n perusidean, mutta lisää muun muassa:

- MQTT TLS -varmenteen tarkistuksen
- NTP-ajan synkronoinnin TLS:ää varten
- vahvemman, laitekohtaisen AP-salasanan
- asetusportaalin rajoittamisen AP-tilaan
- istuntotunnisteen asetustoiminnoille
- salasanojen jättämisen pois HTML-lähteestä
- HTML-escapingin
- yksilöllisen MQTT Client ID:n
- syötteiden validointia
- HTTP-suojausheaderit
- `Cache-Control: no-store` -asetuksen

V6 on suositeltu versio normaaliin käyttöön.

## Asennus

1. Asenna Arduino IDE.
2. Valitse ESP32-kortiksi Wemos D1 R32 / vastaava ESP32 Dev Module.
3. Asenna tarvittavat kirjastot.
4. Avaa haluttu `.ino`-tiedosto.
5. Muuta omat MQTT-asetukset laitteen asetussivulla.
6. Käännä ja lataa ohjelma ESP32:lle.

## MQTT

Tyypillinen MQTT-yhteys käyttää TLS:ää portissa `8883`.

V6:n TLS-varmennetta varten ohjelmassa on luotettu CA-varmenne. Jos käytät muuta MQTT-palvelua kuin tässä projektissa tarkoitettua palvelua, tarkista sen sertifikaattiketju ja vaihda tarvittaessa CA-varmenne.

## Tunnukset

**Älä koskaan lisää oikeita WiFi- tai MQTT-salasanoja GitHubiin.**

V5:n oletusasetuksissa oleva AP-salasana `asetukset` on vain oppimisesimerkki. Vaihda se ennen oikeaa käyttöä.

V6 muodostaa AP-salasanan laitekohtaisesti.

## V5 vs V6

| Ominaisuus | V5 Basic | V6 Secure |
|---|---:|---:|
| WiFi-asetusportaali | ✓ | ✓ |
| Preferences/NVS | ✓ | ✓ |
| MQTT | ✓ | ✓ |
| MQTT TLS | ✓ | ✓ |
| TLS-varmenteen tarkistus | – | ✓ |
| NTP-aika TLS:ää varten | – | ✓ |
| Laitekohtainen AP-salasana | – | ✓ |
| Asetusportaalin rajoitus AP-tilaan | – | ✓ |
| Asetustoimintojen token | – | ✓ |
| HTML escaping | – | ✓ |
| Salasanojen piilotus HTML:stä | – | ✓ |
| Yksilöllinen MQTT Client ID | – | ✓ |
| Syötteiden validointi | – | ✓ |
| HTTP security headers | – | ✓ |

## Oppimispolku

Suositeltu etenemisjärjestys:

1. Aloita V5:stä ja ymmärrä sen toiminta.
2. Testaa SCD30:n mittausten lähetys MQTT:llä.
3. Tutustu Preferences-, WebServer- ja MQTT-koodiin.
4. Siirry V6:een.
5. Vertaa V5:n ja V6:n toteutuksia.
6. Tutki erityisesti TLS-varmenteen tarkistusta ja asetussivun suojausta.

Tällä tavalla V6 toimii käytännössä V5:n tietoturvallisempana jatkokehityksenä.
