#include "PhoenixPhaserInput.h"

PhoenixPhaserInput *PhoenixPhaserInput::_instance = nullptr;

PhoenixPhaserInput::PhoenixPhaserInput(const uint8_t *buttonPins, uint8_t encA, uint8_t encB, uint8_t encSw)
: _buttonPins(buttonPins), _encA(encA), _encB(encB), _encSw(encSw),
  _encRawDelta(0), _encAccum(0), _encLast(0) {}

void PhoenixPhaserInput::initButton(ButtonState &b, uint8_t pin) {
  b.pin = pin;
  pinMode(pin, INPUT_PULLUP);
  b.lastRaw = digitalRead(pin);
  b.stable = b.lastRaw;
  b.press = false;
  b.downMs = 0;
  b.longSent = false;
  b.lastChangeMs = millis();
}

void PhoenixPhaserInput::begin() {
  for (uint8_t i=0; i<8; ++i) initButton(_buttons[i], _buttonPins[i]);
  initButton(_encButton, _encSw);
  pinMode(_encA, INPUT_PULLUP);
  pinMode(_encB, INPUT_PULLUP);
  _encLast = (digitalRead(_encA) ? 1 : 0) | (digitalRead(_encB) ? 2 : 0);
  _instance = this;
  attachInterrupt(digitalPinToInterrupt(_encA), PhoenixPhaserInput::encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(_encB), PhoenixPhaserInput::encoderISR, CHANGE);
}

void IRAM_ATTR PhoenixPhaserInput::encoderISR() {
  if (_instance) _instance->scanEncoderISR();
}

void IRAM_ATTR PhoenixPhaserInput::scanEncoderISR() {
  const uint8_t s = (digitalRead(_encA) ? 1 : 0) | (digitalRead(_encB) ? 2 : 0);
  if (s != _encLast) {
    static const int8_t tbl[16] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
    _encRawDelta += tbl[(_encLast << 2) | s];
    _encLast = s;
  }
}

void PhoenixPhaserInput::scanButton(ButtonState &b) {
  b.press = false;
  const bool raw = digitalRead(b.pin);
  const uint32_t now = millis();
  if (raw != b.lastRaw) {
    b.lastRaw = raw;
    b.lastChangeMs = now;
  }
  if ((now - b.lastChangeMs) > 18 && raw != b.stable) {
    b.stable = raw;
    if (b.stable == LOW) b.press = true;
  }
}

void PhoenixPhaserInput::update(PhoenixPhaserInputState &state) {
  state.clear();

  // Preset keys: fire save once after 2 s; short load only on release.
  for (uint8_t i=0; i<8; ++i) {
    ButtonState &b = _buttons[i];
    const bool wasDown = b.stable == LOW;
    scanButton(b);
    const uint32_t t = millis();
    if (!wasDown && b.stable == LOW) { b.downMs = t; b.longSent = false; }
    if (b.stable == LOW && !b.longSent && t - b.downMs >= 2000) {
      b.longSent = true;
      state.hold[i+1] = true;
    }
    if (wasDown && b.stable == HIGH) {
      if (!b.longSent) state.f[i+1] = true;
      b.downMs = 0;
    }
  }
  scanButton(_encButton);
  if (_encButton.press) { _pressedAt = millis(); _longSent = false; }
  if (_encButton.stable == LOW && !_longSent && millis() - _pressedAt >= 2000) {
    state.encoderLongPress = true; _longSent = true;
  }
  if (_encButton.stable == HIGH && _pressedAt != 0) {
    if (!_longSent) state.encoderPress = true;
    _pressedAt = 0;
  }

  int32_t raw;
  noInterrupts();
  raw = _encRawDelta;
  _encRawDelta = 0;
  interrupts();
  _encAccum += raw;

  if (_encAccum >= 4 || _encAccum <= -4) {
    int16_t detents = (int16_t)(_encAccum / 4);
    _encAccum -= (int32_t)detents * 4;
    if (detents > 10) detents = 10;
    if (detents < -10) detents = -10;
    state.encoderStep = detents;
  }
}
