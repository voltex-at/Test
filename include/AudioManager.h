#pragma once
#include <Arduino.h>
#include "CalendarEngine.h"

enum class MelodyType : uint8_t {
  CHRISTMAS,
  BIRTHDAY,
  HALLOWEEN,
  NEW_YEAR,
  EASTER,
  VACATION
};

class AudioManager {
public:
  void begin();
  void stop();
  void play(MelodyType melody);
  void playForScene(SceneType scene);
  bool isPlaying() const { return playing_; }

  // Uzywane wewnetrznie przez sekwencer nut.
  void playTone(uint16_t frequency, uint16_t durationMs);
  void playRest(uint16_t durationMs);

private:
  bool playing_ = false;
  int8_t sineLut_[256]{};

  void enableDac();
};
