/*************************************************************************
	> File Name: g_test.h
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月27日 星期五 11时22分42秒
 ************************************************************************/

#ifndef _G_TEST_H
#define _G_TEST_H
#include <stdio.h>
#include <stdlib.h>
typedef void (*FuncType)();

#define COLOR(a, b) "\033[" #b "m" a "\033[0m"
#define RED(a) COLOR(a, 31)
#define BLUE(a) COLOR(a, 34)
#define GREEN(a) COLOR(a, 32)
#define YELLOW(a) COLOR(a, 33)

typedef struct FuncData {
    FuncType func;
    const char *name_part1;
    const char *name_part2;
    struct FuncData *next;
} FuncData, *pFuncData;

static FuncData head, *tail = &head;

typedef struct TotalData {
    int succ, fail;
} TotalData;

static TotalData info;

void clear_TotalData() {
    info.succ = info.fail = 0;
}

void add_success() {
    info.succ++;
}

void add_failed(const char *file, const int line, const char *mess) {
    printf(RED("%s:%d: Failure\n"), file, line);
    printf(RED("%s\n"), mess);
    info.fail ++;
}

#define __FuncName(a, b) a##b
#define FuncName(a, b) __FuncName(a, b)
#define STR(a) #a

#define TEST(a, b) \
void FuncName(a, FuncName(_, FuncName(b, FuncName(_, Test))))();\
__attribute__((constructor)) void FuncName(add_Test, __COUNTER__)() {\
    pFuncData p = (pFuncData) malloc (sizeof(FuncData));\
    p->func = FuncName(a, FuncName(_, FuncName(b, FuncName(_, Test))));\
    p->name_part1 = STR(a);\
    p->name_part2 = STR(b);\
    tail->next = p;\
    tail = p;\
}\
void FuncName(a, FuncName(_, FuncName(b, FuncName(_,Test))))()

#define TYPE_STR(a) _Generic((a), \
    int : "%d", \
    double : "%lf", \
    char : "%c",\
    float : "%f",\
    const char* : "%s"\
)

#define P(a, color) {\
    char frm[100];\
    sprintf(frm, color("%s"), TYPE_STR(a));\
    printf(frm, a);\
}

#define EXPECT(a, b, c) {\
    __typeof(a) __temp_a = a,__temp_b = b;\
    if (__temp_a c __temp_b) add_success();\
    else {\
        add_failed(__FILE__, __LINE__, "except " #a " " #c " " #b);\
        printf(RED("actual : "));\
        P(__temp_a, RED);\
        printf(RED(" find "));\
        P(__temp_b,RED);\
        printf("\n");\
    }\
    printf("%s %s %s %s\n\n", STR(a), STR(c), STR(b), __temp_a c __temp_b ? GREEN("True") : RED("False"));\
}

#define EXPECT_EQ(a, b) EXPECT(a, b, ==)
#define EXPECT_NE(a, b) EXPECT(a, b, !=)
#define EXPECT_GT(a, b) EXPECT(a, b, >)
#define EXPECT_LT(a, b) EXPECT(a, b, <)
#define EXPECT_LE(a, b) EXPECT(a, b, <=)
#define EXPECT_GE(a, b) EXPECT(a, b, >=)

void output_info() {
    double rate = 1.0 * info.succ / (info.succ + info.fail);
    const char *output_str = NULL;
    char output_frm[100];
    if (info.fail == 0) {
        output_str = BLUE("%6.2lf%%");
    } else {
        output_str = RED("%6.2lf%%");
    }
    sprintf(output_frm, "[ %s ] toatal: %%3d success: %%3d\n", output_str);
    printf(output_frm, rate * 100, info.succ + info.fail, info.succ);
}

int RUN_ALL_TESTS() {
    pFuncData p = head.next;
    while(p != NULL) {
        printf("[ %s : %s ]\n", p->name_part1,p->name_part2);
        clear_TotalData();
        p->func();
        p = p->next;
        output_info();
        printf("--------------------------\n");
    }
    return 0;
}

#endif
