#include <algorithm> // std::find
#include <deque>
#include <random> // std::mt19937, std::uniform_int_distribution
#include <utility> // std::move

#include <ncurses.h>

#include "window.hpp"
WINDOW *createwindow(int h, int w, int y, int x);

struct coords {
  int y;
  int x;

  friend bool operator==(coords &lhs, const coords &rhs);
  friend coords &operator+=(coords &lhs, const coords &rhs);
};
coords operator+(coords lhs, const coords &rhs) { return lhs += rhs; }

enum class keys {
  UP    = KEY_UP,
  LEFT  = KEY_LEFT,
  DOWN  = KEY_DOWN,
  RIGHT = KEY_RIGHT
};

enum class direction {
  UP    = 0,
  LEFT  = 1,
  DOWN  = 2,
  RIGHT = 3
};

enum class game_status {
  MENU,
  RUNNING,
  PAUSED,
  GAME_OVER
};

struct game_state {
  direction dir;
  int score;
  game_status status;
  coords head;
  std::size_t snake_max_length;
  std::deque<coords> body;
  std::size_t max_food;
  std::deque<coords> food;
  int min_x;
  int min_y;
  int max_x;
  int max_y;
  std::mt19937 gen;
  std::uniform_int_distribution<> x_dist;
  std::uniform_int_distribution<> y_dist;
};

struct window_set {
  window info_box;
  window bottom_border;
  window menu;
  window game;
};

game_state default_game_state(window_set &windows);

void handle_menu(int ch, game_state &state, window_set &windows);
void handle_running(int ch, game_state &state, window_set &windows);
void handle_paused(int ch, game_state &state, window_set &windows);
void handle_game_over(int ch, game_state &state, window_set &windows);

int main() {
  ncw _;
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, true);
  halfdelay(2);
  refresh();

  // windows
  window_set windows = {
    {0, 0, 20, 5},
    {0, 4, 20, 10},
    {1, 5, 18, 8},
    {1, 5, 18, 8}
  };

  game_state current_state = default_game_state(windows);

  box(windows.info_box, 0, 0);
  mvwprintw(windows.info_box, 1, 1, "Snake!");
  wrefresh(windows.info_box);

  wborder(windows.bottom_border, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE, 0, 0);
  wrefresh(windows.bottom_border);

  for (int ch = getch(); ch != KEY_F(4); ch = getch()) {
    switch (current_state.status) {
      case game_status::MENU:
        handle_menu(ch, current_state, windows);
        break;
      case game_status::RUNNING:
        handle_running(ch, current_state, windows);
        break;
      case game_status::PAUSED:
        handle_paused(ch, current_state, windows);
        break;
      case game_status::GAME_OVER:
        handle_game_over(ch, current_state, windows);
        break;
    }
  }

  return 0;
}

game_state default_game_state(window_set &windows) {
  game_state state;

  state.dir = direction::RIGHT;
  state.score = 0;
  state.status = game_status::MENU;
  state.head = {0, 0};
  state.snake_max_length = 3;
  state.body = {};
  state.max_food = 1;
  state.food = {};
  state.min_x = 0;
  state.min_y = 0;

  // set may x/y
  getmaxyx(
    static_cast<WINDOW *>(windows.game),
    state.max_y, state.max_x
  );
  --state.max_x;
  --state.max_y;

  // set rng distributions
  state.x_dist = std::uniform_int_distribution<>{state.min_x, state.max_x};
  state.y_dist = std::uniform_int_distribution<>{state.min_y, state.max_y};

  return state;
}

void handle_menu(int ch, game_state &state, window_set &windows) {
  state = default_game_state(windows);

  wclear(windows.game);
  wrefresh(windows.game);

  mvwprintw(windows.menu, 0, 0, "SNAKE!");
  mvwprintw(windows.menu, 2, 0, "Eat food to grow!");
  mvwprintw(windows.menu, 3, 0, "Avoid the walls!");
  mvwprintw(windows.menu, 4, 0, "Avoid yourself!");
  mvwprintw(windows.menu, 5, 0, "Press F4 to quit,");
  mvwprintw(windows.menu, 6, 0, "Space to play, and");
  mvwprintw(windows.menu, 7, 0, "Arrows to move!");
  wrefresh(windows.menu);

  switch (ch) {
    case ' ':
    state.status = game_status::RUNNING;
    wclear(windows.menu);
    wrefresh(windows.menu);
    break;
  }
}

void handle_running(int ch, game_state &state, window_set &windows) {
  switch (ch) {
    case ' ': state.status = game_status::PAUSED; break;
  }
  wclear(windows.menu);
  wrefresh(windows.menu);

  if (ch != ERR) {
    switch (keys(ch)) {
      case keys::UP: state.dir = direction::UP; break;
      case keys::LEFT: state.dir = direction::LEFT; break;
      case keys::DOWN: state.dir = direction::DOWN; break;
      case keys::RIGHT: state.dir = direction::RIGHT; break;
    }
  }

  switch (state.dir) {
    case direction::UP: state.head += {-1, 0}; break;
    case direction::LEFT: state.head += {0, -1}; break;
    case direction::DOWN: state.head += {1, 0}; break;
    case direction::RIGHT: state.head += {0, 1}; break;
  }

  // collision with wall
  if (
    (state.head.x < state.min_x) || (state.head.x > state.max_x) ||
    (state.head.y < state.min_y) || (state.head.y > state.max_y)
  ) {
    state.status = game_status::GAME_OVER;
  }

  // collision with self
  auto bit = std::find(state.body.begin(), state.body.end(), state.head);
  if (bit != state.body.end()) {
    state.status = game_status::GAME_OVER;
  }

  // generate food
  while (state.food.size() < state.max_food) {
    int x_pos = state.x_dist(state.gen);
    int y_pos = state.y_dist(state.gen);

    state.food.push_back({y_pos, x_pos});
  }

  // draw food
  for (const auto [y, x] : state.food) {
    mvwaddch(windows.game, y, x, '+');
  }

  // eat food
  auto fit = std::find(
    state.food.begin(), state.food.end(),
    state.head
  );
  if (fit != state.food.end()) {
    state.food.erase(fit);
    state.score += 1;
    state.snake_max_length += 1;
  }

  // draw snake
  state.body.push_back(state.head);
  if (state.body.size() > state.snake_max_length) {
    auto f = state.body.front();
    mvwaddch(windows.game, f.y, f.x, ' ');
    state.body.pop_front();
  }

  for (const auto [y, x] : state.body) {
    mvwaddch(windows.game, y, x, '#');
  }

  mvwprintw(windows.info_box, 3, 1, "Score = %3d", state.score);
  mvwaddch(windows.game, state.head.y, state.head.x, '@');

  wrefresh(windows.info_box);
  wrefresh(windows.game);
}

void handle_paused(int ch, game_state &state, window_set &windows) {
  switch (ch) {
    case ' ': state.status = game_status::RUNNING; break;
  }
  mvwprintw(windows.menu, 3, 3, "PAUSED");
  wrefresh(windows.menu);
}

void handle_game_over(int ch, game_state &state, window_set &windows) {
  switch (ch) {
    case KEY_F(2): state.status = game_status::MENU; break;
  }

  mvwprintw(windows.menu, 3, 3, "GAME OVER");
  mvwprintw(windows.menu, 5, 3, "F2 to restart!");
  wrefresh(windows.menu);
}

bool operator==(coords &lhs, const coords &rhs) {
  return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

coords &operator+=(coords &lhs, const coords &rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;

  return lhs;
}
