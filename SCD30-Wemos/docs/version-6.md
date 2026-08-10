# V6 – Secure

V6 on V5:n tietoturvallisempi jatkokehitys.

## Keskeiset muutokset

### TLS-varmenteen tarkistus

V5:n `setInsecure()`-ratkaisu on poistettu.

V6 käyttää CA-varmennetta, jolloin MQTT-palvelimen TLS-varmenne tarkistetaan.

### AP-tila

Asetusportaali on tarkoitettu käytettäväksi laitteen omassa AP-verkossa.

Normaalissa STA/WiFi-tilassa asetusten muuttamiseen tarkoitetut endpointit eivät ole käytettävissä samalla tavalla.

### Salasanat

WiFi- ja MQTT-salasanoja ei tulosteta Serial Monitoriin eikä palauteta salasanoina HTML-lomakkeeseen.

Tyhjä salasana-asetuskenttä tarkoittaa, että nykyinen salasana säilytetään.

### MQTT Client ID

Client ID muodostetaan laitteen MAC-osoitteen perusteella, jotta useat ESP32-laitteet eivät käytä samaa Client ID:tä.

### Syötteet

HTTP-lomakkeen syötteille tehdään pituus- ja sisältötarkistuksia, ja HTML:ään tulostettavat arvot escapetaan.

## Huomio CA-varmenteesta

Jos MQTT-brokeri vaihtuu, tarkista sen käyttämä TLS-varmenneketju. V6:n CA-varmenne on valittu tämän projektin MQTT-käyttöä varten.
