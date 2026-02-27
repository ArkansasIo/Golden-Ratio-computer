#pragma once
#include "wa_types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace wa {

struct MechanicalConfig {
  int wordBits{64};
  int registers{8};
  int ramWords{256};
  int storageWords{4096};
  double clockHz{12.0};
};

class MechanicalClock {
public:
  explicit MechanicalClock(double hz = 12.0);

  void start() { running_ = true; }
  void stop() { running_ = false; }
  bool running() const { return running_; }
  double hz() const { return hz_; }

  void tick(std::uint64_t n = 1);
  std::uint64_t ticks() const { return ticks_; }

private:
  double hz_{12.0};
  bool running_{true};
  std::uint64_t ticks_{0};
};

class GearBus {
public:
  explicit GearBus(int widthBits = 64);

  int widthBits() const { return widthBits_; }
  void writeBits(const std::vector<u8>& bits);
  const std::vector<u8>& readBits() const { return bits_; }
  void clear();

private:
  int widthBits_{64};
  std::vector<u8> bits_;
};

class GearMemory {
public:
  GearMemory(int words = 256, int wordBits = 64);

  int words() const { return words_; }
  int wordBits() const { return wordBits_; }

  void clear();
  std::vector<u8> readWord(int addr) const;
  void writeWord(int addr, const std::vector<u8>& bits);
  u64 readU64(int addr) const;
  void writeU64(int addr, u64 value);

protected:
  int words_{0};
  int wordBits_{0};
  std::vector<std::vector<u8>> cells_;

  void checkAddr(int addr) const;
};

class GearRAM : public GearMemory {
public:
  GearRAM(int words = 256, int wordBits = 64) : GearMemory(words, wordBits) {}
};

class GearStorage : public GearMemory {
public:
  GearStorage(int words = 4096, int wordBits = 64);

  void seek(int addr);
  int head() const { return head_; }

private:
  int head_{0};
};

class GearRegisterBank {
public:
  GearRegisterBank(int count = 8, int wordBits = 64);

  int count() const { return count_; }
  int wordBits() const { return wordBits_; }

  void reset();
  u64 getU64(int r) const;
  void setU64(int r, u64 value);

private:
  int count_{0};
  int wordBits_{0};
  std::vector<std::vector<u8>> regs_;

  void checkReg(int r) const;
};

enum class GearOp : u8 {
  NOP,
  MOVI,
  MOV,
  LOAD,
  STORE,
  ADD,
  AND,
  OR,
  XOR,
  JMP,
  JZ,
  HALT
};

struct GearInstr {
  GearOp op{GearOp::NOP};
  int a{0};
  int b{0};
  int c{0};
  int imm{0};
};

class GearCPUCore {
public:
  GearCPUCore(GearRegisterBank& regs, GearRAM& ram, GearStorage& storage, GearBus& bus, MechanicalClock& clock);

  void loadProgram(std::vector<GearInstr> program);
  void reset();
  bool halted() const { return halted_; }
  std::size_t ip() const { return ip_; }
  void step();

private:
  GearRegisterBank& regs_;
  GearRAM& ram_;
  GearStorage& storage_;
  GearBus& bus_;
  MechanicalClock& clock_;
  std::vector<GearInstr> program_;
  std::size_t ip_{0};
  bool halted_{false};
};

class MechanicalComputer {
public:
  explicit MechanicalComputer(MechanicalConfig cfg = {});

  MechanicalClock& clock() { return clock_; }
  GearBus& bus() { return bus_; }
  GearRegisterBank& registers() { return regs_; }
  GearRAM& ram() { return ram_; }
  GearStorage& storage() { return storage_; }
  GearCPUCore& cpu() { return cpu_; }

  const MechanicalClock& clock() const { return clock_; }
  const GearBus& bus() const { return bus_; }
  const GearRegisterBank& registers() const { return regs_; }
  const GearRAM& ram() const { return ram_; }
  const GearStorage& storage() const { return storage_; }
  const GearCPUCore& cpu() const { return cpu_; }

  std::string summary() const;

private:
  MechanicalConfig cfg_;
  MechanicalClock clock_;
  GearBus bus_;
  GearRegisterBank regs_;
  GearRAM ram_;
  GearStorage storage_;
  GearCPUCore cpu_;
};

} // namespace wa
