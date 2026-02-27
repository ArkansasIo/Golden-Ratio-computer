#include "wolfman_alpha/wa_audio.hpp"
#include <vector>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <algorithm>

namespace wa::audio {

static void write_u32(std::ofstream& f, std::uint32_t v) {
  f.put((char)(v & 0xFF));
  f.put((char)((v >> 8) & 0xFF));
  f.put((char)((v >> 16) & 0xFF));
  f.put((char)((v >> 24) & 0xFF));
}
static void write_u16(std::ofstream& f, std::uint16_t v) {
  f.put((char)(v & 0xFF));
  f.put((char)((v >> 8) & 0xFF));
}

static bool write_wav_mono16(const std::string& path, const std::vector<std::int16_t>& pcm, int sampleRate) {
  std::ofstream f(path, std::ios::binary);
  if (!f) return false;

  const int channels = 1;
  const int bitsPerSample = 16;
  const std::uint32_t dataBytes = (std::uint32_t)(pcm.size() * sizeof(std::int16_t));

  f.write("RIFF", 4);
  write_u32(f, 36 + dataBytes);
  f.write("WAVE", 4);

  f.write("fmt ", 4);
  write_u32(f, 16);
  write_u16(f, 1); // PCM
  write_u16(f, (std::uint16_t)channels);
  write_u32(f, (std::uint32_t)sampleRate);
  write_u32(f, (std::uint32_t)(sampleRate * channels * (bitsPerSample / 8)));
  write_u16(f, (std::uint16_t)(channels * (bitsPerSample / 8)));
  write_u16(f, (std::uint16_t)bitsPerSample);

  f.write("data", 4);
  write_u32(f, dataBytes);
  f.write(reinterpret_cast<const char*>(pcm.data()), dataBytes);
  return true;
}

static inline double clamp1(double x) { return std::max(-1.0, std::min(1.0, x)); }
static inline double dnoise(double t) {
  return 0.55*std::sin(2.0*M_PI*937.0*t) + 0.35*std::sin(2.0*M_PI*1433.0*t) + 0.20*std::sin(2.0*M_PI*2117.0*t);
}

bool generate_clockwork_loop(const std::string& path, double seconds, int bpm) {
  const int sr = 44100;
  const int total = (int)std::round(seconds * sr);
  std::vector<std::int16_t> pcm(total);

  const double beatsPerSec = bpm / 60.0;
  const double tickHz = beatsPerSec;
  const double whirrHz = 130.0;
  const double whirrHz2 = 261.0;

  for (int n=0;n<total;n++) {
    double t = (double)n / sr;

    double am = 0.55 + 0.45*std::sin(2.0*M_PI*2.0*t);
    double whirr = 0.08*am*(0.65*std::sin(2.0*M_PI*whirrHz*t) + 0.35*std::sin(2.0*M_PI*whirrHz2*t));

    double phase = std::fmod(t * tickHz, 1.0);
    double env = 0.0;
    if (phase < 0.03) env = std::exp(-phase * 180.0);
    double tick = 0.33 * env * (0.7*std::sin(2.0*M_PI*900.0*t) + 0.3*std::sin(2.0*M_PI*1400.0*t));

    double gritEnv = 0.0;
    if (phase < 0.10) gritEnv = std::exp(-phase * 22.0);
    double grit = 0.04 * gritEnv * dnoise(t);

    double s = whirr + tick + grit;
    s = clamp1(s);
    pcm[n] = (std::int16_t)std::lround(s * 32767.0);
  }

  return write_wav_mono16(path, pcm, sr);
}

bool generate_ratchet_tick(const std::string& path, double seconds, int bpm) {
  const int sr = 44100;
  const int total = (int)std::round(seconds * sr);
  std::vector<std::int16_t> pcm(total);

  const double beatsPerSec = bpm / 60.0;
  const double tickHz = beatsPerSec;

  for (int n=0;n<total;n++) {
    double t = (double)n / sr;

    double phase = std::fmod(t * tickHz, 1.0);
    double env = 0.0;
    if (phase < 0.02) env = std::exp(-phase * 240.0);

    double click = 0.55*env*(0.6*std::sin(2.0*M_PI*1800.0*t) + 0.4*std::sin(2.0*M_PI*2600.0*t));
    double snap  = 0.35*env*dnoise(t);

    double s = 0.55*click + 0.45*snap;
    s = clamp1(s);
    pcm[n] = (std::int16_t)std::lround(s * 32767.0);
  }

  return write_wav_mono16(path, pcm, sr);
}

bool generate_gear_whirr(const std::string& path, double seconds, double hz) {
  const int sr = 44100;
  const int total = (int)std::round(seconds * sr);
  std::vector<std::int16_t> pcm(total);

  for (int n=0;n<total;n++) {
    double t = (double)n / sr;

    double wob = 0.8 + 0.2*std::sin(2.0*M_PI*0.7*t);
    double f1 = hz * wob;
    double f2 = 2.0*hz * (0.9 + 0.1*std::sin(2.0*M_PI*0.31*t));

    double s = 0.18*(0.65*std::sin(2.0*M_PI*f1*t) + 0.35*std::sin(2.0*M_PI*f2*t));
    s += 0.03*dnoise(t);

    s = clamp1(s);
    pcm[n] = (std::int16_t)std::lround(s * 32767.0);
  }

  return write_wav_mono16(path, pcm, sr);
}

bool generate_haunted_drone(const std::string& path, double seconds) {
  const int sr = 44100;
  const int total = (int)std::round(seconds * sr);
  std::vector<std::int16_t> pcm(total);

  const double base = 48.0;
  const double detune = 0.07;

  for (int n=0;n<total;n++) {
    double t = (double)n / sr;

    double breath = 0.55 + 0.45*std::sin(2.0*M_PI*0.12*t);

    double s = 0.0;
    for (int k=0;k<13;k++) {
      double fk = base * (k+1) * (1.0 + detune*std::sin(2.0*M_PI*(0.03 + 0.004*k)*t));
      double ak = 1.0 / (1.0 + 0.35*k);
      s += ak * std::sin(2.0*M_PI*fk*t);
    }
    s *= (0.06 * breath);
    s += 0.01 * breath * dnoise(t);

    s = clamp1(s);
    pcm[n] = (std::int16_t)std::lround(s * 32767.0);
  }

  return write_wav_mono16(path, pcm, sr);
}

bool generate_zodiac_13_pulse(const std::string& path, double seconds, int bpm) {
  const int sr = 44100;
  const int total = (int)std::round(seconds * sr);
  std::vector<std::int16_t> pcm(total);

  const double beatsPerSec = bpm / 60.0;
  const double stepHz = beatsPerSec;
  const double carrierBase = 220.0;

  for (int n=0;n<total;n++) {
    double t = (double)n / sr;

    double stepPhase = std::fmod(t * stepHz, 1.0);
    int step = (int)std::floor(std::fmod(t * stepHz, 13.0));

    double offsets[13] = {0, 2, 5, 7, 9, 12, 14, 12, 9, 7, 5, 2, 0};
    double semi = offsets[step];
    double freq = carrierBase * std::pow(2.0, semi/12.0);

    double env = 0.0;
    if (stepPhase < 0.10) env = std::exp(-stepPhase * 18.0);

    double s = 0.22 * env * (0.7*std::sin(2.0*M_PI*freq*t) + 0.3*std::sin(2.0*M_PI*(2.0*freq)*t));
    s += 0.02 * env * dnoise(t);

    s = clamp1(s);
    pcm[n] = (std::int16_t)std::lround(s * 32767.0);
  }

  return write_wav_mono16(path, pcm, sr);
}

} // namespace wa::audio
