// --- Definicja Pinów ---
const byte pinPrzekaznikOtwieranie = 2; // Wyjście na cewkę przekaźnika (równolegle do przycisku w szafie)
const byte pinStartOtwieranie = 8;      // Przycisk sterujący Arduino (Start procesu)
const byte pinKrancowkaOtwarty = 7;     // Wejście z optoizolatora (Sygnał: Lampka świeci)

// --- Ustawienia Czasowe ---
const unsigned long maxCzasRuchu = 10000; // 10 sekund - jeśli dłużej, to awaria

// --- Maszyna Stanów ---
enum Stan { SPOCZYNEK, OTWIERANIE, ZAKONCZONO, BLAD };
Stan aktualnyStan = SPOCZYNEK;

unsigned long czasStartuRuchu = 0;

void setup() {
  pinMode(pinPrzekaznikOtwieranie, OUTPUT);
  
  // Konfiguracja wejść (INPUT_PULLUP oznacza: wciśnięty/aktywny = LOW/GND)
  pinMode(pinStartOtwieranie, INPUT_PULLUP);
  pinMode(pinKrancowkaOtwarty, INPUT_PULLUP); 

  Serial.begin(9600);
  Serial.println("System gotowy. Czekam na komende...");
}

void loop() {
  
  switch (aktualnyStan) {
    
    // --- STAN 1: Czekamy na rozkaz ---
    case SPOCZYNEK:
      // Jeśli wciśnięto przycisk startu ORAZ zawór nie jest już otwarty
      if (digitalRead(pinStartOtwieranie) == LOW) {
        // Sprawdzamy czy przypadkiem już nie jest otwarty
        if (digitalRead(pinKrancowkaOtwarty) == LOW) {
          Serial.println("Info: Zawor juz jest otwarty.");
        } else {
          // Rozpoczynamy procedurę
          aktualnyStan = OTWIERANIE;
          czasStartuRuchu = millis();
          digitalWrite(pinPrzekaznikOtwieranie, HIGH); // "Wciskamy" przycisk w szafie
          Serial.println("START: Trzymam przycisk otwierania...");
        }
        delay(200); // Mały anty-drgania styków
      }
      break;

    // --- STAN 2: Trzymamy przycisk i czekamy na lampkę ---
    case OTWIERANIE:
      // 1. Sprawdź czy lampka się zapaliła (Sygnał dotarł)
      if (digitalRead(pinKrancowkaOtwarty) == LOW) {
        // SUKCES!
        digitalWrite(pinPrzekaznikOtwieranie, LOW); // "Puszczamy" przycisk
        aktualnyStan = ZAKONCZONO;
        Serial.println("SUKCES: Lampka swieci. Puscilem przycisk.");
      }
      
      // 2. Sprawdź czy nie minął czas (Watchdog)
      if (millis() - czasStartuRuchu > maxCzasRuchu) {
        // AWARIA!
        digitalWrite(pinPrzekaznikOtwieranie, LOW); // Natychmiast puszczamy dla bezpieczeństwa
        aktualnyStan = BLAD;
        Serial.println("BLAD: Czas minal, a lampka sie nie zapalila!");
      }
      break;

    // --- STAN 3: Reset po sukcesie ---
    case ZAKONCZONO:
      // Tu można dodać logikę co dalej. Na razie wracamy do spoczynku po 1 sek.
      delay(1000);
      aktualnyStan = SPOCZYNEK;
      Serial.println("Gotowy do nastepnego cyklu.");
      break;

    // --- STAN 4: Obsługa błędu ---
    case BLAD:
      // Mruganie diodą, alarm itp.
      // Aby zresetować, trzeba np. wcisnąć start ponownie
      if (digitalRead(pinStartOtwieranie) == LOW) {
        aktualnyStan = SPOCZYNEK;
        Serial.println("Reset bledu.");
        delay(500);
      }
      break;
  }
}