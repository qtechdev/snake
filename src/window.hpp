#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__

#include <ncurses.h>

class ncw {
public:
  ncw();
  ~ncw();

  // copy constructor/assignment
  ncw(const ncw &other) = delete;
  ncw &operator=(const ncw &other) = delete;

  // move constructor/assignment
  ncw(ncw &&other) = delete;
  ncw &operator=(ncw &&other) = delete;
};

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
  WINDOW *win = nullptr;
};

#endif // __WINDOW_HPP__
