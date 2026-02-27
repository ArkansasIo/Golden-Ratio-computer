#pragma once
#include <string>

namespace wa::audio {

bool generate_clockwork_loop(const std::string& path, double seconds=3.0, int bpm=120);
bool generate_ratchet_tick(const std::string& path, double seconds=1.0, int bpm=120);
bool generate_gear_whirr(const std::string& path, double seconds=2.0, double hz=140.0);
bool generate_haunted_drone(const std::string& path, double seconds=4.0);
bool generate_zodiac_13_pulse(const std::string& path, double seconds=4.0, int bpm=120);

} // namespace wa::audio
