#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// 要素を交換するマクロ
#define swap(a, b)      \
    do                  \
    {                   \
        double t = (a); \
        a = b;          \
        b = t;          \
    } while (0);

/*
 * 行列用構造体
 * rows: 行数
 * cols: 列数
 * elems: 行列要素を入れた一次元配列
 */
typedef struct
{
    int rows;
    int cols;
    double *elems;
} matrix;

// 行列要素を取得するマクロ
#define mat_elem(m, i, j) (m).elems[(i) * (m).cols + (j)]
#define pmat_elem(m, i, j) (m)->elems[(i) * (m)->cols + (j)]

// ----------------------------------------------------------------------------
// 行列演算用関数群
// ----------------------------------------------------------------------------

bool mat_same_size(matrix mat1, matrix mat2)
{
    if (mat1.cols != mat2.cols || mat1.rows != mat2.rows)
        return false;
    return true;
}
// mat_alloc: 行列要素用のメモリを確保する
bool mat_alloc(matrix *mat, int rows, int cols)
{
    if (rows <= 0 || cols <= 0)
        return false;
    mat->rows = rows;
    mat->cols = cols;
    mat->elems = (double *)malloc(rows * cols * sizeof(double));

    return true;
}

// mat_free: 使い終わった行列のメモリを解放する
void mat_free(matrix *mat)
{
    free(mat->elems);
    mat->cols = 0;
    mat->rows = 0;
    mat->elems = NULL;
}

// mat_print: 行列の中身を表示する
void mat_print(matrix mat)
{
    if (mat.rows == 0 || mat.cols == 0 || mat.elems == NULL)
    {
        fprintf(stderr, "Matrix is NULL or zero size!\n");
        return;
    }

    for (int i = 0; i < mat.rows; i++)
    {
        for (int j = 0; j < mat.cols; j++)
        {
            printf("%6.4f%s", mat_elem(mat, i, j), (j == mat.cols - 1) ? "\n" : "  ");
        }
    }
}

// mat_copy: srcの中身を*dstにコピーする
bool mat_copy(matrix *dst, matrix src)
{
    if (!mat_same_size(*dst, src))
        return false;
    if (!mat_alloc(dst, src.rows, src.cols))
        return false;
    memcpy(dst->elems, src.elems, dst->rows * dst->cols * sizeof(double));
    return true;
}

// mat_add: mat1+mat2を*resに代入する
bool mat_add(matrix *res, matrix mat1, matrix mat2)
{
    if (!mat_same_size(*res, mat1) || !mat_same_size(mat1, mat2) || !mat_same_size(mat2, *res))
        return false;
    for (int i = 0; i < res->rows * res->cols; i++)
    {
        res->elems[i] = mat1.elems[i] + mat2.elems[i];
    }
    return true;
}

// mat_sub: mat1-mat2を*resに代入する
bool mat_sub(matrix *res, matrix mat1, matrix mat2)
{
    if (!mat_same_size(*res, mat1) || !mat_same_size(mat1, mat2) || !mat_same_size(mat2, *res))
        return false;
    for (int i = 0; i < res->rows * res->cols; i++)
    {
        res->elems[i] = mat1.elems[i] - mat2.elems[i];
    }
    return true;
}

// mat_mul: mat1とmat2の行列積を*resに代入する
bool mat_mul(matrix *res, matrix mat1, matrix mat2)
{
    if (mat1.cols != mat2.rows || res->rows != mat1.rows || res->cols != mat2.cols)
        return false;
    matrix tmp;
    mat_alloc(&tmp, res->rows, res->cols);
    for (int i = 0; i < res->rows; i++)
    {
        for (int j = 0; j < res->cols; j++)
        {
            mat_elem(tmp, i, j) = 0.0;
            for (int k = 0; k < mat1.cols; k++)
            {
                mat_elem(tmp, i, j) += mat_elem(mat1, i, k) * mat_elem(mat2, k, j);
            }
        }
    }
    memcpy(res->elems, tmp.elems, res->rows * res->cols * sizeof(double));
    mat_free(&tmp);
    return true;
}

// mat_muls: matをc倍（スカラー倍）した結果を*resに代入する
bool mat_muls(matrix *res, matrix mat, double c)
{
    if (!mat_same_size(*res, mat))
        return false;
    for (int i = 0; i < res->rows * res->cols; i++)
    {
        res->elems[i] = mat.elems[i] * c;
    }
    return true;
}

// mat_trans: matの転置行列を*resに代入する
bool mat_trans(matrix *res, matrix mat)
{
    if (res->cols != mat.rows || res->rows != mat.cols)
        return false;
    for (int i = 0; i < mat.rows; i++)
    {
        for (int j = 0; j < mat.cols; j++)
        {
            mat_elem(*res, j, i) = mat_elem(mat, i, j);
        }
    }
    return true;
}

// mat_unit: 単位行列を与える
bool mat_ident(matrix *mat)
{
    if (mat->cols != mat->rows)
        return false;
    for (int i = 0; i < mat->rows; i++)
    {
        for (int j = 0; j < mat->cols; j++)
        {
            if (i == j)
            {
                mat_elem(*mat, i, j) = 1;
            }
            else
            {
                mat_elem(*mat, i, j) = 0;
            }
        }
    }
    return true;
}

// mat_equal: mat1とmat2が等しければtrueを返す
bool mat_equal(matrix mat1, matrix mat2)
{
    if (!mat_same_size(mat1, mat2))
        return false;
    for (int i = 0; i < mat1.rows; i++)
    {
        for (int j = 0; j < mat1.cols; j++)
        {
            if (mat_elem(mat1, i, j) != mat_elem(mat2, i, j))
            {
                return false;
            }
        }
    }
    return true;
}

// mat_solve: 連立一次方程式 ax=b を解く．ピボット選択付き
bool mat_solve(matrix *x, matrix A_, matrix b_)
{
    return false;
}

// mat_inverse: 行列Aの逆行列を*invAに与える
bool mat_inverse(matrix *invA, matrix A)
{
    return false;
}
