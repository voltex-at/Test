#include "AudioManager.h"
#include "Config.h"
#include <Wire.h>
#include <ESP_I2S.h>
#include <math.h>

namespace {
struct Note {
  uint16_t hz;
  uint16_t ms;
  uint16_t gap;
};

constexpr uint32_t SAMPLE_RATE = 44100;
I2SClass audioI2S;
bool codecReady = false;

constexpr Note CHRISTMAS[] = {
  {330,180,55},{330,180,55},{330,360,90},
  {330,180,55},{330,180,55},{330,360,90},
  {330,180,55},{392,180,55},{262,180,55},{294,180,55},{330,620,160}
};

constexpr Note BIRTHDAY[] = {
  {392,180,45},{392,180,45},{440,360,70},{392,360,70},{523,360,70},{494,620,120},
  {392,180,45},{392,180,45},{440,360,70},{392,360,70},{587,360,70},{523,620,120}
};

constexpr Note HALLOWEEN[] = {
  {220,260,60},{311,260,60},{294,260,60},{311,260,90},
  {220,260,60},{370,260,60},{349,260,60},{311,520,160}
};

constexpr Note NEW_YEAR[] = {
  {262,260,55},{349,260,55},{330,380,70},{349,260,55},
  {440,260,55},{392,380,70},{349,260,55},{330,260,55},{262,620,140}
};

constexpr Note EASTER[] = {
  {262,180,45},{330,180,45},{392,180,45},{523,360,80},
  {392,180,45},{440,180,45},{494,180,45},{523,520,120}
};

constexpr Note VACATION[] = {
  {392,180,45},{440,180,45},{494,180,45},{587,360,70},
  {494,180,45},{440,180,45},{392,180,45},{330,360,70},
  {392,520,120}
};

void codecWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(AppConfig::ES8311_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

bool codecPresent() {
  Wire.beginTransmission(AppConfig::ES8311_ADDR);
  return Wire.endTransmission() == 0;
}

template <size_t N>
void playSequence(AudioManager& audio, const Note (&notes)[N]) {
  for (const auto& n : notes) {
    if (n.hz) audio.playTone(n.hz, n.ms);
    else audio.playRest(n.ms);
    if (n.gap) audio.playRest(n.gap);
  }
}
}

void AudioManager::begin() {
  pinMode(AppConfig::AUDIO_AMP_EN, OUTPUT);
  digitalWrite(AppConfig::AUDIO_AMP_EN, HIGH); // active-low amplifier: silence

  Wire.begin(AppConfig::I2C_SDA, AppConfig::I2C_SCL);
  Wire.setClock(400000);
  delay(20);

  if (!codecPresent()) {
    Serial.println("ES8311: not found");
    codecReady = false;
    playing_ = false;
    return;
  }

  // ES8311: I2S slave, 16-bit. Sequence verified for ES3C28P board family.
  codecWrite(0x00, 0x1F); delay(20);
  codecWrite(0x00, 0x00);
  codecWrite(0x00, 0x80);
  codecWrite(0x01, 0x3F);
  codecWrite(0x02, 0x00);
  codecWrite(0x03, 0x10);
  codecWrite(0x04, 0x10);
  codecWrite(0x05, 0x00);
  codecWrite(0x06, 0x03);
  codecWrite(0x07, 0x00);
  codecWrite(0x08, 0xFF);
  codecWrite(0x09, 0x0C);
  codecWrite(0x0A, 0x0C);
  codecWrite(0x0D, 0x01);
  codecWrite(0x0E, 0x02);
  codecWrite(0x12, 0x00);
  codecWrite(0x13, 0x10);
  codecWrite(0x1C, 0x6A);
  codecWrite(0x37, 0x08);
  codecWrite(0x32, 0xC8);

  audioI2S.setPins(AppConfig::AUDIO_BCLK, AppConfig::AUDIO_LRCK,
                   AppConfig::AUDIO_DOUT, AppConfig::AUDIO_DIN,
                   AppConfig::AUDIO_MCLK);
  codecReady = audioI2S.begin(I2S_MODE_STD, SAMPLE_RATE,
                              I2S_DATA_BIT_WIDTH_16BIT,
                              I2S_SLOT_MODE_STEREO);
  Serial.printf("ES8311/I2S: %s\n", codecReady ? "OK" : "FAIL");
  playing_ = false;
}

void AudioManager::enableDac() {
  if (!codecReady) return;
  digitalWrite(AppConfig::AUDIO_AMP_EN, LOW);
  delay(8);
}

void AudioManager::stop() {
  if (codecReady) {
    playRest(20);
    delay(5);
    digitalWrite(AppConfig::AUDIO_AMP_EN, HIGH);
  } else {
    digitalWrite(AppConfig::AUDIO_AMP_EN, HIGH);
  }
  playing_ = false;
}

void AudioManager::playRest(uint16_t durationMs) {
  if (!codecReady || durationMs == 0) {
    if (durationMs) delay(durationMs);
    return;
  }

  static int16_t silence[256 * 2] = {};
  uint32_t total = (SAMPLE_RATE * static_cast<uint32_t>(durationMs)) / 1000UL;
  uint32_t done = 0;
  while (done < total) {
    const uint32_t frames = min<uint32_t>(256, total - done);
    audioI2S.write(reinterpret_cast<uint8_t*>(silence), frames * 2 * sizeof(int16_t));
    done += frames;
    delay(0);
  }
}

void AudioManager::playTone(uint16_t frequency, uint16_t durationMs) {
  if (!codecReady || frequency == 0 || durationMs == 0) {
    playRest(durationMs);
    return;
  }

  static int16_t frames[256 * 2];
  uint32_t total = (SAMPLE_RATE * static_cast<uint32_t>(durationMs)) / 1000UL;
  uint32_t done = 0;
  float phase = 0.0f;
  const float step = (2.0f * PI * frequency) / SAMPLE_RATE;
  constexpr int16_t amplitude = 5400;

  while (done < total) {
    const uint32_t count = min<uint32_t>(256, total - done);
    for (uint32_t i = 0; i < count; ++i) {
      int16_t sample = static_cast<int16_t>(sinf(phase) * amplitude);
      // Short attack/release to avoid clicks.
      const uint32_t absoluteFrame = done + i;
      const uint32_t ramp = SAMPLE_RATE / 100; // 10 ms
      if (absoluteFrame < ramp) sample = static_cast<int16_t>((sample * absoluteFrame) / ramp);
      if (total > ramp && absoluteFrame > total - ramp) {
        sample = static_cast<int16_t>((sample * (total - absoluteFrame)) / ramp);
      }
      frames[i * 2] = sample;
      frames[i * 2 + 1] = sample;
      phase += step;
      if (phase >= 2.0f * PI) phase -= 2.0f * PI;
    }
    audioI2S.write(reinterpret_cast<uint8_t*>(frames), count * 2 * sizeof(int16_t));
    done += count;
    delay(0);
  }
}

void AudioManager::play(MelodyType melody) {
  if (playing_ || !codecReady) return;
  playing_ = true;
  enableDac();

  switch (melody) {
    case MelodyType::BIRTHDAY:  playSequence(*this, BIRTHDAY); break;
    case MelodyType::HALLOWEEN: playSequence(*this, HALLOWEEN); break;
    case MelodyType::NEW_YEAR:  playSequence(*this, NEW_YEAR); break;
    case MelodyType::EASTER:    playSequence(*this, EASTER); break;
    case MelodyType::VACATION:  playSequence(*this, VACATION); break;
    case MelodyType::CHRISTMAS:
    default:                    playSequence(*this, CHRISTMAS); break;
  }

  stop();
}

void AudioManager::playForScene(SceneType scene) {
  switch (scene) {
    case SceneType::BIRTHDAY:  play(MelodyType::BIRTHDAY); break;
    case SceneType::SILVESTER: play(MelodyType::NEW_YEAR); break;
    case SceneType::HALLOWEEN: play(MelodyType::HALLOWEEN); break;
    case SceneType::EASTER:    play(MelodyType::EASTER); break;
    case SceneType::VACATION:  play(MelodyType::VACATION); break;
    default:                   play(MelodyType::CHRISTMAS); break;
  }
}
