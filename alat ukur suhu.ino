#include "WiFi.h"
#include <HTTPClient.h>
#include "DHT.h"

// Definisi PIN untuk setiap sensor DHT11
#define DHTPIN_1  4
#define DHTPIN_2  18
#define DHTPIN_3  19
#define DHTPIN_4  21

#define DHTTYPE DHT11

// Informasi WiFi
const char* ssid = "CPH1701";        // Nama WiFi Anda
const char* password = "123456789";  // Password WiFi Anda

// URL Google Script Web App
String Web_App_URL = "https://script.google.com/macros/s/AKfycbz232eCmQUlm7LE4KZjOuooeFH8OQ_BytxlOtb9ndgxwhTmhT5DF-TzGesppdbtYpt_/exec";

// Variabel untuk menyimpan status pembacaan sensor dan data
String Status_Read_Sensor = "";
float Temp[4];
int Humd[4];

// Inisialisasi DHT
DHT dht1(DHTPIN_1, DHTTYPE);
DHT dht2(DHTPIN_2, DHTTYPE);
DHT dht3(DHTPIN_3, DHTTYPE);
DHT dht4(DHTPIN_4, DHTTYPE);

// Fungsi untuk membaca data dari DHT11
void Getting_DHT11_Sensor_Data() {
  // Sensor 1
  Humd[0] = dht1.readHumidity();
  Temp[0] = dht1.readTemperature();

  // Sensor 2
  Humd[1] = dht2.readHumidity();r
  Temp[1] = dht2.readTemperature();

  // Sensor 3
  Humd[2] = dht3.readHumidity();
  Temp[2] = dht3.readTemperature();

  // Sensor 4
  Humd[3] = dht4.readHumidity();
  Temp[3] = dht4.readTemperature();

  Status_Read_Sensor = "Success";

  for (int i = 0; i < 4; i++) {
    if (isnan(Humd[i]) || isnan(Temp[i])) {
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.println(": Gagal membaca data!");
      Status_Read_Sensor = "Failed";
      Temp[i] = 0.0;
      Humd[i] = 0;
    } else {
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.print(" | Kelembapan: ");
      Serial.print(Humd[i]);
      Serial.print("% | Suhu: ");
      Serial.print(Temp[i]);
      Serial.println("°C");
    }
  }
  Serial.println("-------------");
}

// Fungsi setup
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Memulai NodeMCU 32-S...");
  Serial.println("Menghubungkan ke WiFi...");

  // Mode WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Proses koneksi ke WiFi
  int timeout = 20 * 2; // 20 detik
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (--timeout <= 0) {
      Serial.println("\nGagal menghubungkan ke WiFi. Restart...");
      ESP.restart();
    }
  }
  Serial.println("\nWiFi terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Inisialisasi sensor DHT11
  Serial.println("Memulai sensor DHT11...");
  dht1.begin();
  dht2.begin();
  dht3.begin();
  dht4.begin();
  Serial.println("Inisialisasi selesai!");
}

// Fungsi loop
void loop() {
  Serial.println();
  Serial.println("Membaca data dari sensor DHT11...");
  // Membaca data dari sensor DHT11
  Getting_DHT11_Sensor_Data();

  // Mengirim data ke Google Sheets jika WiFi terhubung
  if (WiFi.status() == WL_CONNECTED) {
    String Send_Data_URL = Web_App_URL + "?sts=write";
    Send_Data_URL += "&srs=" + Status_Read_Sensor;
    Send_Data_URL += "&temp1=" + String(Temp[0]);
    Send_Data_URL += "&humd1=" + String(Humd[0]);
    Send_Data_URL += "&temp2=" + String(Temp[1]);
    Send_Data_URL += "&humd2=" + String(Humd[1]);
    Send_Data_URL += "&temp3=" + String(Temp[2]);
    Send_Data_URL += "&humd3=" + String(Humd[2]);
    Send_Data_URL += "&temp4=" + String(Temp[3]);
    Send_Data_URL += "&humd4=" + String(Humd[3]);

    Serial.println("Mengirim data ke Google Spreadsheet...");
    Serial.print("URL: ");
    Serial.println(Send_Data_URL);

    // Kirim data menggunakan HTTPClient
    HTTPClient http;
    http.begin(Send_Data_URL.c_str());
    int httpCode = http.GET();

    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Respons: " + payload);
    } else {
      Serial.println("Gagal mengirim data!");
    }

    http.end();
  } else {
    Serial.println("WiFi tidak terhubung. Data tidak terkirim.");
  }

  Serial.println("Menunggu 10 detik sebelum membaca ulang...");
  delay(5000); // Delay 1 detik sebelum pengulangan berikutnya
}
