#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>

const char* ssid = "Internet_Domowy_88749A";            // Nazwa sieci WiFi
const char* password = "AF345AE5";           // Hasło do sieci WiFi
const char* githubToken  = "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";


// Adres serwera z plikiem JSON
const char* serverUrl = "https://sowakillerr.github.io/Golabek/message.json";

// Unikalna nazwa urządzenia (ESP1, ESP2)
const char* DEVICE_NAME = "ESP2"; // Zmień na "ESP2" dla drugiego ESP

// Ostatnia wiadomość odebrana z serwera
String lastMessage = "";

String encodeBase64(const String& data) {
  // Kodowanie danych na Base64
  String encodedData = base64::encode((const uint8_t*)data.c_str(), data.length());
  return encodedData;
}

void setup() {
  Serial.begin(115200);

  // Połączenie z WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Łączenie z WiFi...");
  }
  Serial.println("Połączono z WiFi");
}

void loop() {
  // Odbieranie wiadomości z serwera
  receiveMessage();

  // Sprawdzenie, czy jest nowa wiadomość do wysłania
  if (Serial.available() > 0) {
    String message = Serial.readString();
    sendMessage(DEVICE_NAME, message);
  }

  delay(5000); // Odczekaj 5 sekund przed kolejną operacją
}

// Funkcja do wysyłania wiadomości na serwer
void sendMessage(const char* sender, const String& message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Adres API GitHub
    const char* apiUrl = "https://api.github.com/repos/<username>/<repo>/contents/Golabek/message.json";

    // Inicjalizacja HTTP
    http.begin(apiUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(githubToken));

    // Tworzenie JSON i kodowanie do Base64
    String jsonData = "{\"sender\": \"" + String(sender) + "\", \"message\": \"" + message + "\"}";
    String contentBase64 = encodeBase64(jsonData);
    String body = "{\"message\": \"Update message.json\", \"content\": \"" + contentBase64 + "\"}";

    // Wysłanie zapytania
    int httpResponseCode = http.PUT(body);

    if (httpResponseCode > 0) {
      Serial.println("Wiadomość wysłana. Kod HTTP: " + String(httpResponseCode));
    } else {
      Serial.println("Błąd wysyłania wiadomości. Kod: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Brak połączenia z WiFi!");
  }
}


// Funkcja do odbierania wiadomości z serwera
void receiveMessage() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);

    // Pobranie danych z serwera
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Odebrano dane: " + payload);

      // Parsowanie JSON
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.println("Błąd parsowania JSON");
        return;
      }

      String sender = doc["sender"];
      String message = doc["message"];

      // Wyświetlenie wiadomości, jeśli jest nowa
      if (message != lastMessage) {
        Serial.println("Nowa wiadomość od " + sender + ": " + message);
        lastMessage = message;
      }
    } else {
      Serial.println("Błąd pobierania danych. Kod: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Brak połączenia z WiFi!");
  }
}
