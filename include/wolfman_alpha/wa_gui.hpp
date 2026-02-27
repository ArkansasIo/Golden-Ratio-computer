#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace wa::gui {

using WidgetId = std::uint64_t;

struct Vec2i {
  int x{0};
  int y{0};
};

struct Rect {
  int x{0};
  int y{0};
  int w{0};
  int h{0};

  bool contains(int px, int py) const {
    return px >= x && py >= y && px < (x + w) && py < (y + h);
  }
};

enum class EventType {
  None,
  MouseDown,
  MouseUp,
  MouseMove,
  KeyDown,
  KeyUp,
  TextInput,
  Click,
  ValueChanged
};

struct Event {
  EventType type{EventType::None};
  WidgetId target{0};
  int mouseX{0};
  int mouseY{0};
  int keyCode{0};
  std::string text;
};

enum class WidgetKind {
  Label,
  Button,
  Input,
  Switch,
  Panel
};

class Widget {
public:
  Widget(WidgetId id, WidgetKind kind, std::string text, Rect bounds);
  virtual ~Widget() = default;

  WidgetId id() const { return id_; }
  WidgetKind kind() const { return kind_; }
  const std::string& text() const { return text_; }
  void setText(std::string value) { text_ = std::move(value); }
  const Rect& bounds() const { return bounds_; }
  void setBounds(Rect r) { bounds_ = r; }
  bool visible() const { return visible_; }
  void setVisible(bool v) { visible_ = v; }
  bool enabled() const { return enabled_; }
  void setEnabled(bool e) { enabled_ = e; }

  virtual bool handleEvent(const Event& ev);

private:
  WidgetId id_{0};
  WidgetKind kind_{WidgetKind::Label};
  std::string text_;
  Rect bounds_;
  bool visible_{true};
  bool enabled_{true};
};

class ButtonWidget final : public Widget {
public:
  ButtonWidget(WidgetId id, std::string text, Rect bounds);
  bool handleEvent(const Event& ev) override;
};

class InputWidget final : public Widget {
public:
  InputWidget(WidgetId id, std::string text, Rect bounds);
  bool handleEvent(const Event& ev) override;
};

class SwitchWidget final : public Widget {
public:
  SwitchWidget(WidgetId id, std::string text, Rect bounds, bool initialOn = false);
  bool handleEvent(const Event& ev) override;
  bool on() const { return on_; }
  void setOn(bool value) { on_ = value; }

private:
  bool on_{false};
};

class GuiWindow {
public:
  GuiWindow(std::string title, Rect bounds);

  const std::string& title() const { return title_; }
  void setTitle(std::string title) { title_ = std::move(title); }
  const Rect& bounds() const { return bounds_; }
  void setBounds(Rect r) { bounds_ = r; }

  WidgetId addLabel(const std::string& text, Rect bounds);
  WidgetId addButton(const std::string& text, Rect bounds);
  WidgetId addInput(const std::string& text, Rect bounds);
  WidgetId addSwitch(const std::string& text, Rect bounds, bool initialOn = false);
  Widget* find(WidgetId id);
  const Widget* find(WidgetId id) const;
  std::vector<WidgetId> widgetIds() const;
  bool dispatch(const Event& ev);

private:
  WidgetId nextId_{1};
  std::string title_;
  Rect bounds_;
  std::unordered_map<WidgetId, std::unique_ptr<Widget>> widgets_;
};

class GuiBackend {
public:
  virtual ~GuiBackend() = default;

  virtual bool init() = 0;
  virtual void shutdown() = 0;
  virtual std::vector<Event> pollEvents() = 0;
  virtual void draw(const GuiWindow& window) = 0;
};

class MockBackend final : public GuiBackend {
public:
  bool init() override;
  void shutdown() override;
  std::vector<Event> pollEvents() override;
  void draw(const GuiWindow& window) override;

  void queueEvent(const Event& ev);
  const std::vector<std::string>& frameLog() const { return frameLog_; }

private:
  bool ready_{false};
  std::vector<Event> pending_;
  std::vector<std::string> frameLog_;
};

class GuiApp {
public:
  using EventHook = std::function<void(const Event&)>;

  GuiApp(std::unique_ptr<GuiBackend> backend, GuiWindow window);

  bool start();
  void stop();
  void runFrame();
  void onEvent(EventHook hook) { hook_ = std::move(hook); }

  GuiWindow& window() { return window_; }
  const GuiWindow& window() const { return window_; }
  bool running() const { return running_; }

private:
  std::unique_ptr<GuiBackend> backend_;
  GuiWindow window_;
  bool running_{false};
  EventHook hook_{};
};

} // namespace wa::gui
