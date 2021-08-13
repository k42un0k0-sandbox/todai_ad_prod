/* ヘッダファイルのインクルード */
#include <stdio.h>

int main(int argc, char **argv) {
    // コマンドラインから変数を読みとる
    int a, b;
    scanf("%d %d", &a, &b);

    // 割り算
    const double c = (double)a / (double)b;
    printf("%d / %d = %f\n", a, b, c);
}