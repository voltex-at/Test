#include "AudioManager.h"
#include "Config.h"
#include <math.h>

namespace {
struct Note {
  uint16_t hz;
  uint16_t ms;
  uint16_t gap;
};

constexpr uint32_t SAMPLE_RATE = 16000;
constexpr uint8_t DAC_CENTER = 128;

// Public-domain/traditional-inspired short phrases, kept intentionally simple.
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
  for (int i = 0; i < 256; ++i) {
    const float a = (2.0f * PI * i) / 256.0f;
    sineLut_[i] = static_cast<int8_t>(lroundf(127.0f * sinf(a)));
  }
  stop();
}

void AudioManager::enableDac() {
  dacWrite(AppConfig::AUDIO_DAC_PIN, DAC_CENTER);
  delay(35);
}

void AudioManager::stop() {
  dacWrite(AppConfig::AUDIO_DAC_PIN, DAC_CENTER);
  delay(25);
  dacDisable(AppConfig::AUDIO_DAC_PIN);
  pinMode(AppConfig::AUDIO_DAC_PIN, INPUT);
  playing_ = false;
}

void AudioManager::playRest(uint16_t durationMs) {
  dacWrite(AppConfig::AUDIO_DAC_PIN, DAC_CENTER);
  delay(durationMs);
}

void AudioManager::playTone(uint16_t frequency, uint16_t durationMs) {
  if (frequency == 0 || durationMs == 0) {
    playRest(durationMs);
    return;
  }

  const uint32_t totalSamples = (SAMPLE_RATE * static_cast<uint32_t>(durationMs)) / 1000UL;
  const uint32_t phaseStep = static_cast<uint32_t>(
    (static_cast<uint64_t>(frequency) << 32) / SAMPLE_RATE
  );

  const uint32_t rampLimit = SAMPLE_RATE * 12UL / 1000UL;
  const uint32_t quarter = totalSamples / 4;
  const uint32_t attackSamples = quarter < rampLimit ? quarter : rampLimit;
  const uint32_t releaseSamples = attackSamples;

  uint32_t phase = 0;
  uint32_t nextUs = micros();

  for (uint32_t i = 0; i < totalSamples; ++i) {
    phase += phaseStep;

    uint16_t envelope = 256;
    if (attackSamples && i < attackSamples) {
      envelope = static_cast<uint16_t>((i * 256UL) / attackSamples);
    } else if (releaseSamples && i >= totalSamples - releaseSamples) {
      envelope = static_cast<uint16_t>(((totalSamples - 1 - i) * 256UL) / releaseSamples);
    }

    const int amplitude = (AppConfig::AUDIO_DAC_AMPLITUDE * envelope) >> 8;
    const int wave = sineLut_[phase >> 24];
    const int sample = DAC_CENTER + ((wave * amplitude) / 127);

    dacWrite(AppConfig::AUDIO_DAC_PIN, constrain(sample, 0, 255));

    nextUs += 1000000UL / SAMPLE_RATE;
    while (static_cast<int32_t>(micros() - nextUs) < 0) {}
  }

  dacWrite(AppConfig::AUDIO_DAC_PIN, DAC_CENTER);
}

void AudioManager::play(MelodyType melody) {
  if (playing_) return;

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
    case SceneType::BIRTHDAY:
      play(MelodyType::BIRTHDAY);
      break;
    case SceneType::SILVESTER:
      play(MelodyType::NEW_YEAR);
      break;
    case SceneType::HALLOWEEN:
      play(MelodyType::HALLOWEEN);
      break;
    case SceneType::EASTER:
      play(MelodyType::EASTER);
      break;
    case SceneType::VACATION:
      play(MelodyType::VACATION);
      break;
    default:
      play(MelodyType::CHRISTMAS);
      break;
  }
}
