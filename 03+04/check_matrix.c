#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#ifdef _NOT_USE_HEADER
#include "matrix.c"
#else
#include "matrix.h"
#endif

static const double tolerance = 1.0e-12;

// ------------------------------------
// Unit test macros
// ------------------------------------

#ifdef _MSC_VER
#define ERROR_TEXT "ERROR"
#define PASSED_TEXT "PASSED"
#define FAILED_TEXT "FAILED"
#else
#define ERROR_TEXT "\x1b[31mERROR\x1b[m"
#define PASSED_TEXT "\x1b[32mPASSED\x1b[m"
#define FAILED_TEXT "\x1b[31mFAILED\x1b[m"
#endif

#define FATAL_ERROR(msg)                                 \
    do                                                   \
    {                                                    \
        fprintf(stdout, "[ %s ] %s\n", ERROR_TEXT, msg); \
        *success = false;                                \
    } while (0)

#define EXPECT_TRUE(actual)                                                                              \
    do                                                                                                   \
    {                                                                                                    \
        if (!(actual))                                                                                   \
        {                                                                                                \
            fprintf(stderr, "[ %s ] expected = true, actual = false (Line %d)\n", ERROR_TEXT, __LINE__); \
            *success = false;                                                                            \
        }                                                                                                \
    } while (0)

#define ASSERT_TRUE(actual) \
    EXPECT_TRUE(actual);    \
    if (!*success)          \
    return

#define EXPECT_FALSE(actual)                                                                             \
    do                                                                                                   \
    {                                                                                                    \
        if ((actual))                                                                                    \
        {                                                                                                \
            fprintf(stderr, "[ %s ] expected = false, actual = true (Line %d)\n", ERROR_TEXT, __LINE__); \
            *success = false;                                                                            \
        }                                                                                                \
    } while (0)

#define ASSERT_FALSE(actual) \
    EXPECT_FALSE(actual);    \
    if (!*success)           \
    return

#define EXPECT_EQUAL(expected, actual)                                                                                \
    do                                                                                                                \
    {                                                                                                                 \
        if (fabs((expected) - (actual)) >= tolerance)                                                                 \
        {                                                                                                             \
            fprintf(stderr, "[ %s ] expected = %f, actual = %f (Line %d)\n", ERROR_TEXT, expected, actual, __LINE__); \
            *success = false;                                                                                         \
        }                                                                                                             \
    } while (0)

#define ASSERT_EQUAL(expected, actual) \
    EXPECT_EQUAL(expected, actual);    \
    if (!*success)                     \
    return

#define TEST_INIT()        \
    int count_success = 0; \
    int count_tests = 0;

#define TEST_FINISH()                                                            \
    printf("%d / %d tests failed,\n", count_tests - count_success, count_tests); \
    printf("[ STATUS ] %s\n", count_success == count_tests ? PASSED_TEXT : FAILED_TEXT);

#define TESTCASE(name)                                                      \
    void test_##name(bool *success);                                        \
    bool run_test_##name()                                                  \
    {                                                                       \
        printf("[ RUN ] %s\n", #name);                                      \
        bool success = true;                                                \
        test_##name(&success);                                              \
        printf("[ %6s ] %s\n", success ? PASSED_TEXT : FAILED_TEXT, #name); \
        printf("=====\n");                                                  \
        return success;                                                     \
    }                                                                       \
    void test_##name(bool *success)

#define RUN_TEST(name)                          \
    count_success += run_test_##name() ? 1 : 0; \
    count_tests += 1;

// ------------------------------------
// Utility
// ------------------------------------

#define SAFE_DECLARE(_type, _x) \
    _type _x;                   \
    memset(&_x, 0, sizeof(_type))

bool is_valid_mat(matrix mat)
{
    return mat.rows > 0 && mat.cols > 0 && mat.elems != NULL;
}

void mat_rand(matrix *mat)
{
    for (int i = 0; i < mat->rows; i++)
    {
        for (int j = 0; j < mat->cols; j++)
        {
            mat_elem(*mat, i, j) = rand() / (double)RAND_MAX;
        }
    }
}

// ------------------------------------
// Unit tests
// ------------------------------------

TESTCASE(mat_alloc_and_free)
{
    // 行列のメモリ確保が成功するかどうか
    SAFE_DECLARE(matrix, A);
    ASSERT_TRUE(mat_alloc(&A, 4, 5));
    ASSERT_TRUE(4 == A.rows);
    ASSERT_TRUE(5 == A.cols);
    ASSERT_FALSE(NULL == A.elems);

    // 行列のメモリ解放(+変数の初期化)が成功するかどうか
    mat_free(&A);
    ASSERT_TRUE(0 == A.rows);
    ASSERT_TRUE(0 == A.cols);
    ASSERT_TRUE(NULL == A.elems);

    // 不正な行列サイズ
    ASSERT_FALSE(mat_alloc(&A, 0, 0));
    mat_free(&A);

    // 不正な行列サイズ
    ASSERT_FALSE(mat_alloc(&A, -1, 10));
    mat_free(&A);
}

TESTCASE(mat_copy)
{
    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);

    // 異なるサイズの行列はコピーができない
    mat_alloc(&A, 123, 45);
    mat_alloc(&B, 23, 56);
    ASSERT_FALSE(mat_copy(&B, A));
    mat_free(&A);
    mat_free(&B);

    // 同じサイズの行列はコピーができる
    mat_alloc(&A, 123, 45);
    mat_alloc(&B, 123, 45);
    mat_rand(&A);

    // コピーができるかどうか
    ASSERT_TRUE(mat_copy(&B, A));

    // コピーした値が正しいかどうか
    for (int i = 0; i < A.rows; i++)
    {
        for (int j = 0; j < A.cols; j++)
        {
            ASSERT_EQUAL(mat_elem(A, i, j), mat_elem(B, i, j));
        }
    }

    mat_free(&A);
    mat_free(&B);
}

TESTCASE(mat_add)
{
    const int rows = 123;
    const int cols = 45;

    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);
    SAFE_DECLARE(matrix, C);

    // ランダムな行列の足し算
    mat_alloc(&A, rows, cols);
    mat_rand(&A);
    mat_alloc(&B, rows, cols);
    mat_rand(&B);
    mat_alloc(&C, rows, cols);

    ASSERT_TRUE(mat_add(&C, A, B));

    for (int i = 0; i < C.rows; i++)
    {
        for (int j = 0; j < C.cols; j++)
        {
            ASSERT_EQUAL(mat_elem(A, i, j) + mat_elem(B, i, j), mat_elem(C, i, j));
        }
    }

    mat_free(&A);
    mat_free(&B);
    mat_free(&C);

    // サイズの異なる行列 (その1)
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 23, 45);
    mat_alloc(&C, 12, 34);
    mat_rand(&A);
    mat_rand(&B);
    ASSERT_FALSE(mat_add(&C, A, B));
    mat_free(&A);
    mat_free(&B);
    mat_free(&C);

    // サイズの異なる行列 (その2)
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 12, 34);
    mat_alloc(&C, 23, 45);
    mat_rand(&A);
    mat_rand(&B);
    ASSERT_FALSE(mat_add(&C, A, B));
    mat_free(&A);
    mat_free(&B);
    mat_free(&C);
}

TESTCASE(mat_sub)
{
    const int rows = 123;
    const int cols = 45;

    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);
    SAFE_DECLARE(matrix, C);

    // ランダムな行列の引き算
    mat_alloc(&A, rows, cols);
    mat_rand(&A);
    mat_alloc(&B, rows, cols);
    mat_rand(&B);
    mat_alloc(&C, rows, cols);

    ASSERT_TRUE(mat_sub(&C, A, B));
    for (int i = 0; i < C.rows; i++)
    {
        for (int j = 0; j < C.cols; j++)
        {
            ASSERT_EQUAL(mat_elem(A, i, j) - mat_elem(B, i, j), mat_elem(C, i, j));
        }
    }

    mat_free(&A);
    mat_free(&B);
    mat_free(&C);

    // サイズの異なる行列 (その1)
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 23, 45);
    mat_alloc(&C, 12, 34);
    mat_rand(&A);
    mat_rand(&B);
    ASSERT_FALSE(mat_sub(&C, A, B));
    mat_free(&A);
    mat_free(&B);
    mat_free(&C);

    // サイズの異なる行列 (その2)
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 12, 34);
    mat_alloc(&C, 23, 45);
    mat_rand(&A);
    mat_rand(&B);
    ASSERT_FALSE(mat_sub(&C, A, B));
    mat_free(&A);
    mat_free(&B);
    mat_free(&C);
}

TESTCASE(mat_mul)
{
    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);
    SAFE_DECLARE(matrix, C);

    // サイズが不整合の行列 (A * Bが計算できない)
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 56, 34);
    mat_alloc(&C, 12, 34);
    ASSERT_FALSE(mat_mul(&C, A, B));
    mat_free(&A);
    mat_free(&B);
    mat_free(&C);

    // サイズが不整合の行列 (A * Bは計算できるが、Cに代入できない)
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 34, 56);
    mat_alloc(&C, 78, 90);
    ASSERT_FALSE(mat_mul(&C, A, B));
    mat_free(&A);
    mat_free(&B);
    mat_free(&C);

    // 積が計算できる行列
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 34, 56);
    mat_alloc(&C, 12, 56);

    mat_rand(&A);
    mat_rand(&B);

    // 行列の積が計算できるかどうか
    ASSERT_TRUE(mat_mul(&C, A, B));

    // 計算結果のサイズが正しいかどうか
    ASSERT_TRUE(C.rows == A.rows);
    ASSERT_TRUE(C.cols == B.cols);

    // 積の計算結果が正しいかどうか
    for (int i = 0; i < C.rows; i++)
    {
        for (int j = 0; j < C.cols; j++)
        {
            double val = 0.0;
            for (int k = 0; k < A.cols; k++)
            {
                val += mat_elem(A, i, k) * mat_elem(B, k, j);
            }
            ASSERT_EQUAL(val, mat_elem(C, i, j));
        }
    }

    mat_free(&A);
    mat_free(&B);
    mat_free(&C);

    // 同じ行列を入力にしても大丈夫か
    mat_alloc(&A, 12, 12);
    mat_alloc(&B, 12, 12);
    mat_alloc(&C, 12, 12);

    mat_rand(&A);
    mat_rand(&B);

    // 先にCにAの内容をコピーしておく
    memcpy(C.elems, A.elems, sizeof(double) * A.rows * A.cols);

    // 同じ行列を入れても積が計算できるかどうか
    ASSERT_TRUE(mat_mul(&C, C, B));

    // 積の計算結果が正しいかどうか
    for (int i = 0; i < C.rows; i++)
    {
        for (int j = 0; j < C.cols; j++)
        {
            double val = 0.0;
            for (int k = 0; k < A.cols; k++)
            {
                val += mat_elem(A, i, k) * mat_elem(B, k, j);
            }
            ASSERT_EQUAL(val, mat_elem(C, i, j));
        }
    }

    mat_free(&A);
    mat_free(&B);
    mat_free(&C);
}

TESTCASE(mat_muls)
{
    const int rows = 123;
    const int cols = 45;

    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);

    // ランダムの行列とスカラの掛け算
    mat_alloc(&A, rows, cols);
    mat_rand(&A);
    mat_alloc(&B, rows, cols);
    mat_rand(&B);
    const double s = rand() / (double)RAND_MAX;

    ASSERT_TRUE(mat_muls(&B, A, s));
    for (int i = 0; i < B.rows; i++)
    {
        for (int j = 0; j < B.cols; j++)
        {
            ASSERT_EQUAL(mat_elem(A, i, j) * s, mat_elem(B, i, j));
        }
    }

    mat_free(&A);
    mat_free(&B);

    // サイズの異なる行列
    mat_alloc(&A, 12, 34);
    mat_alloc(&B, 23, 45);
    mat_rand(&A);
    mat_rand(&B);
    ASSERT_FALSE(mat_muls(&B, A, 3.1415));
    mat_free(&A);
    mat_free(&B);
}

TESTCASE(mat_ident)
{
    SAFE_DECLARE(matrix, A);

    // 非正方行列は単位行列にできない
    mat_alloc(&A, 10, 15);
    ASSERT_FALSE(mat_ident(&A));
    mat_free(&A);

    // 正方行列が単位行列になるか
    const int size = 123;
    mat_alloc(&A, size, size);
    ASSERT_TRUE(mat_ident(&A));

    // 正方行列の値が正しいかどうか
    for (int i = 0; i < A.rows; i++)
    {
        for (int j = 0; j < A.cols; j++)
        {
            ASSERT_EQUAL(i == j ? 1.0 : 0.0, mat_elem(A, i, j));
        }
    }

    mat_free(&A);
}

TESTCASE(mat_trans)
{
    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);

    // 行列の形の不整合
    mat_alloc(&A, 123, 45);
    mat_alloc(&B, 12, 23);
    ASSERT_FALSE(mat_trans(&B, A));
    mat_free(&A);
    mat_free(&B);

    // 転置できる行列
    mat_alloc(&A, 123, 45);
    mat_alloc(&B, 45, 123);
    mat_rand(&A);

    // 転置ができるかどうか
    ASSERT_TRUE(mat_trans(&B, A));

    // 転置の結果が正しいかどうか
    for (int i = 0; i < A.rows; i++)
    {
        for (int j = 0; j < A.cols; j++)
        {
            ASSERT_EQUAL(mat_elem(A, i, j), mat_elem(B, j, i));
        }
    }

    mat_free(&A);
    mat_free(&B);

    // 同じ行列を入力にしても大丈夫か
    mat_alloc(&A, 12, 12);
    mat_alloc(&B, 12, 12);
    mat_rand(&A);

    // 先にBにAの内容をコピーしておく
    memcpy(B.elems, A.elems, sizeof(double) * A.rows * A.cols);

    // 同じ行列を入れても積が計算できるかどうか
    ASSERT_TRUE(mat_trans(&B, A));

    // 転置の結果が正しいかどうか
    for (int i = 0; i < A.rows; i++)
    {
        for (int j = 0; j < A.cols; j++)
        {
            ASSERT_EQUAL(mat_elem(A, i, j), mat_elem(B, j, i));
        }
    }

    mat_free(&A);
    mat_free(&B);
}

TESTCASE(mat_equal)
{
    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);

    // サイズの異なる行列
    mat_alloc(&A, 123, 45);
    mat_alloc(&B, 234, 56);
    ASSERT_FALSE(mat_equal(A, B));
    mat_free(&A);
    mat_free(&B);

    // サイズが同じ行列で要素が異なる
    mat_alloc(&A, 123, 45);
    mat_alloc(&B, 123, 45);
    mat_rand(&A);
    memcpy(B.elems, A.elems, sizeof(double) * A.rows * A.cols);

    const int rand_row = A.rows > 0 ? rand() % A.rows : 0;
    const int rand_col = A.cols > 0 ? rand() % A.cols : 0;
    if (rand_row >= 0 && rand_col >= 0 && B.elems != NULL)
    {
        mat_elem(B, rand_row, rand_col) += 1.0e-10;
    }

    ASSERT_FALSE(mat_equal(A, B));

    // サイズが同じで要素も同じ
    mat_rand(&A);
    mat_copy(&B, A);
    memcpy(B.elems, A.elems, sizeof(double) * A.rows * A.cols);

    ASSERT_TRUE(mat_equal(A, B));

    mat_free(&A);
    mat_free(&B);
}

TESTCASE(mat_solve_simple)
{
    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, x);
    SAFE_DECLARE(matrix, b);

    mat_alloc(&A, 3, 3);
    mat_alloc(&x, 3, 1);
    mat_alloc(&b, 3, 1);

    ASSERT_TRUE(is_valid_mat(A));
    ASSERT_TRUE(is_valid_mat(x));
    ASSERT_TRUE(is_valid_mat(b));

    mat_elem(A, 0, 0) = 2.0;
    mat_elem(A, 0, 1) = 3.0;
    mat_elem(A, 0, 2) = 1.0;
    mat_elem(A, 1, 0) = 4.0;
    mat_elem(A, 1, 1) = 1.0;
    mat_elem(A, 1, 2) = -3.0;
    mat_elem(A, 2, 0) = -1.0;
    mat_elem(A, 2, 1) = 2.0;
    mat_elem(A, 2, 2) = 1.0;

    mat_elem(b, 0, 0) = 2.0;
    mat_elem(b, 1, 0) = 3.0;
    mat_elem(b, 2, 0) = 4.0;

    // 正しく連立方程式が解けるかどうか
    ASSERT_TRUE(mat_solve(&x, A, b));

    // 連立方程式の答えが正しいかどうか
    ASSERT_EQUAL(mat_elem(x, 0, 0), -1.45);
    ASSERT_EQUAL(mat_elem(x, 1, 0), 2.35);
    ASSERT_EQUAL(mat_elem(x, 2, 0), -2.15);
}

TESTCASE(mat_inverse_simple)
{
    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);
    SAFE_DECLARE(matrix, invA);

    mat_alloc(&A, 3, 3);
    mat_alloc(&B, 3, 3);
    mat_alloc(&invA, 3, 3);

    ASSERT_TRUE(is_valid_mat(A));
    ASSERT_TRUE(is_valid_mat(invA));
    ASSERT_TRUE(is_valid_mat(B));

    // 正しく逆行列が求まる行列
    mat_elem(A, 0, 0) = 1.0;
    mat_elem(A, 0, 1) = 2.0;
    mat_elem(A, 0, 2) = 3.0;
    mat_elem(A, 1, 0) = 2.0;
    mat_elem(A, 1, 1) = 2.0;
    mat_elem(A, 1, 2) = 3.0;
    mat_elem(A, 2, 0) = 3.0;
    mat_elem(A, 2, 1) = 3.0;
    mat_elem(A, 2, 2) = 3.0;

    mat_elem(B, 0, 0) = -1.0;
    mat_elem(B, 0, 1) = 1.0;
    mat_elem(B, 0, 2) = 0.0;
    mat_elem(B, 1, 0) = 1.0;
    mat_elem(B, 1, 1) = -2.0;
    mat_elem(B, 1, 2) = 1.0;
    mat_elem(B, 2, 0) = 0.0;
    mat_elem(B, 2, 1) = 1.0;
    mat_elem(B, 2, 2) = -2.0 / 3.0;

    // 正しく逆行列が求まるかどうか
    ASSERT_TRUE(mat_inverse(&invA, A));

    // 逆行列の値が正しいかどうか
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            ASSERT_EQUAL(mat_elem(B, i, j), mat_elem(invA, i, j));
        }
    }

    // 同じ行列を入れても逆行列が求まるかどうか
    ASSERT_TRUE(mat_inverse(&A, A));

    // 逆行列の値が正しいかどうか
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            ASSERT_EQUAL(mat_elem(B, i, j), mat_elem(A, i, j));
        }
    }

    // 特異な行列
    mat_elem(A, 0, 0) = 1.0;
    mat_elem(A, 0, 1) = 2.0;
    mat_elem(A, 0, 2) = 3.0;
    mat_elem(A, 1, 0) = 2.0;
    mat_elem(A, 1, 1) = 4.0;
    mat_elem(A, 1, 2) = 6.0;
    mat_elem(A, 2, 0) = 3.0;
    mat_elem(A, 2, 1) = 6.0;
    mat_elem(A, 2, 2) = 9.0;

    // 逆行列が求まら「ない」かどうか
    ASSERT_FALSE(mat_inverse(&invA, A));

    mat_free(&A);
    mat_free(&invA);
    mat_free(&B);
}

TESTCASE(mat_inverse)
{
    const int size = 100;

    SAFE_DECLARE(matrix, A);
    SAFE_DECLARE(matrix, B);
    SAFE_DECLARE(matrix, invA);
    SAFE_DECLARE(matrix, I);

    // 非正方行列がエラーになる
    mat_alloc(&A, size, size + 314);
    mat_alloc(&invA, size + 314, size);
    ASSERT_TRUE(is_valid_mat(A));
    ASSERT_TRUE(is_valid_mat(invA));
    ASSERT_FALSE(mat_inverse(&invA, A));
    mat_free(&A);
    mat_free(&invA);

    // ランダムな正方行列を用意
    mat_alloc(&A, size, size);
    mat_alloc(&invA, size, size);
    mat_alloc(&I, size, size);
    mat_alloc(&B, size, size);

    ASSERT_TRUE(is_valid_mat(A));
    ASSERT_TRUE(is_valid_mat(invA));
    ASSERT_TRUE(is_valid_mat(B));
    ASSERT_TRUE(is_valid_mat(I));

    mat_rand(&A);
    mat_ident(&I);

    // 逆行列が計算できるかどうか (ほぼ100%できるはず)
    ASSERT_TRUE(mat_inverse(&invA, A));

    // 逆行列との積が単位行列になるかどうか
    mat_mul(&B, invA, A);
    ASSERT_TRUE(mat_equal(B, I));

    mat_mul(&B, A, invA);
    ASSERT_TRUE(mat_equal(B, I));

    // 同じ行列をいれても逆行列が求まるかどうか
    ASSERT_TRUE(mat_inverse(&A, A));

    // さっき求めた逆行列と値が同じかどうか
    ASSERT_TRUE(mat_equal(invA, A));

    mat_free(&A);
    mat_free(&invA);
    mat_free(&I);
    mat_free(&B);
}

int main()
{
    TEST_INIT();
    unsigned long seed = (unsigned long)time(0) % 65536;
    printf("seed: %lu\n", seed);
    srand(seed);

    // 連立一次方程式と行列 (その2)
    RUN_TEST(mat_alloc_and_free);
    RUN_TEST(mat_copy);
    RUN_TEST(mat_add);
    RUN_TEST(mat_sub);
    RUN_TEST(mat_mul);
    RUN_TEST(mat_muls);
    RUN_TEST(mat_ident);
    RUN_TEST(mat_trans);
    RUN_TEST(mat_equal);

    // 連立一次方程式と行列 (その3)
    // 「その2」の課題に取り組んでいるときは適宜コメントアウトすること
    RUN_TEST(mat_solve_simple);
    RUN_TEST(mat_inverse_simple);
    RUN_TEST(mat_inverse);

    TEST_FINISH();
}
