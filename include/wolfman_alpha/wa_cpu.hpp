#pragma once
#include "wa_machine.hpp"
#include "wa_alu.hpp"
#include "wa_zodiac.hpp"
#include <vector>
#include <string>

namespace wa {

enum class Op : u8 { NOP, MOV, XOR, AND, OR, ADD, SHIFT_RING, TICK_ALL, HALT };

struct Instr {
  Op op{Op::NOP};
  int a{0}, b{0}, c{0};
  int imm{0};
  Dir dir{Dir::Right};
};

struct CpuConfig {
  WordSize wordSize{WordSize::W64};
  int regs{8};
  bool useZodiac{true};
  bool soundEvents{true};
};

class CpuBase {
public:
  CpuBase(Machine& m, CpuConfig cfg);

  void loadProgram(std::vector<Instr> p) { prog_ = std::move(p); ip_ = 0; halted_ = false; }
  bool halted() const { return halted_; }

  void step();
  std::string regDump(int countBits=64) const;

protected:
  Machine& m_;
  CpuConfig cfg_;
  std::vector<WordRef> R_;
  std::vector<Instr> prog_;
  std::size_t ip_{0};
  bool halted_{false};

  void exec(const Instr& ins);
};

class CPU64  : public CpuBase { public: explicit CPU64(Machine& m); };
class CPU360 : public CpuBase { public: explicit CPU360(Machine& m); };
class CPU720 : public CpuBase { public: explicit CPU720(Machine& m); };

} // namespace wa
