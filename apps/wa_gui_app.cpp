#include "wolfman_alpha/wa_components.hpp"
#include "wolfman_alpha/wa_gui.hpp"
#include <iostream>
#include <memory>
#include <sstream>

namespace {

struct UiIds {
  wa::gui::WidgetId statusLabel{0};
  wa::gui::WidgetId clockSwitch{0};
  wa::gui::WidgetId autoTickSwitch{0};
  wa::gui::WidgetId tickButton{0};
  wa::gui::WidgetId stepButton{0};
  wa::gui::WidgetId loadButton{0};
};

void setLabel(wa::gui::GuiWindow& w, wa::gui::WidgetId id, const std::string& text) {
  if (auto* widget = w.find(id)) widget->setText(text);
}

bool switchState(const wa::gui::GuiWindow& w, wa::gui::WidgetId id) {
  if (auto* base = w.find(id)) {
    if (base->kind() == wa::gui::WidgetKind::Switch) {
      const auto* sw = dynamic_cast<const wa::gui::SwitchWidget*>(base);
      return sw ? sw->on() : false;
    }
  }
  return false;
}

void setSwitch(wa::gui::GuiWindow& w, wa::gui::WidgetId id, bool on) {
  if (auto* base = w.find(id)) {
    if (base->kind() == wa::gui::WidgetKind::Switch) {
      if (auto* sw = dynamic_cast<wa::gui::SwitchWidget*>(base)) sw->setOn(on);
    }
  }
}

std::string statusText(const wa::MechanicalComputer& mech) {
  std::ostringstream oss;
  oss << "clk=" << mech.clock().ticks()
      << " running=" << (mech.clock().running() ? "ON" : "OFF")
      << " ip=" << mech.cpu().ip()
      << " halted=" << (mech.cpu().halted() ? "YES" : "NO")
      << " r0=" << mech.registers().getU64(0)
      << " ram[0]=" << mech.ram().readU64(0);
  return oss.str();
}

void loadDemoProgram(wa::MechanicalComputer& mech) {
  std::vector<wa::GearInstr> p;
  p.push_back({wa::GearOp::MOVI, 0, 0, 0, 1});  // R0=1
  p.push_back({wa::GearOp::MOVI, 1, 0, 0, 1});  // R1=1
  p.push_back({wa::GearOp::ADD,  0, 0, 1, 0});  // R0=R0+R1
  p.push_back({wa::GearOp::MOVI, 2, 0, 0, 0});  // R2=0 (addr)
  p.push_back({wa::GearOp::STORE,0, 2, 0, 0});  // RAM[R2]=R0
  p.push_back({wa::GearOp::JMP,  0, 0, 0, 2});  // loop ADD/STORE
  mech.cpu().loadProgram(std::move(p));
}

} // namespace

int main() {
  wa::MechanicalComputer mech({64, 8, 512, 8192, 24.0});

  wa::gui::GuiWindow window("WolfmanAlpha GUI Clockwork", {0, 0, 1024, 600});
  UiIds ids;
  ids.statusLabel = window.addLabel("status init", {24, 24, 960, 32});
  ids.clockSwitch = window.addSwitch("Clock Power", {24, 72, 180, 28}, true);
  ids.autoTickSwitch = window.addSwitch("Auto Tick", {220, 72, 180, 28}, false);
  ids.tickButton = window.addButton("Tick +1", {24, 120, 140, 32});
  ids.stepButton = window.addButton("CPU Step", {180, 120, 140, 32});
  ids.loadButton = window.addButton("Load Demo Program", {336, 120, 220, 32});
  setLabel(window, ids.statusLabel, statusText(mech));

  auto backend = std::make_unique<wa::gui::MockBackend>();
  auto* mock = backend.get();
  wa::gui::GuiApp app(std::move(backend), std::move(window));

  app.onEvent([&](const wa::gui::Event& ev) {
    switch (ev.target) {
      case 0:
        break;
      default:
        break;
    }

    if (ev.type == wa::gui::EventType::Click && ev.target == ids.clockSwitch) {
      if (switchState(app.window(), ids.clockSwitch)) mech.clock().start();
      else mech.clock().stop();
    } else if (ev.type == wa::gui::EventType::Click && ev.target == ids.tickButton) {
      mech.clock().tick(1);
    } else if (ev.type == wa::gui::EventType::Click && ev.target == ids.stepButton) {
      mech.cpu().step();
    } else if (ev.type == wa::gui::EventType::Click && ev.target == ids.loadButton) {
      loadDemoProgram(mech);
    }

    setLabel(app.window(), ids.statusLabel, statusText(mech));
  });

  if (!app.start()) {
    std::cerr << "GUI backend failed to start\n";
    return 1;
  }

  // Demo script for the mock backend: load program, step CPU, toggle auto tick, pulse ticks.
  mock->queueEvent({wa::gui::EventType::Click, ids.loadButton});
  mock->queueEvent({wa::gui::EventType::Click, ids.stepButton});
  mock->queueEvent({wa::gui::EventType::Click, ids.autoTickSwitch});
  mock->queueEvent({wa::gui::EventType::Click, ids.tickButton});
  mock->queueEvent({wa::gui::EventType::Click, ids.stepButton});

  for (int i = 0; i < 5; ++i) {
    if (switchState(app.window(), ids.autoTickSwitch)) mech.clock().tick(1);
    app.runFrame();
    setLabel(app.window(), ids.statusLabel, statusText(mech));
  }

  std::cout << "WolfmanAlpha GUI app initialized\n";
  std::cout << statusText(mech) << "\n";
  if (!switchState(app.window(), ids.clockSwitch)) setSwitch(app.window(), ids.clockSwitch, true);

  app.stop();
  return 0;
}
