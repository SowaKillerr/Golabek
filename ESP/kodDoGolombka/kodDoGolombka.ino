#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Base64.h>

const char* ssid = "Internet_Domowy_88749A";            // Nazwa sieci WiFi
const char* password = "AF345AE5";           // Hasło do sieci WiFi
const char* token = "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";
const char* repoUrl = "https://sowakillerr.github.io/Golabek/messages.json";

bool messageSent = false;

// Funkcja kodowania wiadomości do Base64
String encodeBase64(const String& input) {
    String encodedStr = base64::encode(input);  // Używamy funkcji base64 z biblioteki Base64.h
    return encodedStr;
}

// Funkcja wysyłania wiadomości
void sendMessage(const String& sender, const String& message) {
    Serial.println("Wysyłanie wiadomości...");

    // Przygotowanie zapytania HTTP
    HTTPClient http;
    http.begin(repoUrl);
    http.addHeader("Authorization", "Bearer " + String(token));
    http.addHeader("Content-Type", "application/json");

    // Treść wiadomości
    String json = "{\"sender\": \"" + sender + "\", \"message\": \"" + message + "\"}";

    // Zakodowanie treści w Base64 (wymagane przez GitHub API)
    String encodedMessage = encodeBase64(json);

    // Payload
    String payload = "{\"message\": \"Wysłano wiadomość\", \"content\": \"" + encodedMessage + "\", \"branch\": \"main\"}";

    int httpResponseCode = http.PUT(payload);

    if (httpResponseCode > 0) {
        Serial.println("Wiadomość wysłana! Kod odpowiedzi: " + String(httpResponseCode));
    } else {
        Serial.println("Błąd wysyłania wiadomości: " + String(httpResponseCode));
    }

    http.end();
    messageSent = true;
}

// Funkcja odbierania wiadomości
void receiveMessage() {
    Serial.println("Nasłuchiwanie wiadomości...");

    HTTPClient http;
    http.begin(repoUrl);
    http.addHeader("Authorization", "Bearer " + String(token));

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Otrzymano odpowiedź z serwera:");
        Serial.println(response);

        // Parsowanie JSON
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response);

        if (error) {
            Serial.println("Błąd parsowania JSON!");
            return;
        }

        // Sprawdzanie zawartości wiadomości
        String sender = doc["sender"];
        String message = doc["message"];

        if (message != "") {
            Serial.println("Nowa wiadomość od: " + sender);
            Serial.println("Treść: " + message);

            // Usuwanie wiadomości po odczytaniu
            sendMessage("", "");
        }
    } else {
        Serial.println("Błąd odbierania danych: " + String(httpResponseCode));
    }

    http.end();
}

void setup() {
    Serial.begin(115200);

    // Połączenie z WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Łączenie z WiFi...");
    }
    Serial.println("Połączono z WiFi!");
}

void loop() {
    // Odczyt danych z Serial Monitora
    if (Serial.available() > 0) {
        String message = Serial.readStringUntil('\n');
        message.trim(); // Usuń białe znaki (np. nową linię)

        if (message.length() > 0) {
            sendMessage("ESP1", message); // Wyślij wiadomość
        }
    }

    // Nasłuchiwanie, gdy nic nie wysyłamy
    if (!messageSent) {
        receiveMessage();
        delay(10000); // Odbieranie co 10 sekund
    }
}
