#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <ncurses.h>

class window {
public:
  window();
  window(int x, int y, int w, int h);
  ~window();

  // copy constructor/assignment
  window(const window &other) = delete;
  window &operator=(const window &other) = delete;

  // move constructor/assignment
  window(window &&other) noexcept;
  window &operator=(window &&other) noexcept;

  operator WINDOW *() const;
private:
  WINDOW *win;
};

#endif // __WINDOW_HPP__
