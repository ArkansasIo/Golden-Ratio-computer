#include "wolfman_alpha/wa_gui.hpp"
#include <sstream>

namespace wa::gui {

Widget::Widget(WidgetId id, WidgetKind kind, std::string text, Rect bounds)
  : id_(id), kind_(kind), text_(std::move(text)), bounds_(bounds) {}

bool Widget::handleEvent(const Event&) {
  return false;
}

ButtonWidget::ButtonWidget(WidgetId id, std::string text, Rect bounds)
  : Widget(id, WidgetKind::Button, std::move(text), bounds) {}

bool ButtonWidget::handleEvent(const Event& ev) {
  if (!enabled() || !visible()) return false;
  if (ev.type == EventType::Click && ev.target == id()) return true;
  return false;
}

InputWidget::InputWidget(WidgetId id, std::string text, Rect bounds)
  : Widget(id, WidgetKind::Input, std::move(text), bounds) {}

bool InputWidget::handleEvent(const Event& ev) {
  if (!enabled() || !visible()) return false;
  if (ev.type == EventType::TextInput && ev.target == id()) {
    setText(text() + ev.text);
    return true;
  }
  return false;
}

SwitchWidget::SwitchWidget(WidgetId id, std::string text, Rect bounds, bool initialOn)
  : Widget(id, WidgetKind::Switch, std::move(text), bounds), on_(initialOn) {}

bool SwitchWidget::handleEvent(const Event& ev) {
  if (!enabled() || !visible()) return false;
  if (ev.type == EventType::Click && ev.target == id()) {
    on_ = !on_;
    return true;
  }
  return false;
}

GuiWindow::GuiWindow(std::string title, Rect bounds) : title_(std::move(title)), bounds_(bounds) {}

WidgetId GuiWindow::addLabel(const std::string& text, Rect bounds) {
  const WidgetId id = nextId_++;
  widgets_.emplace(id, std::make_unique<Widget>(id, WidgetKind::Label, text, bounds));
  return id;
}

WidgetId GuiWindow::addButton(const std::string& text, Rect bounds) {
  const WidgetId id = nextId_++;
  widgets_.emplace(id, std::make_unique<ButtonWidget>(id, text, bounds));
  return id;
}

WidgetId GuiWindow::addInput(const std::string& text, Rect bounds) {
  const WidgetId id = nextId_++;
  widgets_.emplace(id, std::make_unique<InputWidget>(id, text, bounds));
  return id;
}

WidgetId GuiWindow::addSwitch(const std::string& text, Rect bounds, bool initialOn) {
  const WidgetId id = nextId_++;
  widgets_.emplace(id, std::make_unique<SwitchWidget>(id, text, bounds, initialOn));
  return id;
}

Widget* GuiWindow::find(WidgetId id) {
  auto it = widgets_.find(id);
  if (it == widgets_.end()) return nullptr;
  return it->second.get();
}

const Widget* GuiWindow::find(WidgetId id) const {
  auto it = widgets_.find(id);
  if (it == widgets_.end()) return nullptr;
  return it->second.get();
}

std::vector<WidgetId> GuiWindow::widgetIds() const {
  std::vector<WidgetId> ids;
  ids.reserve(widgets_.size());
  for (const auto& kv : widgets_) ids.push_back(kv.first);
  return ids;
}

bool GuiWindow::dispatch(const Event& ev) {
  if (Widget* w = find(ev.target)) return w->handleEvent(ev);
  return false;
}

bool MockBackend::init() {
  ready_ = true;
  frameLog_.clear();
  return true;
}

void MockBackend::shutdown() {
  ready_ = false;
}

std::vector<Event> MockBackend::pollEvents() {
  std::vector<Event> out;
  out.swap(pending_);
  return out;
}

void MockBackend::draw(const GuiWindow& window) {
  if (!ready_) return;
  std::ostringstream oss;
  oss << "draw window \"" << window.title() << "\" widgets=" << window.widgetIds().size();
  frameLog_.push_back(oss.str());
}

void MockBackend::queueEvent(const Event& ev) {
  pending_.push_back(ev);
}

GuiApp::GuiApp(std::unique_ptr<GuiBackend> backend, GuiWindow window)
  : backend_(std::move(backend)), window_(std::move(window)) {}

bool GuiApp::start() {
  if (!backend_) return false;
  running_ = backend_->init();
  return running_;
}

void GuiApp::stop() {
  if (backend_) backend_->shutdown();
  running_ = false;
}

void GuiApp::runFrame() {
  if (!running_ || !backend_) return;
  const auto events = backend_->pollEvents();
  for (const auto& ev : events) {
    window_.dispatch(ev);
    if (hook_) hook_(ev);
  }
  backend_->draw(window_);
}

} // namespace wa::gui
