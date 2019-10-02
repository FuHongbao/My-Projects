/*************************************************************************
	> File Name: g_test.c
	> Author: victoria 
	> Mail: 1105847344@qq.com
	> Created Time: 2019年09月27日 星期五 14时40分43秒
 ************************************************************************/

#include <stdio.h>
#include "g_test.h"

int add(int a, int b) {
    return a + b;
}

int is_prime(int x) {
    if (x <= 1) return 0;
    for (int i = 2; i * i <= x; i++) {
        if (x % i == 0) return 0;
    }
    return 1;
}

TEST(test, is_prime_func) {
    EXPECT_EQ(is_prime(2), 0);
    EXPECT_EQ(is_prime(-2), 0);
    EXPECT_EQ(is_prime(15), 1);
    EXPECT_EQ(is_prime(9981), 1);
}

TEST(test, add_func) {
    EXPECT_EQ(add(1, 2), 3);
    EXPECT_EQ(add(3, 2), 3);
    EXPECT_EQ(add(6, 2), 8);
    EXPECT_NE(add(2, 2), 6);

}

int main() {
    return RUN_ALL_TESTS();
}

