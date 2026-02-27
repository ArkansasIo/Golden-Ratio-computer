#pragma once
#include "wa_machine.hpp"
#include "wa_cpu.hpp"
#include "wa_zodiac.hpp"
#include <string>
#include <vector>
#include <memory>
#include <deque>
#include <cstdint>
#include <chrono>

namespace wa {

struct ConsoleConfig {
  int printCount{64};
};

class WindowApi {
public:
  explicit WindowApi(std::size_t maxLines = 128) : maxLines_(maxLines) {}

  void pushInput(const std::string& line);
  void pushOutput(const std::string& line);
  void pushEvent(const std::string& line);

  std::string inputPrompt() const;
  std::string renderInput() const;
  std::string renderOutput() const;
  std::string renderEvents() const;
  std::string renderAll() const;

private:
  std::size_t maxLines_;
  std::deque<std::string> input_;
  std::deque<std::string> output_;
  std::deque<std::string> events_;

  void pushLine(std::deque<std::string>& pane, const std::string& line);
  std::string renderPane(const char* title, const std::deque<std::string>& pane) const;
};

class IoClock {
public:
  IoClock();

  void onCommand();
  void onGearTick(int k);

  std::uint64_t commandCount() const { return commandCount_; }
  std::uint64_t gearTicks() const { return gearTicks_; }

  std::string promptTag() const;
  std::string stamp(const char* channel, const std::string& msg) const;

private:
  std::chrono::system_clock::time_point startedAt_;
  std::uint64_t commandCount_{0};
  std::uint64_t gearTicks_{0};

  static std::string timeNowString();
};

class Console {
public:
  explicit Console(Machine& m, ConsoleConfig cfg = {});
  void repl();

private:
  Machine& m_;
  ConsoleConfig cfg_;
  std::unique_ptr<CpuBase> cpu_;
  WindowApi windows_;
  IoClock clock_;

  static std::vector<std::string> split(const std::string& s);
  static Dir parseDir(const std::string& s);
  static int toInt(const std::string& s);
  void emitOutput(const std::string& msg);
  void emitEvent(const std::string& msg);
  void recordInput(const std::string& line);

  void help() const;

  static Zodiac13 parseGlyph(const std::string& s);
  static std::string glyphName(Zodiac13 g);

  void dialRingToGlyph(int ring, Zodiac13 target);
};

} // namespace wa
