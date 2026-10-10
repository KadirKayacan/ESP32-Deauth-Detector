# ESP32 Deauth Detector

## Genel Bakış

Bu proje, ESP32 mikrodenetleyicisi üzerinde çalışan, C++ (Arduino framework + ESP-IDF) ile geliştirilmiş bir **Deauth / Disassociation Paket Dedektörü** aracıdır. ESP32'nin Wi-Fi donanımını **promiscuous (karışık) mod** ve **raw packet sniffing** yetenekleriyle kullanarak, ortamdaki kablosuz ağ trafiğini izler ve **802.11 yönetim çerçeveleri** arasından **Deauthentication (0xA0)** ve **Disassociation (0xC0)** paketlerini tespit eder.

Bu tür paketler genellikle **Wi-Fi deauth saldırılarının** (kullanıcıyı ağdan düşürme, handshake yakalama, Evil Twin saldırısı hazırlığı) habercisidir. Araç, yakalanan her saldırı paketi için hedef MAC adresini, kanalı ve sinyal gücünü (RSSI) seri porttan anlık olarak raporlar.

Araç, 1–13 kanalları arasında sürekli **kanal hopping** yaparak tüm bantta dinleme yapar. Bu proje; **kablosuz ağ güvenliği farkındalığı**, **802.11 çerçeve yapısı öğrenimi**, **deauth saldırılarının tespiti** ve **yetkili güvenlik izleme** amacıyla geliştirilmiştir. Kendi ağınızı izlemek, saldırı girişimlerini fark etmek ve bir IDS (Intrusion Detection System) mantığını anlamak için eğitim amaçlı bir laboratuvar aracıdır.

## Özellikler

- **Promiscuous (karışık) mod** ile ortamdaki tüm Wi-Fi paketlerini yakalama
- **Sadece yönetim çerçevelerini** (`WIFI_PKT_MGMT`) filtreleyerek işleme
- **Deauthentication (0xA0)** ve **Disassociation (0xC0)** paketlerini tespit etme
- Yakalanan her saldırı paketi için **hedef MAC adresi** (Address 1) çıktısı
- Paketin **kanal bilgisi** (`rx_ctrl.channel`) ve **sinyal gücü** (`rx_ctrl.rssi`) raporlama
- **Otomatik kanal hopping**: 1 → 13 kanalları arasında 200 ms'de bir geçiş
- Seri port (115200 baud) üzerinden anlık uyarı çıktısı
- ESP32'yi **Station (STA) moduna** alıp önceki bağlantıları temizleyerek temiz bir dinleme ortamı sağlama

## Teknolojiler

- C++ (Arduino framework + ESP-IDF)
- ESP32 (ESP-WROOM-32 veya uyumlu kartlar)
- Arduino IDE / PlatformIO
- Kütüphaneler:
  - `Arduino.h` (temel Arduino fonksiyonları)
  - `WiFi.h` (ESP32 dahili Wi-Fi kütüphanesi)
  - `esp_wifi.h` (ESP-IDF Wi-Fi sürücüsü — promiscuous mod ve raw sniffing için)

## Kurulum

```bash
git clone https://github.com/KadirKayacan/ESP32-Deauth-Detector.git
cd ESP32-Deauth-Detector

# PlatformIO kullanıyorsan:
pio run --target upload

# Arduino IDE kullanıyorsan:
# 1. Arduino IDE'yi aç
# 2. Board Manager'dan "esp32" paketini kur (Espressif Systems)
# 3. Karta "ESP32 Dev Module" seç
# 4. main.cpp dosyasını aç ve yükle
```

> Not: `esp_wifi_set_promiscuous()` ve `esp_wifi_set_promiscuous_rx_cb()` fonksiyonları ESP-IDF'in düşük seviyeli Wi-Fi API'sidir ve ESP32 Arduino çekirdeği ile birlikte gelir. Harici bir kütüphane kurulumu gerekmez. Ancak **promiscuous mode** kullanabilmek için ESP32 çekirdeğinin güncel bir sürümü (2.0.x veya üzeri) önerilir.

## Kullanım

```bash
# PlatformIO ile derleme ve yükleme
pio run --target upload

# Seri monitörü aç (115200 baud)
pio device monitor -b 115200
```

Cihaz açıldıktan sonra herhangi bir komut girmeye gerek yoktur; kod otomatik olarak dinlemeye başlar. Seri porttan aşağıdaki gibi bir çıktı görürsünüz:

```text
==================================================
Deauth Dedektörü Aktif. Ağ güvenliği izleniyor...
==================================================

[DİKKAT!] Deauth Paketi Yakalandı! Hedef MAC: FF:FF:FF:FF:FF:FF | Kanal: 06 | Sinyal: -42 dBm
[DİKKAT!] Deauth Paketi Yakalandı! Hedef MAC: A4:5E:60:C1:2D:8B | Kanal: 06 | Sinyal: -55 dBm
[DİKKAT!] Deauth Paketi Yakalandı! Hedef MAC: A4:5E:60:C1:2D:8B | Kanal: 06 | Sinyal: -57 dBm
[DİKKAT!] Deauth Paketi Yakalandı! Hedef MAC: 3C:22:FB:11:AA:04 | Kanal: 11 | Sinyal: -68 dBm
```

> **Not:** `FF:FF:FF:FF:FF:FF` hedef MAC adresi, paketin **broadcast** (tüm cihazlara) gönderildiğini gösterir. Bu, tipik bir deauth saldırısının en yaygın biçimidir. Tek bir hedefe yönelik saldırılarda hedef MAC adresi belirli bir cihaza ait olur.

### Test Etme (Yetkili Ortamda)

Kendi ağınızda deauth dedektörünü test etmek için (yalnızca sahibi olduğunuz ağda ve izinli cihazlarda):

1. ESP32'yi yukarıdaki kodla yükleyin ve seri monitörü açın.
2. Yetkili bir test cihazından (ör. `aireplay-ng --deauth` komutuyla kendi ağınıza) deauth paketi gönderin.
3. ESP32'nin seri portunda `[DİKKAT!]` uyarısının göründüğünü doğrulayın.

> **Uyarı:** Başkalarına ait ağlara deauth paketi göndermek **yasadışıdır**. Bu test yalnızca kendi ağınızda yapılmalıdır.

### Kanalları Sınırlama

Belirli bir kanalı sürekli izlemek isterseniz `loop()` fonksiyonundaki `for` döngüsünü düzenleyebilirsiniz:

```cpp
void loop() {
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE); // Sadece 6. kanalı izle
  delay(1000);
}
```

## Proje Yapısı

```text
ESP32-Deauth-Detector/
├── src/
│   └── main.cpp          # Ana uygulama (sniffer callback + kanal hopping)
├── platformio.ini        # PlatformIO yapılandırması
├── README.md
├── .gitignore
```

Arduino IDE kullanıyorsan dosya yapısı şu şekilde olur:

```text
ESP32-Deauth-Detector/
├── esp32_deauth_detector.ino
├── README.md
├── .gitignore
```

## Teknik Detaylar

### Deauth / Disassociation Çerçevesi Yapısı

Yakalanan paket, IEEE 802.11 standardına uygun bir **Management Frame** yapısına sahiptir. İlk byte (Frame Control) paketin türünü belirler:

| Frame Control Değeri | Tür | Açıklama |
|----------------------|-----|----------|
| `0x80` | Beacon | AP tarafından yayınlanır |
| `0xA0` | **Deauthentication** | Bağlantıyı zorla koparma |
| `0xB0` | Authentication | Kimlik doğrulama |
| `0xC0` | **Disassociation** | İlişkiyi zorla sonlandırma |

### MAC Adresi Konumu

802.11 çerçevesinde MAC adresleri şu konumlardadır:

| Alan | Byte Aralığı | Açıklama |
|------|--------------|----------|
| Frame Control | 0–1 | Paket türü |
| Duration | 2–3 | Süre |
| **Address 1 (Hedef)** | **4–9** | Alıcı MAC (bizim kodda `frame[4..9]`) |
| Address 2 (Kaynak) | 10–15 | Gönderen MAC |
| Address 3 (BSSID) | 16–21 | AP MAC |

Kod, `frame[4]`–`frame[9]` aralığını okuyarak **hedef cihazın MAC adresini** çıkarır.

### Kanal Hopping

`loop()` fonksiyonu, 1–13 kanalları arasında 200 ms'de bir geçiş yapar:

```cpp
for (int i = 1; i <= 13; i++) {
  esp_wifi_set_channel(i, WIFI_SECOND_CHAN_NONE);
  delay(200);
}
```

200 ms, bir kanalda deauth paketini kaçırmamak için dengeli bir süredir. Daha kısa süre → daha hızlı tarama ama paket kaçırma riski; daha uzun süre → daha güvenilir yakalama ama yavaş tarama.

### Sniffer Callback

`sniffer_callback` fonksiyonu, promiscuous modda yakalanan **her paket** için çağrılır. `type == WIFI_PKT_MGMT` filtresi sayesinde yalnızca yönetim çerçeveleri işlenir; veri paketleri (data frames) yok sayılır. Bu, işlemci yükünü azaltır ve yalnızca güvenlik açısından önemli paketlere odaklanır.

## Sınırlamalar

- **Tek kanal dinleme**: ESP32 aynı anda yalnızca bir kanalı dinleyebilir; bu nedenle kanal hopping sırasında bazı paketler kaçırılabilir.
- **Şifreli yönetim çerçeveleri**: WPA3 ve 802.11w (PMF — Protected Management Frames) etkin ağlarda deauth/disassoc paketleri şifrelenir ve içerikleri okunamaz (ancak yine de tespit edilebilir).
- **RSSI doğruluğu**: RSSI değeri ortam gürültüsüne ve anten kalitesine göre değişir; kesin mesafe ölçümü için kullanılamaz.
- **Broadcast tespiti**: Broadcast deauth paketleri (`FF:FF:FF:FF:FF:FF`) tek bir hedef göstermez; saldırının tüm ağa yönelik olduğunu belirtir.

## Geliştirme Fikirleri

- **SD kart / SPIFFS loglama**: Yakalanan saldırı paketlerini zaman damgasıyla kaydetme.
- **OLED ekran**: Anlık uyarıları ve istatistikleri küçük bir ekranda gösterme.
- **Web arayüzü**: ESP32 dahili HTTP sunucusu ile uzaktan izleme.
- **Telegram / MQTT bildirimi**: Wi-Fi üzerinden bağlanıp saldırı anında bildirim gönderme.
- **Beyaz liste (whitelist)**: Kendi ağınızın BSSID'sini filtreleyerek yalnızca kendi ağınıza yönelik saldırıları raporlama.
- **Sayaç / istatistik**: Belirli bir MAC'ten gelen deauth paketlerini sayarak saldırı yoğunluğunu ölçme.

