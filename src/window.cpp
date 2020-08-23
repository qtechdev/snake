#include <utility> // std::swap

#include "window.hpp"

window::window() {
  win = stdscr;
}

window::window(int x, int y, int w, int h) {
  win = newwin(h, w, y, x);
}

window::~window() {
  if (win != nullptr && win != stdscr) {
    wclear(win);
    wrefresh(win);
    delwin(win);
  }
}

// move constructor/assignment
window::window(window &&other) noexcept {
  win = other.win;
  other.win = nullptr;
}

window &window::operator=(window &&other) noexcept {
  std::swap(win, other.win);
  return *this;
}

window::operator WINDOW *() const {
  return win;
}
