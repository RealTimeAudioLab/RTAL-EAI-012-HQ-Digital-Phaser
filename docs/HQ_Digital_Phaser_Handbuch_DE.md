# HQ Digital Phaser

## Benutzer- und Technikhandbuch · Ausgabe Oktober 2026

**RealTimeAudioLab | Hardware, MIDI, Windows-Editor und Praxisworkshops**

**Dokumentationsstand:** Auf Grundlage des vom Nutzer bereitgestellten achtseitigen Firmware-Handbuchs, der MIDI-CC-Tabelle, zweier GitHub-READMEs und des bisherigen Editor-Entwicklungsverlaufs. Frühere Firmware-Freigabe P015 FIX5; neuere Editor-kompatible Firmware P016 FIX4b. Die Bezeichnung „HQ Digital Phaser“ ist neu, das MIDI- und SD-Datenformat bleibt unverändert.

## 1. Was ist der HQ Digital Phaser?

Der HQ Digital Phaser ist ein digitaler Stereo-Phaser auf ESP32-S3-Basis. Das analoge Eingangssignal wird über einen PCM1808 digitalisiert, durch eine Allpass-Phaser-Engine mit wahlweise sechs oder zwölf Stufen verarbeitet und über einen PCM5102A ausgegeben. Resonance, Drive und Analog-Parameter verändern den Charakter. LFO, Envelope Follower und eine zweifache Modulationsmatrix sorgen für Bewegung. Die Bedienung erfolgt am OLED/Encoder oder über DIN-MIDI und den Windows-Editor.

Das Gerät ist ein unabhängiges RealTimeAudioLab-Projekt. Der Bezug zum klassischen Moog MF-103 bezeichnet lediglich die Inspiration.

## 2. Sicherheit vor dem ersten Einschalten

Audioausgänge zunächst mit moderatem Pegel betreiben. Hohe Resonance-, Drive- und Modulationstiefen können Pegelspitzen erzeugen. Vor SD-Änderungen alle Preset- und Namensdateien sowie `MIDI.CFG` sichern. Vor einem vollständigen Bank-Restore sowohl die SD-Karte als auch einen PC-Bankordner sichern. Bei elektrischen Änderungen die Pinbelegung gegen den tatsächlich eingesetzten Schaltplan prüfen.

## 3. Hardware und Audioanschlüsse

| Baugruppe | Pinbelegung |
|---|---|
| ESP32-S3 + PCM1808 | ADC DIN 5, BCLK 18, LRCK 16, MCLK 0 |
| PCM5102A | DAC DOUT 17, BCLK 18, LRCK 16 |
| OLED SSD1309 128×64 | SPI SCK 12, MOSI 11, CS 10, DC 6, RST 7 |
| SD | SPI SCK 12, MOSI 11, MISO 13, CS 9 |
| Encoder | A 1, B 2, Taster 42 |
| DIN MIDI | RX 40, TX 39, 31250 Baud |

OLED und SD teilen sich den SPI-Bus, verwenden aber getrennte Chip-Select-Leitungen. Die ältere Firmwaredokumentation nennt 32 kHz Audio-Samplerate und 128 Frames je Block. Dies ist eine Eigenschaft des dokumentierten Firmwarestands, keine Zusicherung für beliebige zukünftige Builds.

## 4. Inbetriebnahme

1. Hardware anschließen, gemeinsame Masse und korrekte Versorgung sicherstellen.
2. SD-Karte vollständig sichern und Presetdateien unter `/RTAL_PHASER/` bereitstellen.
3. Arduino IDE 2 mit der passenden ESP32-S3-Unterstützung und U8g2 einrichten.
4. Den zum Gerät passenden Sketch öffnen und kompilieren; Ordner- und Sketchname müssen übereinstimmen.
5. Firmware flashen; seriellen Monitor bei 115200 Baud zur Diagnose verwenden.
6. Gerät einschalten, OLED-Startanzeige prüfen und einen bekannten Preset-Slot laden.
7. Eingangssignal einspeisen, Ausgänge verbinden, Output Mode kontrollieren und mit niedriger Resonanz beginnen.

**Versionshinweis:** Die alten Installationspfade in früheren GitHub-Dokumenten können nach der Repository-Umbenennung anders lauten. Den aktuellen Sketch-Pfad vor dem Flashen kontrollieren.

## 5. Bedienung am Gerät

| Zustand | Drehen | Kurzer Druck |
|---|---|---|
| HOME | SWEEP direkt ändern | Hauptmenü öffnen |
| MAIN MENU | Menü auswählen | Menü öffnen |
| PAGE | Parameter auswählen | Bearbeitung starten |
| EDIT | Wert ändern | Übernehmen |
| PRESET BROWSER | Slot 001–128 auswählen | LOAD/SAVE öffnen |
| PRESET ACTION | LOAD oder SAVE wählen | Ausführen |

**Langer Encoderdruck:** Rückkehr zur HOME-Seite. Bei schneller Drehung kann im EDIT-Modus eine Beschleunigung greifen. Beispiel: HOME → Encoder drücken → PHASER → RESONANCE → drehen → übernehmen.

## 6. PHASER-Parameter

**STAGES (6/12):** Sechs Stufen erzeugen eine übersichtlichere, häufig weichere Phasenbewegung. Zwölf Stufen ermöglichen komplexere Kerbstrukturen. **SWEEP:** Grundposition der Phasenkerben. **RESONANCE:** Rückkopplung und Intensität; hohe Werte können sehr ausgeprägt klingen. **DRIVE:** Sättigung/Klangfärbung; vorsichtig dosieren. **BYPASS:** Effektbearbeitung an/aus.

**ANALOG MODE:** Schaltet zusätzliche analoge Klangcharakteristik zu. **STAGE VARIATION:** Variiert die Stufen und verändert damit die Kerbstruktur. **RESONANCE CHARACTER:** Beeinflusst den Charakter der Rückkopplung. **ANALOG COLOR:** Zusätzliche Färbung.

**Praxis:** Erst STAGES=6, Resonance moderat und Drive niedrig einstellen. SWEEP langsam bewegen und die Veränderung hören. Dann STAGES=12 wählen, um die dichtere Struktur direkt zu vergleichen.

## 7. LFO und Stereo

**RANGE:** LOW/HIGH bestimmt den Frequenzbereich. **RATE:** Geschwindigkeit; die reale Frequenz hängt von RANGE und gegebenenfalls der Clock ab. Die ursprüngliche Dokumentation nennt insgesamt 0,01–250 Hz. **AMOUNT:** Stärke der LFO-Modulation. **SYNC:** FREE oder MIDI. **DIVISION:** Eine von elf rhythmischen Unterteilungen. **STEREO PHASE:** Phasenversatz zwischen links/rechts in 45°-Schritten.

**OUTPUT MODE:** MONO, MAIN/AUX oder STEREO. Die konkrete Signalführung hängt von der angeschlossenen Audiobeschaltung ab. Für einen breiten Stereo-Effekt STEREO wählen und eine hörbare Phasenverschiebung einstellen. Vorher sicherstellen, dass beide Ausgangskanäle korrekt angeschlossen sind.

## 8. Envelope Follower

Der Envelope Follower folgt der Dynamik des Eingangssignals. **ENV FOLLOW:** aktivieren. **ENV SENS:** Eingangsempfindlichkeit. **ENV ATTACK:** Ansprechzeit (historische Anzeige ungefähr 1–200 ms). **ENV RELEASE:** Abklingzeit (ungefähr 10–2000 ms). **ENV DEPTH:** positive oder negative Wirkung auf den Phaser. Negative Werte kehren die Bewegungsrichtung um.

**Praxis:** Kurze, perkussive Töne einspeisen. Mit moderater Sensitivity und positiver Depth beginnen. Attack verkürzen, wenn die Reaktion zu träge ist; Release verlängern, wenn der Effekt nach dem Anschlag zu schnell zurückfällt. Danach Depth negativ setzen und den umgekehrten Verlauf vergleichen.

## 9. Zweifache Modulationsmatrix

**MATRIX ON:** gesamte Matrix aktivieren. Für jede Route Quelle **LFO/ENVELOPE**, Ziel **SWEEP/RESONANCE/DRIVE** und bipolare **DEPTH** wählen. **MOD SMOOTH** glättet abrupte Bewegungen. Zwei Routen können unterschiedliche Ziele oder dasselbe Ziel modulieren.

| Kombination | Klangidee |
|---|---|
| LFO → SWEEP | Regelmäßige klassische Bewegung |
| LFO → RESONANCE | Pulsierende Intensität |
| LFO → DRIVE | Bewegte Sättigung |
| ENVELOPE → SWEEP | Anschlagabhängige Kerbverschiebung |
| ENVELOPE → RESONANCE | Dynamische Rückkopplung |
| ENVELOPE → DRIVE | Anschlagabhängige Färbung |

**Praxis:** Route 1 = LFO→SWEEP mit geringer positiver Tiefe; Route 2 = ENVELOPE→RESONANCE mit kleiner negativer Tiefe. MATRIX ON aktivieren und mit unterschiedlichen Eingangssignalen vergleichen. Bei unruhiger Bewegung MOD SMOOTH vorsichtig erhöhen.

## 10. Presets und SD-Karte

Der Browser bietet 128 Slots. **LOAD** lädt ein Preset, **SAVE (OVERWRITE)** überschreibt es. Auf der SD-Karte bleiben die technischen Dateinamen `PRESET01.DAT` bis `PRESET128.DAT` und `NAME01.TXT` bis `NAME128.TXT` im Ordner `/RTAL_PHASER/` erhalten. Das dokumentierte Preset-V5-Format umfasst 41 Bytes mit Prüfsumme. `MIDI.CFG` enthält MIDI-Konfiguration. Die Umbenennung des Produkts ist **kein** Grund, diese Dateien umzubenennen.

Bei `INVALID PRESET` Dateiformat, Prüfsumme, Werte und Dateipfad prüfen. Bei `PRESET EMPTY` ist für den Slot keine lesbare Datei vorhanden. Vor einer Soundbank-Aktualisierung ein komplettes Backup anlegen.

## 11. MIDI – Bankauswahl und Presetwechsel

DIN-MIDI läuft mit 31250 Baud. **CC32=0** wählt die Hauptparameter, **CC32=1** die Modulationsmatrix. Dieselbe CC-Nummer kann je nach Bank unterschiedliche Funktionen steuern. Program Change 0–127 entspricht Preset 001–128. MIDI Clock ist die Echtzeitnachricht F8; die genaue Wirkung von Start/Continue/Stop hängt vom Firmwarestand ab.

**Praxisbeispiel 1:** CC32=0, danach CC106 senden → Sweep ändern. **Praxisbeispiel 2:** CC32=1, danach CC110 senden → MOD1 DEPTH ändern. Für die Rückkehr zur Hauptbank CC32=0 senden. Die vollständige Tabelle steht in `MIDI_CC.md`.

## 12. HQ Digital Phaser Editor für Windows

Der Editor ermöglicht die komfortable Bearbeitung der Klangparameter, Klartextauswahl für diskrete Werte, physikalische Einheiten/Tooltips, Undo/Redo und A/B-Klangvergleich. Die **LIVE**-Funktion sendet Änderungen unmittelbar; alternativ überträgt **Zum Phaser senden** den kompletten Zustand. Der Editor kann einzelne SD-Presets laden/speichern und Preset-Namen bearbeiten.

**MIDI-Portauswahl:** MIDI IN/OUT auswählen und Verbinden klicken. Ab RC5 FIX1 werden die zuletzt erfolgreich verbundenen Ports gespeichert und beim nächsten Start vorausgewählt. Dies bedeutet **nicht**, dass beim Start automatisch verbunden wird. Der zuletzt korrigierte Startslot ist 1; nach ausdrücklichem Laden eines anderen Slots kann die Anzeige entsprechend wechseln.

**Undo/Redo:** Strg+Z und Strg+Y für Bearbeitungszustände. **A/B:** Zustand A speichern, Änderungen durchführen, B speichern und zwischen beiden vergleichen. Für einen tatsächlichen Hörvergleich jeweils **Zum Phaser senden** anklicken, falls die Umschaltung nicht bereits LIVE übertragen wurde.

## 13. PRO LIBRARIAN – zwei PC-Banken

Zwei unabhängige Banken mit jeweils 128 Presets können parallel geöffnet werden. Drag & Drop kopiert Presets; Shift + Drag & Drop verschiebt sie. Bankeinträge lassen sich umbenennen, kategorisieren und als Favoriten markieren. Der Preset-Vergleich stellt Parameter zweier Programme gegenüber und ermöglicht die Übernahme einzelner Werte. PC-Bankänderungen werden erst durch das Speichern des Bankordners dauerhaft.

**Praxis:** Bank A als Original öffnen, Bank B als Arbeitsbank. Einen Klang aus A nach B kopieren, im Vergleich nur SWEEP und RESONANCE anpassen und B in einem **neuen** Ordner speichern. Die Originalbank bleibt dadurch unangetastet.

## 14. MIDI-BACKUP und RESTORE

**Backup:** Im Editor die 128 Slots vom Phaser sichern; danach den PC-Bankordner speichern und überprüfen. **Einzel-Restore + Verify:** PC-Preset und SD-Zielslot auswählen, Überschreiben bestätigen, übertragen und Rücklesekontrolle abwarten. **Vollbank-Restore + Verify:** alle 128 Slots werden nacheinander geschrieben und zurückgelesen; die Anwendung verlangt eine ausdrückliche Sicherheitsbestätigung.

**Wichtig:** Ein Abbruch verhindert weitere Schreibvorgänge, stellt bereits überschriebene Slots aber nicht automatisch wieder her. Deshalb vor einem Vollbank-Restore ein vollständiges Backup anlegen und zusätzlich die SD-Karte sichern. Die erfolgreich getestete Firmware P016 FIX4b berücksichtigt auch den SysEx-Sonderfall Slot 128.

## 15. Sechs Workshops zur Klanggestaltung

### Workshop A – Classic Slow Phaser

Mit 6 Stufen, niedriger LFO-Rate, moderater Resonance, geringem Drive und mittlerer LFO Amount beginnen. SWEEP so einstellen, dass die Bewegung im musikalisch interessanten Bereich liegt. Den Effekt mit BYPASS vergleichen. **Variante:** Resonance erhöhen und prüfen, wie sich die Kerben betonen.

### Workshop B – Wide Stereo Sweep

OUTPUT MODE auf STEREO stellen, 12 Stufen wählen und STEREO PHASE zunächst auf 90° setzen. LFO RATE langsam einstellen und AMOUNT moderat erhöhen. Beide Ausgänge abhören. **Variante:** 180° ausprobieren und die räumliche Bewegung vergleichen.

### Workshop C – MIDI Clock Phaser

MIDI Clock aus einer DAW oder einem Sequencer an den Phaser senden. LFO SYNC auf MIDI stellen und eine rhythmische DIVISION wählen. Mit kurzen Akkorden oder perkussivem Material testen. **Variante:** Division wechseln und auf die Taktakzente achten. Bei fehlender Synchronisation Clock-Quelle und CLOCK STATUS prüfen.

### Workshop D – Dynamic Guitar Phaser

ENV FOLLOW aktivieren, Attack eher kurz, Release mittel und Sensitivity so einstellen, dass leise und kräftige Anschläge unterschiedliche Bewegungen auslösen. ENV DEPTH positiv wählen. **Variante:** negative Depth verwenden und den umgekehrten Anschlagverlauf vergleichen.

### Workshop E – Dual Matrix Motion

MATRIX ON aktivieren. Route 1: LFO→SWEEP, geringe positive Depth. Route 2: ENVELOPE→RESONANCE, kleine negative Depth. LFO eher langsam und MOD SMOOTH moderat einstellen. **Variante:** Route 2 auf DRIVE umstellen; Ausgangspegel kontrollieren.

### Workshop F – A/B-Optimierung am Windows-PC

Ein Preset in den Editor laden und als A sichern. Für B STAGES von 6 auf 12 ändern, Resonance und Analog Color leicht variieren. B speichern. Beide Zustände nacheinander an den Phaser senden und bei gleichem Eingangssignal vergleichen. Den bevorzugten Zustand in einen **neuen** PC-Bank-Slot übernehmen und abspeichern.

## 16. Fehlersuche

| Symptom | Prüfschritte |
|---|---|
| Kein Audio | Eingang, Ausgang, ADC/DAC, I²S, Output Mode, Bypass |
| Kein OLED | SSD1309-Typ, SPI, CS/DC/RST, Versorgung |
| SD nicht erkannt | CS9, SPI, Dateisystem, `/RTAL_PHASER/` |
| INVALID PRESET | V5-Datei, Prüfsumme, Herkunft, Dateiname |
| Keine MIDI-Reaktion | IN/OUT, MIDI-Kanal, CC32-Bank, DIN RX40/TX39 |
| LFO nicht synchron | SYNC=MIDI, MIDI Clock F8, CLOCK STATUS |
| Verzerrung | Drive, Resonance, Mod Depth und Eingangspegel reduzieren |
| Audioaussetzer | DSP AVG/PEAK, UNDERRUNS, Versorgung, SD-Aktivität |
| Editor-Port nicht verfügbar | USB-MIDI-Interface verbinden, Portliste aktualisieren, neu verbinden |
| SysEx-Datenbytefehler | Passende Editor-/Firmware-Kombination verwenden; Slot-128-Korrektur ab P016 FIX4b |

## 17. Release- und Qualitätscheckliste

Für eine reproduzierbare GitHub-Freigabe müssen Firmwarestand, Arduino-Core, Boardkonfiguration, Soundbank-Version und Editor-Build getrennt angegeben werden. Empfohlene Tests: alle 128 Presets laden, ausgewählte kritische Resonanz-/Matrixprogramme abhören, MIDI Program Change 0 und 127, CC32-Bänke, MIDI Clock, drei Ausgangsmodi, 6/12 Stufen, Envelope, beide Matrixwege, Editor Undo/Redo, A/B, Bankverwaltung, Backup und Restore mit Verify. Auch einen Windows-Build auf einem zweiten Rechner testen.

## 18. Historische Bezeichnungen und technische Kompatibilität

Die älteren Dokumente verwendeten „RTAL Phoenix Phaser“ und teilweise bereits „RTAL HQ Digital Phaser“. Der offizielle Name ist nun **HQ Digital Phaser**. Die frühere Firmware-Version v1.0.0 FINAL/P015 FIX5 und der später getestete Editor/Firmware-Stand P016 FIX4b/RC5 gehören unterschiedlichen Entwicklungsständen an. Technische Dateipfade, Presetkennungen und SysEx-Header bleiben aus Kompatibilitätsgründen unverändert. Der aktuelle GitHub-Repository-Pfad wurde in den bereitgestellten Unterlagen nicht verifiziert.

© 2026 RealTimeAudioLab. Technische Angaben basieren auf den gelieferten Projektunterlagen und dem dokumentierten Entwicklungsverlauf. Keine neue Hardwareprüfung im Rahmen dieser Handbuchaktualisierung.
