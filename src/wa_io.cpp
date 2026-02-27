#include "wolfman_alpha/wa_io.hpp"
#include "wolfman_alpha/wa_calc.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace wa {

namespace {

enum class CommandId {
  Help,
  Quit,
  Mode,
  Cap,
  Set,
  Flip,
  Shift,
  Tick,
  Print,
  Regs,
  Run,
  Step,
  Glyph,
  Dial,
  Clock,
  Windows,
  Calc,
  Equation,
  Unknown
};

CommandId parseCommandId(const std::string& op) {
  static const std::unordered_map<std::string, CommandId> kMap{
    {"help", CommandId::Help},
    {"quit", CommandId::Quit},
    {"exit", CommandId::Quit},
    {"mode", CommandId::Mode},
    {"cap", CommandId::Cap},
    {"set", CommandId::Set},
    {"flip", CommandId::Flip},
    {"shift", CommandId::Shift},
    {"tick", CommandId::Tick},
    {"print", CommandId::Print},
    {"regs", CommandId::Regs},
    {"run", CommandId::Run},
    {"step", CommandId::Step},
    {"glyph", CommandId::Glyph},
    {"dial", CommandId::Dial},
    {"clock", CommandId::Clock},
    {"windows", CommandId::Windows},
    {"calc", CommandId::Calc},
    {"equation", CommandId::Equation},
  };

  auto it = kMap.find(op);
  if (it == kMap.end()) return CommandId::Unknown;
  return it->second;
}

std::string joinTokens(const std::vector<std::string>& tokens, std::size_t start) {
  if (start >= tokens.size()) return {};
  std::ostringstream oss;
  for (std::size_t i = start; i < tokens.size(); ++i) {
    if (i > start) oss << ' ';
    oss << tokens[i];
  }
  return oss.str();
}

} // namespace

IoClock::IoClock() : startedAt_(std::chrono::system_clock::now()) {}

void IoClock::onCommand() {
  commandCount_++;
}

void IoClock::onGearTick(int k) {
  if (k > 0) gearTicks_ += static_cast<std::uint64_t>(k);
}

std::string IoClock::timeNowString() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t tt = std::chrono::system_clock::to_time_t(now);
  std::tm localTm{};
#if defined(_WIN32)
  localtime_s(&localTm, &tt);
#else
  localTm = *std::localtime(&tt);
#endif
  std::ostringstream oss;
  oss << std::put_time(&localTm, "%H:%M:%S");
  return oss.str();
}

std::string IoClock::promptTag() const {
  std::ostringstream oss;
  oss << "[clk " << timeNowString() << " cmd=" << commandCount_ << " ticks=" << gearTicks_ << "]";
  return oss.str();
}

std::string IoClock::stamp(const char* channel, const std::string& msg) const {
  std::ostringstream oss;
  oss << "[" << timeNowString() << "][" << channel << "][ticks=" << gearTicks_ << "] " << msg;
  return oss.str();
}

void WindowApi::pushLine(std::deque<std::string>& pane, const std::string& line) {
  pane.push_back(line);
  while (pane.size() > maxLines_) pane.pop_front();
}

void WindowApi::pushInput(const std::string& line) {
  pushLine(input_, line);
}

void WindowApi::pushOutput(const std::string& line) {
  pushLine(output_, line);
}

void WindowApi::pushEvent(const std::string& line) {
  pushLine(events_, line);
}

std::string WindowApi::inputPrompt() const {
  return "wa[in]> ";
}

std::string WindowApi::renderPane(const char* title, const std::deque<std::string>& pane) const {
  std::ostringstream oss;
  oss << "=== " << title << " ===\n";
  for (const auto& line : pane) oss << line << "\n";
  return oss.str();
}

std::string WindowApi::renderInput() const {
  return renderPane("Input", input_);
}

std::string WindowApi::renderOutput() const {
  return renderPane("Output", output_);
}

std::string WindowApi::renderEvents() const {
  return renderPane("Events", events_);
}

std::string WindowApi::renderAll() const {
  std::ostringstream oss;
  oss << renderInput() << renderOutput() << renderEvents();
  return oss.str();
}

Console::Console(Machine& m, ConsoleConfig cfg) : m_(m), cfg_(cfg), windows_(128) {
  cpu_ = std::make_unique<CPU64>(m_);
}

void Console::emitOutput(const std::string& msg) {
  const std::string stamped = clock_.stamp("OUT", msg);
  windows_.pushOutput(stamped);
  std::cout << stamped << "\n";
}

void Console::emitEvent(const std::string& msg) {
  windows_.pushEvent(clock_.stamp("EVT", msg));
}

void Console::recordInput(const std::string& line) {
  windows_.pushInput(clock_.stamp("IN", line));
}

std::vector<std::string> Console::split(const std::string& s) {
  std::istringstream iss(s);
  std::vector<std::string> out;
  for (std::string t; iss >> t;) out.push_back(t);
  return out;
}

int Console::toInt(const std::string& s) {
  if (s.rfind("0b",0)==0) return std::stoi(s.substr(2), nullptr, 2);
  if (s.rfind("0x",0)==0) return std::stoi(s.substr(2), nullptr, 16);
  return std::stoi(s);
}

Dir Console::parseDir(const std::string& s) {
  std::string u = s;
  std::transform(u.begin(), u.end(), u.begin(), ::toupper);
  if (u == "LEFT" || u == "L") return Dir::Left;
  if (u == "RIGHT" || u == "R") return Dir::Right;
  throw std::invalid_argument("Bad direction");
}

Zodiac13 Console::parseGlyph(const std::string& s) {
  std::string u = s;
  std::transform(u.begin(), u.end(), u.begin(), ::toupper);
  if (u.size()>=2 && u[0]=='Z') {
    int v = std::stoi(u.substr(1));
    if (v < 0 || v >= ZODIAC_COUNT) throw std::invalid_argument("Glyph out of range Z0..Z12");
    return (Zodiac13)v;
  }
  throw std::invalid_argument("Glyph format: Z0..Z12");
}

std::string Console::glyphName(Zodiac13 g) {
  return std::string(ZodiacNames[(int)g]);
}

void Console::dialRingToGlyph(int ring, Zodiac13 target) {
  const int maxSteps = m_.gearsPerRing() * 2;
  for (int step=0; step<maxSteps; step++) {
    auto cur = activeGlyphFromOffset(m_.ring(ring).offset(), m_.gearsPerRing());
    if (cur == target) return;

    int curV = (int)cur;
    int tgtV = (int)target;
    int diff = (tgtV - curV);

    if (diff == 0) return;
    if (diff > 0) m_.shiftRing(ring, Dir::Left, 1);
    else          m_.shiftRing(ring, Dir::Right, 1);
  }
}

void Console::help() const {
  std::cout <<
    "Commands:\n"
    "  help\n"
    "  mode 64|360|720           # select CPU profile\n"
    "  cap [1|2]                 # capacity: 1-bit-per-gear or 2-bits-per-gear-cell\n"
    "  set r i v                 # set gear-bit (0/1)\n"
    "  flip r i\n"
    "  shift r LEFT|RIGHT k      # stargate ring shift\n"
    "  tick k                    # tick all rings by their current dir\n"
    "  print r [count]\n"
    "  regs [countBits]          # dump CPU registers (first N bits)\n"
    "  run n                     # run n CPU steps\n"
    "  step                      # run 1 CPU step\n"
    "  glyph r                   # show active glyph on ring r\n"
    "  dial r Z0..Z12            # rotate ring until target glyph is active\n"
    "  clock status|tick [n]     # clock/gear tick controls\n"
    "  calc eval <expr>          # arithmetic/formal expression evaluator\n"
    "  calc evalx <x> <expr>     # evaluate expression using variable x\n"
    "  calc deriv <x> <expr>     # numeric derivative d/dx at x\n"
    "  calc integ <a> <b> <n> <expr> # simpson integral on [a,b] with n steps\n"
    "  calc quad <a> <b> <c>     # solve a*x^2 + b*x + c = 0\n"
    "  calc solve <equation>     # solve linear equation with x (example: 2*x+3=9)\n"
    "  equation <lhs=rhs>        # alias of calc solve\n"
    "  windows                   # render input/output/event panes\n"
    "  quit\n";
}

void Console::repl() {
  emitOutput("WolfmanAlpha Gear Console (CPU + Zodiac)");
  emitOutput("Type 'help' for commands.");

  std::string line;
  while (true) {
    std::cout << clock_.promptTag() << " " << windows_.inputPrompt() << std::flush;
    if (!std::getline(std::cin, line)) break;
    clock_.onCommand();
    recordInput(line);

    auto t = split(line);
    if (t.empty()) continue;

    try {
      std::string op = t[0];
      std::transform(op.begin(), op.end(), op.begin(), ::tolower);
      const auto cmd = parseCommandId(op);

      switch (cmd) {
        case CommandId::Help:
          help();
          break;

        case CommandId::Quit:
          return;

        case CommandId::Mode: {
          int mode = toInt(t[1]);
          if (mode == 64) cpu_ = std::make_unique<CPU64>(m_);
          else if (mode == 360) cpu_ = std::make_unique<CPU360>(m_);
          else if (mode == 720) cpu_ = std::make_unique<CPU720>(m_);
          else throw std::invalid_argument("mode must be 64, 360, or 720");
          emitOutput("CPU mode set to " + std::to_string(mode));
          break;
        }

        case CommandId::Cap: {
          int mode = (t.size() >= 2) ? toInt(t[1]) : 2;
          emitOutput(m_.capacityString(mode == 2));
          break;
        }

        case CommandId::Set: {
          int r = toInt(t[1]);
          int i = toInt(t[2]);
          int v = toInt(t[3]);
          m_.setBit(r, i, (u8)v);
          emitEvent("set ring=" + std::to_string(r) + " idx=" + std::to_string(i) + " v=" + std::to_string(v));
          break;
        }

        case CommandId::Flip: {
          int r = toInt(t[1]);
          int i = toInt(t[2]);
          m_.flipBit(r, i);
          emitEvent("flip ring=" + std::to_string(r) + " idx=" + std::to_string(i));
          break;
        }

        case CommandId::Shift: {
          int r = toInt(t[1]);
          Dir d = parseDir(t[2]);
          int k = (t.size() >= 4) ? toInt(t[3]) : 1;
          m_.shiftRing(r, d, k);
          emitEvent("shift ring=" + std::to_string(r) + " steps=" + std::to_string(k));
          break;
        }

        case CommandId::Tick: {
          int k = (t.size() >= 2) ? toInt(t[1]) : 1;
          m_.tickAll(k);
          clock_.onGearTick(k);
          emitEvent("tick +" + std::to_string(k) + " (total=" + std::to_string(clock_.gearTicks()) + ")");
          break;
        }

        case CommandId::Print: {
          int r = toInt(t[1]);
          int c = (t.size() >= 3) ? toInt(t[2]) : cfg_.printCount;
          emitOutput(m_.dumpRing(r, c));
          break;
        }

        case CommandId::Regs: {
          int c = (t.size() >= 2) ? toInt(t[1]) : 64;
          emitOutput(cpu_->regDump(c));
          break;
        }

        case CommandId::Run: {
          int n = toInt(t[1]);
          for (int i = 0; i < n && !cpu_->halted(); i++) cpu_->step();
          emitOutput("ran " + std::to_string(n) + " steps");
          break;
        }

        case CommandId::Step:
          cpu_->step();
          emitOutput("ok");
          break;

        case CommandId::Glyph: {
          int r = toInt(t[1]);
          auto g = activeGlyphFromOffset(m_.ring(r).offset(), m_.gearsPerRing());
          emitOutput("Ring " + std::to_string(r) + " active glyph = " + glyphName(g));
          break;
        }

        case CommandId::Dial: {
          int r = toInt(t[1]);
          auto target = parseGlyph(t[2]);
          dialRingToGlyph(r, target);
          auto g = activeGlyphFromOffset(m_.ring(r).offset(), m_.gearsPerRing());
          emitOutput("Ring " + std::to_string(r) + " active glyph = " + glyphName(g));
          emitEvent("dial ring=" + std::to_string(r) + " -> " + glyphName(target));
          break;
        }

        case CommandId::Clock: {
          std::string sub = (t.size() >= 2) ? t[1] : "status";
          std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
          if (sub == "status") {
            emitOutput("clock status: commands=" + std::to_string(clock_.commandCount()) +
                       " total_ticks=" + std::to_string(clock_.gearTicks()));
          } else if (sub == "tick") {
            int k = (t.size() >= 3) ? toInt(t[2]) : 1;
            m_.tickAll(k);
            clock_.onGearTick(k);
            emitEvent("clock tick +" + std::to_string(k) + " (total=" + std::to_string(clock_.gearTicks()) + ")");
          } else {
            throw std::invalid_argument("clock supports: status | tick [n]");
          }
          break;
        }

        case CommandId::Calc: {
          if (t.size() < 2) {
            throw std::invalid_argument("calc requires a subcommand");
          }
          std::string sub = t[1];
          std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);

          if (sub == "eval") {
            const std::string expr = joinTokens(t, 2);
            const double v = calc::eval_expr(expr);
            std::ostringstream oss;
            oss << std::setprecision(15) << v;
            emitOutput(oss.str());
          } else if (sub == "evalx") {
            if (t.size() < 4) throw std::invalid_argument("calc evalx <x> <expr>");
            const double x = std::stod(t[2]);
            const std::string expr = joinTokens(t, 3);
            const double v = calc::eval_expr(expr, x);
            std::ostringstream oss;
            oss << std::setprecision(15) << v;
            emitOutput(oss.str());
          } else if (sub == "deriv") {
            if (t.size() < 4) throw std::invalid_argument("calc deriv <x> <expr>");
            const double x = std::stod(t[2]);
            const std::string expr = joinTokens(t, 3);
            const double v = calc::derivative(expr, x);
            std::ostringstream oss;
            oss << "d/dx|x=" << std::setprecision(8) << x << " -> " << std::setprecision(15) << v;
            emitOutput(oss.str());
          } else if (sub == "integ") {
            if (t.size() < 6) throw std::invalid_argument("calc integ <a> <b> <n> <expr>");
            const double a = std::stod(t[2]);
            const double b = std::stod(t[3]);
            const int n = toInt(t[4]);
            const std::string expr = joinTokens(t, 5);
            const double v = calc::integrate(expr, a, b, n);
            std::ostringstream oss;
            oss << "Integral[" << std::setprecision(8) << a << "," << b << "] = " << std::setprecision(15) << v;
            emitOutput(oss.str());
          } else if (sub == "quad") {
            if (t.size() < 5) throw std::invalid_argument("calc quad <a> <b> <c>");
            const double a = std::stod(t[2]);
            const double b = std::stod(t[3]);
            const double c = std::stod(t[4]);
            const auto qr = calc::solve_quadratic(a, b, c);
            std::ostringstream oss;
            if (qr.rootCount == 0) {
              oss << "No roots";
            } else if (qr.realRoots) {
              if (qr.rootCount == 1) {
                oss << "x = " << std::setprecision(15) << qr.x1;
              } else {
                oss << "x1 = " << std::setprecision(15) << qr.x1 << ", x2 = " << qr.x2;
              }
            } else {
              oss << "x = " << std::setprecision(15) << qr.x1 << " +/- " << qr.imag << "i";
            }
            emitOutput(oss.str());
          } else if (sub == "solve") {
            const std::string eq = joinTokens(t, 2);
            const auto res = calc::solve_linear_equation(eq);
            if (res.kind == calc::LinearSolveKind::OneSolution) {
              std::ostringstream oss;
              oss << "x = " << std::setprecision(15) << res.x;
              emitOutput(oss.str());
            } else if (res.kind == calc::LinearSolveKind::InfiniteSolutions) {
              emitOutput("Infinite solutions");
            } else {
              emitOutput("No solution");
            }
          } else {
            throw std::invalid_argument("calc subcommands: eval|evalx|deriv|integ|quad|solve");
          }
          break;
        }

        case CommandId::Equation: {
          const std::string eq = joinTokens(t, 1);
          const auto res = calc::solve_linear_equation(eq);
          if (res.kind == calc::LinearSolveKind::OneSolution) {
            std::ostringstream oss;
            oss << "x = " << std::setprecision(15) << res.x;
            emitOutput(oss.str());
          } else if (res.kind == calc::LinearSolveKind::InfiniteSolutions) {
            emitOutput("Infinite solutions");
          } else {
            emitOutput("No solution");
          }
          break;
        }

        case CommandId::Windows:
          std::cout << windows_.renderAll();
          break;

        case CommandId::Unknown:
          emitOutput("Unknown command. Type 'help'.");
          break;
      }
    } catch (const std::exception& e) {
      emitOutput(std::string("Error: ") + e.what());
    }
  }
}

} // namespace wa
