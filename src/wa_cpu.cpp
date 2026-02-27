#include "wolfman_alpha/wa_cpu.hpp"
#include <sstream>

namespace wa {

static WordRef makeReg(WordSize ws, int regIndex) {
  if (ws == WordSize::W64)  return WordRef::w64(regIndex, 0);
  if (ws == WordSize::W360) return WordRef::w360(regIndex);
  return WordRef::w720(regIndex*2, regIndex*2 + 1);
}

CpuBase::CpuBase(Machine& m, CpuConfig cfg) : m_(m), cfg_(cfg) {
  R_.reserve(cfg_.regs);
  for (int i=0;i<cfg_.regs;i++) R_.push_back(makeReg(cfg_.wordSize, i));
}

void CpuBase::step() {
  if (halted_) return;
  if (ip_ >= prog_.size()) { halted_ = true; return; }

  // Zodiac hook example: glyph Z12 causes an extra tick before executing
  if (cfg_.useZodiac && m_.ringCount() > 0) {
    auto g = activeGlyphFromOffset(m_.ring(0).offset(), m_.gearsPerRing());
    if (g == Zodiac13::Z12) m_.tickAll(1);
  }

  exec(prog_[ip_]);
  ip_++;

  // mechanical clock tick
  m_.tickAll(1);
}

void CpuBase::exec(const Instr& ins) {
  switch (ins.op) {
    case Op::NOP: break;

    case Op::MOV: {
      const int n = word_bits(cfg_.wordSize);
      for (int i=0;i<n;i++) set_word_bit(m_, R_[ins.a], i, get_word_bit(m_, R_[ins.b], i));
      break;
    }

    case Op::XOR: alu_xor(m_, R_[ins.b], R_[ins.c], R_[ins.a]); break;
    case Op::AND: alu_and(m_, R_[ins.b], R_[ins.c], R_[ins.a]); break;
    case Op::OR:  alu_or (m_, R_[ins.b], R_[ins.c], R_[ins.a]); break;

    case Op::ADD: { (void)alu_add(m_, R_[ins.b], R_[ins.c], R_[ins.a]); break; }

    case Op::SHIFT_RING: { m_.shiftRing(ins.imm, ins.dir, ins.a); break; }
    case Op::TICK_ALL:   { m_.tickAll(ins.imm); break; }
    case Op::HALT:       { halted_ = true; break; }
  }
}

std::string CpuBase::regDump(int countBits) const {
  std::ostringstream oss;
  const int n = word_bits(cfg_.wordSize);
  const int c = (countBits > n) ? n : countBits;
  for (int r=0;r<(int)R_.size();r++) {
    oss << "R" << r << ": ";
    for (int i=0;i<c;i++) oss << int(get_word_bit(m_, R_[r], i));
    oss << "\n";
  }
  return oss.str();
}

CPU64::CPU64(Machine& m)  : CpuBase(m, CpuConfig{WordSize::W64,  8, true, true}) {}
CPU360::CPU360(Machine& m): CpuBase(m, CpuConfig{WordSize::W360, 8, true, true}) {}
CPU720::CPU720(Machine& m): CpuBase(m, CpuConfig{WordSize::W720, 8, true, true}) {}

} // namespace wa
