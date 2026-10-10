#include <Arduino.h>
#include "WiFi.h"
#include "esp_wifi.h"

// Havada yakalanan her paket için tetiklenen fonksiyon
void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
  // Sadece yönetim (Management) paketleriyle ilgileniyoruz
  if (type == WIFI_PKT_MGMT) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    uint8_t *frame = pkt->payload;
    
    // Çerçevenin ilk byte'ı paketin türünü belirler.
    // 0xA0 (Deauthentication) veya 0xC0 (Disassociation) paketlerini arıyoruz.
    uint8_t frame_type = frame[0];
    
    if (frame_type == 0xA0 || frame_type == 0xC0) {
      // Hedef cihazın MAC adresi paketin 4. byte'ından başlar (Address 1)
      Serial.printf("[DİKKAT!] Deauth Paketi Yakalandı! Hedef MAC: %02X:%02X:%02X:%02X:%02X:%02X | Kanal: %02d | Sinyal: %d dBm\n", 
                    frame[4], frame[5], frame[6], frame[7], frame[8], frame[9],
                    pkt->rx_ctrl.channel, pkt->rx_ctrl.rssi);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
  
  Serial.println("==================================================");
  Serial.println("Deauth Dedektörü Aktif. Ağ güvenliği izleniyor...");
  Serial.println("==================================================");
}

void loop() {
  // Wi-Fi kanallarını sürekli taramaya devam ediyoruz
  for (int i = 1; i <= 13; i++) {
    esp_wifi_set_channel(i, WIFI_SECOND_CHAN_NONE);
    // Saldırı paketlerini kaçırmamak için kanalda biraz daha kısa kalıyoruz (200ms)
    delay(200); 
  }
}