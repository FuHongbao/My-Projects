/*************************************************************************
	> File Name: ball_client.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月29日 星期日 11时48分23秒
 ************************************************************************/

#include "../util/util.h"
#include <pthread.h>
#define MAX_N 20
#define WIDTH 110
#define HEIGHT 33

typedef struct Player {
    int loc_x;
    int loc_y;
    int team;
    int face[2];
    int fd;
} Player;

typedef struct Ball {
    int ballx;
    int bally;
    int speed;
    int power_row;
    int power_col;
} Ball;

typedef struct Game {
    int who_has_ball;   
    int num;
    int score_teamA;
    int score_teamB;
    int pno;
    Ball ball;
    Player peo[MAX_N + 5];
} Game;

struct Message {
    int fd;
    int team;
    int pno;
    char name[20];
};

struct Comm {
    int op;
    int pno;
};

int flag = 1;
int mypno;
int myteam;
Game game;

void sigFunc() {
    flag = 0;
}

void *to_recv(void *arg) {
    int *fd = (int*) arg;
    char plr[2] = "M";
    char peo[2] = "F";
    char enermy[2] = "P";
    char ball[2] = "o";
    char mid_line[2] = ".";
    char men_str[2] = "#";
    char row_str[2] = "-";
    char col_str[2] = "|";
    char score_infoA[200];
    char score_infoB[200];
    initscr();
    clear();
    signal(SIGINT, sigFunc);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);
    init_pair(2, COLOR_WHITE, COLOR_GREEN);
    init_pair(3, COLOR_WHITE, COLOR_YELLOW);
    while (flag) {
        memset(&game, 0, sizeof(game));
        if (recv(*fd, &game, sizeof(game), 0) < 0) {
            perror("Error recv : *fd");
            exit(1);
        }
        if (game.num == 0) {
            continue;
        }
        mypno = game.pno;
        clear();
        for (int i = 0; i <= WIDTH; i++) {
            move(0, i);
            addstr(row_str);
            move(HEIGHT/2+1, i);
            addstr(mid_line);
            move(HEIGHT, i);
            addstr(row_str);
        }
        for (int i = 10; i < 24; i++) {
            move(i, 45);
            addstr(mid_line);
            move(i, 65);
            addstr(mid_line);
        }
        for (int i = 45; i <= 65; i++) {
            move(10, i);
            addstr(mid_line);
            move(23, i);
            addstr(mid_line);
        }
        for (int i = 1; i < HEIGHT; i++) {
            move(i, 0);
            addstr(col_str);
            move(i, WIDTH/2);
            addstr(col_str);
            move(i, WIDTH);
            addstr(col_str);
        }
        for (int i = (HEIGHT / 2)-1; i <= (HEIGHT / 2)+3; i++) {
            move(i, 0);
            addstr(men_str);
            if(i == (HEIGHT / 2) - 1 || i == (HEIGHT / 2) + 3) {
                move(i, 1);
                addstr(men_str);
                move(i, 2);
                addstr(men_str);
                move(i, WIDTH - 1);
                addstr(men_str);
                move(i, WIDTH - 2);
                addstr(men_str);
            }
            move(i, WIDTH);
            addstr(men_str);
        }  
        move(game.ball.ballx, game.ball.bally);
        addstr(ball);
        for (int i = 0; i < game.num; i++) {
            if (game.peo[i].fd < 0) continue;
            move(game.peo[i].loc_x, game.peo[i].loc_y);
            if (i == mypno) {
                attron(COLOR_PAIR(3));
                addstr(plr);
                attroff(COLOR_PAIR(3));
            } else {
                if (game.peo[i].team == myteam) {
                    attron(COLOR_PAIR(2));
                    addstr(peo);
                    attroff(COLOR_PAIR(2));
                } else {
                    attron(COLOR_PAIR(1));
                    addstr(enermy);
                    attroff(COLOR_PAIR(1));
                }
            }
        }
        memset(score_infoA, 0, sizeof(score_infoA));
        sprintf(score_infoA, "Team one get score : %d", game.score_teamA);
        move(7, WIDTH + 5);
        addstr(score_infoA);
        memset(score_infoB, 0, sizeof(score_infoB));
        sprintf(score_infoB, "Team two get score : %d", game.score_teamB);
        move(9, WIDTH + 5);
        addstr(score_infoB);
        refresh();
    }
    endwin(); 
}

void to_send(int fd) {
    signal(SIGINT, sigFunc);
    struct Comm comm;
    while (flag) {
        char c;
        scanf("%c", &c);
        if (!flag) break;
        memset(&comm, 0, sizeof(comm));
        comm.pno = mypno; 
        if (c == 'w') comm.op = 1;
        else if (c == 's') comm.op = 2;
        else if (c == 'a') comm.op = 3;
        else if (c == 'd') comm.op = 4;
        else if (c == ' ') comm.op = 5;
        if (comm.op >= 1 && comm.op <= 5) {
            if (send(fd, &comm, sizeof(comm), 0) < 0) {
                perror("Error send : fd");
                break;
            }
        } 
    }
    return ;
}





