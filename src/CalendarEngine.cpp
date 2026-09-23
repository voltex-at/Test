#include "CalendarEngine.h"

// Algorytm typu civil-date -> liczba dni. Operuje na datach kalendarzowych,
// wiec zmiany DST i dlugosc doby 23/25h nie psuja licznika.
int64_t CalendarEngine::daysFromCivil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe);
}

CalendarState CalendarEngine::evaluate(const tm& t, const DeviceSettings& settings) const {
  CalendarState s;
  const int year = t.tm_year + 1900;
  const int month = t.tm_mon + 1;
  const int day = t.tm_mday;

  const bool christmas = (month == 12 && day >= 24 && day <= 26);
  s.christmasGreeting = christmas;

  int targetYear = year;
  if (month == 12 && day >= 27) {
    targetYear = year + 1;
  }
  s.targetYear = targetYear;

  const int64_t nowDay = daysFromCivil(year, month, day);
  const int64_t targetDay = daysFromCivil(targetYear, 12, 24);
  s.daysToChristmas = static_cast<int>(targetDay - nowDay);
  if (s.daysToChristmas < 0) s.daysToChristmas = 0;

  if (christmas) {
    s.scene = SceneType::CHRISTMAS;
    return s;
  }

  const bool birthday = settings.birthdayDay == day && settings.birthdayMonth == month &&
                        settings.birthdayDay > 0 && settings.birthdayMonth > 0;
  if (birthday) {
    s.scene = SceneType::BIRTHDAY;
    s.birthday = true;
    return s;
  }

  if (month == 12 && day == 5) {
    s.scene = SceneType::KRAMPUS;
  } else if (month == 12 && day == 6) {
    s.scene = SceneType::NIKOLAUS;
  } else if (month == 12 && day == 27) {
    s.scene = SceneType::GRINCH;
  } else if (month == 12 && day == 31) {
    s.scene = SceneType::SILVESTER;
  } else if (month == 1 && day == 6) {
    s.scene = SceneType::GRINCH;
  } else {
    s.scene = SceneType::NORMAL;
  }

  return s;
}

const char* CalendarEngine::sceneImage(SceneType scene) {
  switch (scene) {
    case SceneType::KRAMPUS:   return "/images/krampus.jpg";
    case SceneType::NIKOLAUS:  return "/images/nikolaus.jpg";
    case SceneType::CHRISTMAS: return "/images/christmas.jpg";
    case SceneType::GRINCH:    return "/images/grinch.jpg";
    case SceneType::SILVESTER: return "/images/silvester.jpg";
    case SceneType::BIRTHDAY:  return "/images/birthday.jpg";
    default:                   return "/images/normal.jpg";
  }
}

const char* CalendarEngine::sceneLabel(SceneType scene) {
  switch (scene) {
    case SceneType::KRAMPUS:   return "KRAMPUS";
    case SceneType::NIKOLAUS:  return "NIKOLAUS";
    case SceneType::CHRISTMAS: return "WEIHNACHTEN";
    case SceneType::GRINCH:    return "GRINCH";
    case SceneType::SILVESTER: return "SILVESTER";
    case SceneType::BIRTHDAY:  return "HAPPY BIRTHDAY";
    default:                   return "";
  }
}
