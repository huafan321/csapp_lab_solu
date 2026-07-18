/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 64)
    {
        int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, i, j, k;
        for (i = 0; i < N; i += 8)
        {
            for (j = 0; j < M; j += 8)
            {
                for (k = 0; k < 4; k++)
                {
                    tmp1 = A[i + k][j];
                    tmp2 = A[i + k][j + 1];
                    tmp3 = A[i + k][j + 2];
                    tmp4 = A[i + k][j + 3];
                    tmp5 = A[i + k][j + 4];
                    tmp6 = A[i + k][j + 5];
                    tmp7 = A[i + k][j + 6];
                    tmp8 = A[i + k][j + 7];
                    B[j][i + k] = tmp1;
                    B[j + 1][i + k] = tmp2;
                    B[j + 2][i + k] = tmp3;
                    B[j + 3][i + k] = tmp4;
                    B[j][i + k + 4] = tmp5;
                    B[j + 1][i + k + 4] = tmp6;
                    B[j + 2][i + k + 4] = tmp7;
                    B[j + 3][i + k + 4] = tmp8;
                }
                for (k = 0; k < 4; k++)
                {

                    tmp5 = B[j + k][i + 4];
                    tmp6 = B[j + k][i + 5];
                    tmp7 = B[j + k][i + 6];
                    tmp8 = B[j + k][i + 7];

                    tmp1 = A[i + 4][j + k];
                    tmp2 = A[i + 5][j + k];
                    tmp3 = A[i + 6][j + k];
                    tmp4 = A[i + 7][j + k];

                    B[j + k][i + 4] = tmp1;
                    B[j + k][i + 5] = tmp2;
                    B[j + k][i + 6] = tmp3;
                    B[j + k][i + 7] = tmp4;

                    B[j + 4 + k][i] = tmp5;
                    B[j + 4 + k][i + 1] = tmp6;
                    B[j + 4 + k][i + 2] = tmp7;
                    B[j + 4 + k][i + 3] = tmp8;
                }

                for (k = 0; k < 4; k++)
                {
                    tmp1 = A[i + 4 + k][j + 4];
                    tmp2 = A[i + 4 + k][j + 5];
                    tmp3 = A[i + 4 + k][j + 6];
                    tmp4 = A[i + 4 + k][j + 7];
                    B[j + 4][i + 4 + k] = tmp1;
                    B[j + 5][i + 4 + k] = tmp2;
                    B[j + 6][i + 4 + k] = tmp3;
                    B[j + 7][i + 4 + k] = tmp4;
                }
            }
        }
    }
    else if (M == 32)
    {
        int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, i, j, k;
        for (i = 0; i < N; i += 8)
        {
            for (j = 0; j < M; j += 8)
            {
                for (k = 0; k < 8; k++)
                {
                    tmp1 = A[i + k][j];
                    tmp2 = A[i + k][j + 1];
                    tmp3 = A[i + k][j + 2];
                    tmp4 = A[i + k][j + 3];
                    tmp5 = A[i + k][j + 4];
                    tmp6 = A[i + k][j + 5];
                    tmp7 = A[i + k][j + 6];
                    tmp8 = A[i + k][j + 7];
                    B[j][i + k] = tmp1;
                    B[j + 1][i + k] = tmp2;
                    B[j + 2][i + k] = tmp3;
                    B[j + 3][i + k] = tmp4;
                    B[j + 4][i + k] = tmp5;
                    B[j + 5][i + k] = tmp6;
                    B[j + 6][i + k] = tmp7;
                    B[j + 7][i + k] = tmp8;
                }
            }
        }
    }
    else
    {
        int i, j, ii, jj;

        for (i = 0; i < N; i += 16)
        {
            for (j = 0; j < M; j += 16)
            {
                for (ii = i; ii < i + 16 && ii < N; ii++)
                {
                    for (jj = j; jj < j + 16 && jj < M; jj++)
                    {
                        B[jj][ii] = A[ii][jj];
                    }
                }
            }
        }
    }
}
/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
