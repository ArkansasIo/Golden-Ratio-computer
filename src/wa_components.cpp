#include "wolfman_alpha/wa_components.hpp"
#include <sstream>
#include <stdexcept>

namespace wa {

namespace {

std::vector<u8> u64ToBits(u64 v, int width) {
  if (width < 0) throw std::invalid_argument("width must be >= 0");
  std::vector<u8> out(static_cast<std::size_t>(width), 0);
  const int n = (width < 64) ? width : 64;
  for (int i = 0; i < n; ++i) out[static_cast<std::size_t>(i)] = static_cast<u8>((v >> i) & 1ull);
  return out;
}

u64 bitsToU64(const std::vector<u8>& bits) {
  const std::size_t n = (bits.size() < 64u) ? bits.size() : 64u;
  u64 v = 0;
  for (std::size_t i = 0; i < n; ++i) v |= (static_cast<u64>(bits[i] & 1u) << i);
  return v;
}

std::vector<u8> bitwise(const std::vector<u8>& a, const std::vector<u8>& b, char op) {
  if (a.size() != b.size()) throw std::invalid_argument("word size mismatch");
  std::vector<u8> out(a.size(), 0);
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (op == '&') out[i] = static_cast<u8>((a[i] & b[i]) & 1u);
    else if (op == '|') out[i] = static_cast<u8>((a[i] | b[i]) & 1u);
    else out[i] = static_cast<u8>((a[i] ^ b[i]) & 1u);
  }
  return out;
}

std::vector<u8> addWords(const std::vector<u8>& a, const std::vector<u8>& b) {
  if (a.size() != b.size()) throw std::invalid_argument("word size mismatch");
  std::vector<u8> out(a.size(), 0);
  u8 carry = 0;
  for (std::size_t i = 0; i < a.size(); ++i) {
    const u8 A = a[i] & 1u;
    const u8 B = b[i] & 1u;
    out[i] = static_cast<u8>(A ^ B ^ carry);
    carry = static_cast<u8>((A & B) | (A & carry) | (B & carry));
  }
  return out;
}

} // namespace

MechanicalClock::MechanicalClock(double hz) : hz_(hz) {
  if (hz_ <= 0.0) throw std::invalid_argument("clock hz must be > 0");
}

void MechanicalClock::tick(std::uint64_t n) {
  if (running_) ticks_ += n;
}

GearBus::GearBus(int widthBits) : widthBits_(widthBits), bits_(static_cast<std::size_t>(widthBits), 0) {
  if (widthBits_ <= 0) throw std::invalid_argument("bus width must be > 0");
}

void GearBus::writeBits(const std::vector<u8>& bits) {
  if (static_cast<int>(bits.size()) != widthBits_) throw std::invalid_argument("bus write width mismatch");
  bits_ = bits;
  for (auto& b : bits_) b &= 1u;
}

void GearBus::clear() {
  std::fill(bits_.begin(), bits_.end(), 0);
}

GearMemory::GearMemory(int words, int wordBits) : words_(words), wordBits_(wordBits) {
  if (words_ <= 0 || wordBits_ <= 0) throw std::invalid_argument("memory geometry must be positive");
  cells_.assign(static_cast<std::size_t>(words_), std::vector<u8>(static_cast<std::size_t>(wordBits_), 0));
}

void GearMemory::checkAddr(int addr) const {
  if (addr < 0 || addr >= words_) throw std::out_of_range("memory address out of range");
}

void GearMemory::clear() {
  for (auto& w : cells_) std::fill(w.begin(), w.end(), 0);
}

std::vector<u8> GearMemory::readWord(int addr) const {
  checkAddr(addr);
  return cells_[static_cast<std::size_t>(addr)];
}

void GearMemory::writeWord(int addr, const std::vector<u8>& bits) {
  checkAddr(addr);
  if (static_cast<int>(bits.size()) != wordBits_) throw std::invalid_argument("memory write width mismatch");
  cells_[static_cast<std::size_t>(addr)] = bits;
  for (auto& b : cells_[static_cast<std::size_t>(addr)]) b &= 1u;
}

u64 GearMemory::readU64(int addr) const {
  return bitsToU64(readWord(addr));
}

void GearMemory::writeU64(int addr, u64 value) {
  writeWord(addr, u64ToBits(value, wordBits_));
}

GearStorage::GearStorage(int words, int wordBits) : GearMemory(words, wordBits) {}

void GearStorage::seek(int addr) {
  checkAddr(addr);
  head_ = addr;
}

GearRegisterBank::GearRegisterBank(int count, int wordBits) : count_(count), wordBits_(wordBits) {
  if (count_ <= 0 || wordBits_ <= 0) throw std::invalid_argument("register geometry must be positive");
  regs_.assign(static_cast<std::size_t>(count_), std::vector<u8>(static_cast<std::size_t>(wordBits_), 0));
}

void GearRegisterBank::checkReg(int r) const {
  if (r < 0 || r >= count_) throw std::out_of_range("register index out of range");
}

void GearRegisterBank::reset() {
  for (auto& w : regs_) std::fill(w.begin(), w.end(), 0);
}

u64 GearRegisterBank::getU64(int r) const {
  checkReg(r);
  return bitsToU64(regs_[static_cast<std::size_t>(r)]);
}

void GearRegisterBank::setU64(int r, u64 value) {
  checkReg(r);
  regs_[static_cast<std::size_t>(r)] = u64ToBits(value, wordBits_);
}

GearCPUCore::GearCPUCore(GearRegisterBank& regs, GearRAM& ram, GearStorage& storage, GearBus& bus, MechanicalClock& clock)
  : regs_(regs), ram_(ram), storage_(storage), bus_(bus), clock_(clock) {}

void GearCPUCore::loadProgram(std::vector<GearInstr> program) {
  program_ = std::move(program);
  ip_ = 0;
  halted_ = false;
}

void GearCPUCore::reset() {
  ip_ = 0;
  halted_ = false;
}

void GearCPUCore::step() {
  if (halted_) return;
  if (ip_ >= program_.size()) {
    halted_ = true;
    return;
  }

  const GearInstr& ins = program_[ip_];
  switch (ins.op) {
    case GearOp::NOP:
      break;

    case GearOp::MOVI:
      regs_.setU64(ins.a, static_cast<u64>(ins.imm));
      break;

    case GearOp::MOV:
      regs_.setU64(ins.a, regs_.getU64(ins.b));
      break;

    case GearOp::LOAD: {
      const int addr = static_cast<int>(regs_.getU64(ins.b)) % ram_.words();
      const auto w = ram_.readWord(addr);
      bus_.writeBits(w);
      regs_.setU64(ins.a, bitsToU64(bus_.readBits()));
      break;
    }

    case GearOp::STORE: {
      const int addr = static_cast<int>(regs_.getU64(ins.b)) % ram_.words();
      const auto bits = u64ToBits(regs_.getU64(ins.a), bus_.widthBits());
      bus_.writeBits(bits);
      ram_.writeWord(addr, bus_.readBits());
      break;
    }

    case GearOp::ADD: {
      const auto A = u64ToBits(regs_.getU64(ins.b), bus_.widthBits());
      const auto B = u64ToBits(regs_.getU64(ins.c), bus_.widthBits());
      const auto S = addWords(A, B);
      bus_.writeBits(S);
      regs_.setU64(ins.a, bitsToU64(bus_.readBits()));
      break;
    }

    case GearOp::AND: {
      const auto A = u64ToBits(regs_.getU64(ins.b), bus_.widthBits());
      const auto B = u64ToBits(regs_.getU64(ins.c), bus_.widthBits());
      bus_.writeBits(bitwise(A, B, '&'));
      regs_.setU64(ins.a, bitsToU64(bus_.readBits()));
      break;
    }

    case GearOp::OR: {
      const auto A = u64ToBits(regs_.getU64(ins.b), bus_.widthBits());
      const auto B = u64ToBits(regs_.getU64(ins.c), bus_.widthBits());
      bus_.writeBits(bitwise(A, B, '|'));
      regs_.setU64(ins.a, bitsToU64(bus_.readBits()));
      break;
    }

    case GearOp::XOR: {
      const auto A = u64ToBits(regs_.getU64(ins.b), bus_.widthBits());
      const auto B = u64ToBits(regs_.getU64(ins.c), bus_.widthBits());
      bus_.writeBits(bitwise(A, B, '^'));
      regs_.setU64(ins.a, bitsToU64(bus_.readBits()));
      break;
    }

    case GearOp::JMP:
      ip_ = static_cast<std::size_t>(ins.imm);
      clock_.tick(1);
      return;

    case GearOp::JZ:
      if (regs_.getU64(ins.a) == 0) {
        ip_ = static_cast<std::size_t>(ins.imm);
        clock_.tick(1);
        return;
      }
      break;

    case GearOp::HALT:
      halted_ = true;
      break;
  }

  ip_++;
  clock_.tick(1);
}

MechanicalComputer::MechanicalComputer(MechanicalConfig cfg)
  : cfg_(cfg),
    clock_(cfg.clockHz),
    bus_(cfg.wordBits),
    regs_(cfg.registers, cfg.wordBits),
    ram_(cfg.ramWords, cfg.wordBits),
    storage_(cfg.storageWords, cfg.wordBits),
    cpu_(regs_, ram_, storage_, bus_, clock_) {}

std::string MechanicalComputer::summary() const {
  std::ostringstream oss;
  oss << "MechanicalComputer"
      << " clockHz=" << cfg_.clockHz
      << " wordBits=" << cfg_.wordBits
      << " regs=" << cfg_.registers
      << " ramWords=" << cfg_.ramWords
      << " storageWords=" << cfg_.storageWords
      << " ticks=" << clock_.ticks();
  return oss.str();
}

} // namespace wa
