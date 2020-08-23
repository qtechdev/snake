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
  direction dir = direction::RIGHT;
  int score = 0;
  game_status status = game_status::MENU;
  coords head = {0, 0};
  std::size_t snake_max_length = 3;
  std::deque<coords> body = {};
  std::size_t max_food = 1;
  std::deque<coords> food = {};
};

int main() {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, true);
  halfdelay(2);
  refresh();

  game_state current_state;

  // windows
  window info_box(0, 0, 20, 5);
  window bottom_border(0, 4, 20, 10);
  window menu(1, 5, 18, 8);
  window game(1, 5, 18, 8);

  int min_x = 0;
  int min_y = 0;
  int max_x;
  int max_y;
  getmaxyx(static_cast<WINDOW *>(game), max_y, max_x);
  --max_x;
  --max_y;

  // rng
  std::mt19937 gen;
  std::uniform_int_distribution<> x_dist(min_x, max_x);
  std::uniform_int_distribution<> y_dist(min_y, max_y);

  box(info_box, 0, 0);
  mvwprintw(info_box, 1, 1, "Snake!");
  wrefresh(info_box);

  wborder(bottom_border, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE, 0, 0);
  wrefresh(bottom_border);

  for (int ch = getch(); ch != KEY_F(4); ch = getch()) {
    switch (current_state.status) {
      case game_status::MENU: {
        // reset game state
        current_state = game_state{};

        wclear(game);
        wrefresh(game);

        mvwprintw(menu, 0, 0, "SNAKE!");
        mvwprintw(menu, 2, 0, "Eat food to grow!");
        mvwprintw(menu, 3, 0, "Avoid the walls!");
        mvwprintw(menu, 4, 0, "Avoid yourself!");
        mvwprintw(menu, 5, 0, "Press F4 to quit,");
        mvwprintw(menu, 6, 0, "Space to play, and");
        mvwprintw(menu, 7, 0, "Arrows to move!");
        wrefresh(menu);

        switch (ch) {
          case ' ':
          current_state.status = game_status::RUNNING;
          wclear(menu);
          wrefresh(menu);
          break;
        }

      } break;
      case game_status::RUNNING: {
        switch (ch) {
          case ' ': current_state.status = game_status::PAUSED; break;
        }
        wclear(menu);
        wrefresh(menu);

        if (ch != ERR) {
          switch (keys(ch)) {
            case keys::UP: current_state.dir = direction::UP; break;
            case keys::LEFT: current_state.dir = direction::LEFT; break;
            case keys::DOWN: current_state.dir = direction::DOWN; break;
            case keys::RIGHT: current_state.dir = direction::RIGHT; break;
          }
        }

        switch (current_state.dir) {
          case direction::UP: current_state.head += {-1, 0}; break;
          case direction::LEFT: current_state.head += {0, -1}; break;
          case direction::DOWN: current_state.head += {1, 0}; break;
          case direction::RIGHT: current_state.head += {0, 1}; break;
        }

        // collision with wall
        if (
          (current_state.head.x < min_x) || (current_state.head.x > max_x) ||
          (current_state.head.y < min_y) || (current_state.head.y > max_y)
        ) {
          current_state.status = game_status::GAME_OVER;
        }

        // collision with self
        auto bit = std::find(
          current_state.body.begin(), current_state.body.end(),
          current_state.head
        );
        if (bit != current_state.body.end()) {
          current_state.status = game_status::GAME_OVER;
        }

        // generate food
        while (current_state.food.size() < current_state.max_food) {
          int x_pos = x_dist(gen);
          int y_pos = y_dist(gen);

          current_state.food.push_back({y_pos, x_pos});
        }

        // draw food
        for (const auto [y, x] : current_state.food) {
          mvwaddch(game, y, x, '+');
        }

        // eat food
        auto fit = std::find(
          current_state.food.begin(), current_state.food.end(),
          current_state.head
        );
        if (fit != current_state.food.end()) {
          current_state.food.erase(fit);
          current_state.score += 1;
          current_state.snake_max_length += 1;
        }

        // draw snake
        current_state.body.push_back(current_state.head);
        if (current_state.body.size() > current_state.snake_max_length) {
          auto f = current_state.body.front();
          mvwaddch(game, f.y, f.x, ' ');
          current_state.body.pop_front();
        }

        for (const auto [y, x] : current_state.body) {
          mvwaddch(game, y, x, '#');
        }

        mvwprintw(info_box, 3, 1, "Score = %3d", current_state.score);
        mvwaddch(game, current_state.head.y, current_state.head.x, '@');

        wrefresh(info_box);
        wrefresh(game);
      } break;
      case game_status::PAUSED: {
        switch (ch) {
          case ' ': current_state.status = game_status::RUNNING; break;
        }
        mvwprintw(menu, 3, 3, "PAUSED");
        wrefresh(menu);
      } break;
      case game_status::GAME_OVER: {
        switch (ch) {
          case KEY_F(2): current_state.status = game_status::MENU; break;
        }

        mvwprintw(menu, 3, 3, "GAME OVER");
        mvwprintw(menu, 5, 3, "F2 to restart!");
        wrefresh(menu);

      } break;
    }
  }

  endwin();
  return 0;
}

bool operator==(coords &lhs, const coords &rhs) {
  return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

coords &operator+=(coords &lhs, const coords &rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;

  return lhs;
}
