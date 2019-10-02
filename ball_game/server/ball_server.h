/*************************************************************************
	> File Name: ball_server.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月28日 星期六 19时28分15秒
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

Game game;

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

void *to_send(void *arg);
void *to_recv(void *arg);
int is_win(int row, int col);
int out_Range(int x, int y);
int has_ball(int pno);
void init_pos();

int out_Range(int x, int y) {
    if (x <= 0 || x >= HEIGHT || y <= 0 || y >= WIDTH) {
        return 1;
    }
    return 0;
}

int is_win(int x, int y) {
    if (x < ((HEIGHT/2) - 1) || x > ((HEIGHT / 2) + 3)) {
        return -1;
    }
    if (y <= 0) return 1;
    else if (y >= WIDTH) return 2;
    else return -1;
}

void init_pos() {
    game.ball.ballx = HEIGHT / 2 + 1;
    game.ball.bally = WIDTH / 2;
    game.who_has_ball = -1;
}

void *to_send(void *arg) {
    char peo[2] = "P";
    char ball[2] = "o";
    char mid_line[2] = ".";
    char men_str[2] = "#";
    char row_str[2] = "-";
    char col_str[2] = "|";
    char score_infoA[200];
    char score_infoB[200];
    init_pos();
    initscr();
    clear();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);
    while (1) {
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
        
        if (game.ball.speed > 0) {
            game.ball.ballx += game.ball.power_row;
            game.ball.bally += game.ball.power_col;
            game.ball.speed --;
        } else {
            if (game.who_has_ball >= 0) {
                game.ball.ballx = game.peo[game.who_has_ball].loc_x + game.peo[game.who_has_ball].face[0];
                game.ball.bally = game.peo[game.who_has_ball].loc_y + game.peo[game.who_has_ball].face[1];
            }
            game.ball.power_row = game.ball.power_col = 0;
        }
        move(game.ball.ballx, game.ball.bally);
        addstr(ball);
        //出界与进球；
        if (game.ball.bally <= 0 || game.ball.bally >= WIDTH) {
            int k = is_win(game.ball.ballx, game.ball.bally);
            if (k > 0 || out_Range(game.ball.ballx, game.ball.bally)) {
                game.ball.ballx = HEIGHT / 2 + 1;
                game.ball.bally = WIDTH / 2;
                game.ball.speed = 0;
                if (k == 1) game.score_teamA += 1;
                else if (k == 2) game.score_teamB += 1;
            }
        }
        for (int i = 0; i < game.num; i++) {
            if (game.peo[i].fd < 0) continue;
            move(game.peo[i].loc_x, game.peo[i].loc_y);
            if (game.peo[i].team == 1) {
                attron(COLOR_PAIR(1));
                addstr(peo);
                attroff(COLOR_PAIR(1));
            } else if (game.peo[i].team == 2) {
                attron(COLOR_PAIR(2));
                addstr(peo);
                attroff(COLOR_PAIR(2));
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
        int cnt = game.num;
        refresh();
        for (int i = 0; i < cnt; i++) {
            int client_fd = game.peo[i].fd;
            if (client_fd < 0) continue;
            game.pno = i;
            send(client_fd, &game, sizeof(game), 0);
        }
        usleep(100000);
        clear();
    }
    endwin();
}

int dest[8][2] = {-1, 0, 1, 0, 0, -1, 0, 1, -1, 1, 1, 1, 1, -1, -1, -1};
int has_ball(int pno) {
    int row = game.peo[pno].loc_x;
    int col = game.peo[pno].loc_y;
    int ball_x = game.ball.ballx;
    int ball_y = game.ball.bally;
    if (fabs(row - ball_x) <= 1 && fabs(col - ball_y) <= 1) {
        for (int i = 0; i < 8; i++) {
            int nrow = row + dest[i][0];
            int ncol = col + dest[i][1];
            if (nrow == ball_x && ncol == ball_y) {
                game.ball.power_row = dest[i][0];
                game.ball.power_col = dest[i][1] * 2;
                break;
            }
        } 
        return true;
    } else {
        return false;
    }
}


void *to_recv(void *arg) {
    struct Message *mes = (struct Message*) arg;
    int fd = mes->fd;
    struct Comm comm;
    while (1) {
        memset(&comm, 0, sizeof(comm));
        if (recv(fd, &comm, sizeof(comm), 0) <= 0) {
            char buff[100] = {0};
            sprintf(buff, "%s logout\n", mes->name);
            FILE *fp;
            fp = fopen("./game.log", "ab+");
            fwrite(buff, strlen(buff), 1, fp);
            fclose(fp);
            close(fd);
            game.peo[comm.pno].fd = -1;
            break;
        }
        int flag = 0;
        if( has_ball(comm.pno)) {
            game.who_has_ball = comm.pno;
            flag = 1;
        }
        FILE *fp = fopen("./game.log", "ab+");
        fprintf(fp, "pno = %d\n", comm.pno);
        fclose(fp);
        switch (comm.op) {
            case 1: {
                game.peo[comm.pno].loc_x -= 1;
                game.peo[comm.pno].face[0] = -1;
                game.peo[comm.pno].face[1] = 0;
            }break;
            case 2: {
                game.peo[comm.pno].loc_x += 1;
                game.peo[comm.pno].face[0] = 1;
                game.peo[comm.pno].face[1] = 0;
            }break;
            case 3: {
                game.peo[comm.pno].loc_y -= 1;
                game.peo[comm.pno].face[0] = 0;
                game.peo[comm.pno].face[1] = -1;
            }break;
            case 4: {
                game.peo[comm.pno].loc_y += 1;
                game.peo[comm.pno].face[0] = 0;
                game.peo[comm.pno].face[1] = 1;
            }break;
            case 5: {
                if (flag) {
                    game.ball.speed = 6;
                }
                game.who_has_ball = -1;
            }break;
            default:
                break;
        }
    }
}



