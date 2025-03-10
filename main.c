#define _POSIX_C_SOURCE 199309L
#include <math.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEFT 'a'
#define DOWN 's'
#define UP 'w'
#define RIGHT 'd'

// #define LEFT h
// #definne DOWN j
// #define UP k
// #define RIGHT l

typedef enum _Movement {
    Move_up,
    Move_down,
    Move_left,
    Move_right
} Movement;

struct Cell { int x_pos; int y_pos; };
struct Cells {
    struct Cell units[256];
    int len;
};

void add_body(struct Cells* body, struct Cell c) {
    if (body->len < 255) {
        body->units[body->len] = c;
        body->len++;
    }
}

int rand_int(int max) {
    return rand() % max;
}

void render_board(char board[COLS][LINES]) {
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < COLS; j++) {
            switch (board[i][j]) {
                case 'A':
                    attron(COLOR_PAIR(1));
                    mvaddch(j, i, 'A');
                    attroff(COLOR_PAIR(1));
                    break;
                case '<':
                    attron(COLOR_PAIR(2));
                    mvaddch(j, i, '<');
                    attroff(COLOR_PAIR(2));
                    break;
                case '^':
                    attron(COLOR_PAIR(2));
                    mvaddch(j, i, '^');
                    attroff(COLOR_PAIR(2));
                    break;
                case 'V':
                    attron(COLOR_PAIR(2));
                    mvaddch(j, i, 'V');
                    attroff(COLOR_PAIR(2));
                    break;
                case '>':
                    attron(COLOR_PAIR(2));
                    mvaddch(j, i, '>');
                    attroff(COLOR_PAIR(2));
                    break;
                default:
                    attron(COLOR_PAIR(3));
                    mvaddch(j, i, ' ');
                    attroff(COLOR_PAIR(3));
                    break;
            }
        }
    }

    refresh();
}

bool ate_apple = false;

struct Cell move_head(char board[COLS][LINES], struct Cell head, Movement m) {
    struct Cell new_head = head;
    board[head.x_pos][head.y_pos] = ' ';
    
    switch (m) {
        case Move_left:
            new_head.x_pos = (head.x_pos > 0) ? head.x_pos - 1 : COLS - 1;

            ate_apple = (board[new_head.x_pos][new_head.y_pos] == 'A');
            board[new_head.x_pos][new_head.y_pos] = '<';
            break;
        case Move_right:
            new_head.x_pos = (head.x_pos < COLS - 1) ? head.x_pos + 1 : 0;

            ate_apple = (board[new_head.x_pos][new_head.y_pos] == 'A');
            board[new_head.x_pos][new_head.y_pos] = '>';
            break;
        case Move_up:
            new_head.y_pos = (head.y_pos > 0) ? head.y_pos - 1 : LINES - 1;

            ate_apple = (board[new_head.x_pos][new_head.y_pos] == 'A');
            board[new_head.x_pos][new_head.y_pos] = '^';
            break;
        case Move_down:
            new_head.y_pos = (head.y_pos < LINES - 1) ? head.y_pos + 1 : 0;

            ate_apple = (board[new_head.x_pos][new_head.y_pos] == 'A');
            board[new_head.x_pos][new_head.y_pos] = 'V';
            break;
    }
    
    return new_head;
}

void place_apple(char board[COLS][LINES]) {
    int apple_x, apple_y;
    do {
        apple_x = rand_int(COLS - 1);
        apple_y = rand_int(LINES - 1);
    } while (board[apple_x][apple_y] != ' ');
    
    board[apple_x][apple_y] = 'A';
}

void draw_segment(char board[COLS][LINES], struct Cell pos, Movement m) {
    switch (m) {
        case Move_left:  board[pos.x_pos][pos.y_pos] = '<'; break;
        case Move_right: board[pos.x_pos][pos.y_pos] = '>'; break;
        case Move_up:    board[pos.x_pos][pos.y_pos] = '^'; break;
        case Move_down:  board[pos.x_pos][pos.y_pos] = 'V'; break;
    }
}

bool check_snake_collision(struct Cells* body) {
    struct Cell head = body->units[0];
    
    for (int i = 1; i < body->len; i++) {
        if (body->units[i].x_pos == head.x_pos && 
            body->units[i].y_pos == head.y_pos) {
            return true;
        }
    }
    
    return false;
}

bool update_board(char board[COLS][LINES], struct Cells* body, const Movement m) {
    struct Cell old_head = body->units[0];
    struct Cell new_head = move_head(board, old_head, m);
    
    // bool ate_apple = (board[new_head.x_pos][new_head.y_pos] == 'A');
    
    // If we grow, we need to store the last body pos
    struct Cell last_pos = body->units[body->len - 1];
    
    struct Cell prev_pos = old_head;
    for (int i = 1; i < body->len; i++) {
        struct Cell current = body->units[i];
        board[current.x_pos][current.y_pos] = ' ';
        
        body->units[i] = prev_pos;
        
        draw_segment(board, prev_pos, m);
        prev_pos = current;
    }
    
    body->units[0] = new_head;
    
    if (ate_apple) {
        add_body(body, last_pos);
        place_apple(board);
    }
    
    if (check_snake_collision(body)) {
        return true;
    }

    return false;
}

void end_game(int score) {
    printf("Thanks for playing. Your final score is %i\n", score);
}

int main(void) {

    WINDOW* win = initscr();
    nodelay(win, true);
    cbreak();
    noecho();
    curs_set(0);
    start_color();

    srand(time(NULL));

    init_pair(1, COLOR_RED, COLOR_RED); // Apple
    init_pair(2, COLOR_GREEN, COLOR_GREEN); // snake
    init_pair(3, COLOR_BLUE, COLOR_BLUE); // bg

    // <>^V to represent snake next movement, A for Apple
    // otherwise, empty is blank
    char board[COLS][LINES] = {};

    // initial state
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = ' ';
        }
    }

    // Populate initial apple and snake head
    board[(int)round(COLS / 5)][(int)round(LINES / 5)] = '>';
    board[rand_int(COLS - 1)][rand_int(LINES - 1)] = 'A';
    struct Cells cells = {};
    cells.len = 1;
    cells.units[0] = (struct Cell){
        .x_pos = (int)round(COLS / 5),
        .y_pos = (int)round(LINES / 5)
    };


    Movement default_dir = Move_right;
    bool exit = false;
    while (!(exit)) {
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000000}; // 0.1 sec
        nanosleep(&ts, NULL);
        render_board(board);

        char c = getch();
        switch (c) {
            case 'q':
                exit = true;
                break;
            case LEFT:
                if (update_board(board, &cells, Move_left)) { exit = true; }
                default_dir = Move_left;
                break;
            case DOWN:
                if (update_board(board, &cells, Move_down)) { exit = true; }
                default_dir = Move_down;
                break;
            case UP:
                if (update_board(board, &cells, Move_up)) { exit = true; }
                default_dir = Move_up;
                break;
            case RIGHT:
                if (update_board(board, &cells, Move_right)) { exit = true; }
                default_dir = Move_right;
                break;

            default:
                if (update_board(board, &cells, default_dir)) { exit = true; }
                break;
        }
    }


    nocbreak();
    echo();
    endwin();

    end_game(cells.len);
    return 0;
}
