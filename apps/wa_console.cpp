#include "wolfman_alpha/wa_machine.hpp"
#include "wolfman_alpha/wa_io.hpp"
#include "wolfman_alpha/wa_audio.hpp"
#include "wolfman_alpha/wa_components.hpp"
#include <iostream>
#include <filesystem>

int main() {
  wa::Machine m(10, 360);
  wa::MechanicalComputer mech({
    64,   // wordBits
    8,    // registers
    512,  // ramWords
    8192, // storageWords
    24.0  // clockHz
  });

  std::filesystem::create_directories("assets");

  // Original haunted clockwork sound pack (NOT film audio)
  if (wa::audio::generate_clockwork_loop("assets/clockwork_loop.wav", 3.0, 120))
    std::cout << "Generated assets/clockwork_loop.wav\n";
  if (wa::audio::generate_ratchet_tick("assets/ratchet_tick.wav", 2.0, 120))
    std::cout << "Generated assets/ratchet_tick.wav\n";
  if (wa::audio::generate_gear_whirr("assets/gear_whirr.wav", 2.5, 140.0))
    std::cout << "Generated assets/gear_whirr.wav\n";
  if (wa::audio::generate_haunted_drone("assets/haunted_drone.wav", 5.0))
    std::cout << "Generated assets/haunted_drone.wav\n";
  if (wa::audio::generate_zodiac_13_pulse("assets/zodiac_13_pulse.wav", 6.0, 120))
    std::cout << "Generated assets/zodiac_13_pulse.wav\n";

  std::cout << m.capacityString(true) << "\n";
  std::cout << mech.summary() << "\n";

  wa::Console console(m);
  console.repl();
  return 0;
}
