% % writefile mult.c
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

	float *A,
	*B, *C;

struct info
{
	int lin_a;
	int col_a;
	int lin_b;
	int col_b;
	int inic;
	int numlin;
} s_info;

int main(int argc, char *argv[])
{
	int lin_a, col_a, lin_b, col_b, lin_c, col_c;
	int i, j, k, t;
	int numtasks, rank;
	int result;
	int numlin, resto;

	result = MPI_Init(&argc, &argv);
	if (result != MPI_SUCCESS)
	{
		fprintf(stderr, "Erro iniciando MPI: %d\n", result);
		MPI_Abort(MPI_COMM_WORLD, result);
	}
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		setbuf(stdout, NULL);

		printf("Linhas A: ");
		scanf("%d", &lin_a);

		printf("Colunas A / Linhas B: ");
		scanf("%d", &col_a);

		lin_b = col_a;

		printf("Colunas B: ");
		scanf("%d", &col_b);
		printf("\n");

		lin_c = lin_a;
		col_c = col_b;

		A = (float *)malloc(lin_a * col_a * sizeof(float));
		B = (float *)malloc(lin_b * col_b * sizeof(float));
		C = (float *)malloc(lin_c * col_c * sizeof(float));

		srandom(time(NULL));

		for (i = 0; i < lin_a * col_a; i++)
			A[i] = (float)rand() / (float)RAND_MAX;

		for (i = 0; i < lin_b * col_b; i++)
			B[i] = (float)rand() / (float)RAND_MAX;

		s_info.lin_a = lin_a;
		s_info.col_a = col_a;
		s_info.lin_b = lin_b;
		s_info.col_b = col_b;

		numlin = lin_a / (numtasks - 1);
		resto = lin_a % (numtasks - 1);

		for (t = 1; t < numtasks; t++)
		{

			s_info.numlin = numlin;
			if (t <= resto)
				s_info.numlin += 1;
			s_info.inic = (t - 1) * numlin;
			if (resto)
			{
				if (t <= resto)
					s_info.inic += t - 1;
				else
					s_info.inic += resto;
			}

			MPI_Send(&s_info, sizeof(s_info), MPI_INT, t, 1, MPI_COMM_WORLD);
		}

		MPI_Scatter(A, s_info.numlin * col_a, MPI_FLOAT, A, s_info.numlin * col_a, MPI_FLOAT, 0, MPI_COMM_WORLD);

		// Use MPI_Bcast para transmitir a matriz B para todos os processos
		MPI_Bcast(B, lin_b * col_b, MPI_FLOAT, 0, MPI_COMM_WORLD);

		MPI_Gather(C, s_info.numlin * col_c, MPI_FLOAT, C, s_info.numlin * col_c, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Recv(&s_info, sizeof(s_info), MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		printf("%d recebeu: A[%d,%d], B[%d,%d], inic: %d, numlin: %d\n",
			   rank, s_info.lin_a, s_info.col_a, s_info.lin_b, s_info.col_b, s_info.inic, s_info.numlin);

		B = (float *)malloc(s_info.lin_b * s_info.col_b * sizeof(float));
		A = (float *)malloc(s_info.numlin * s_info.col_a * sizeof(float));
		C = (float *)malloc(s_info.numlin * s_info.col_b * sizeof(float));

		// Use MPI_Scatter para receber as partes de A
		MPI_Scatter(NULL, s_info.numlin * s_info.col_a, MPI_FLOAT, A, s_info.numlin * s_info.col_a, MPI_FLOAT, 0, MPI_COMM_WORLD);

		// Use MPI_Bcast para receber a matriz B
		MPI_Bcast(B, s_info.lin_b * s_info.col_b, MPI_FLOAT, 0, MPI_COMM_WORLD);

		for (i = 0; i < s_info.numlin; i++)
			for (j = 0; j < s_info.col_b; j++)
			{
				C[i * s_info.col_b + j] = 0;
				for (k = 0; k < s_info.col_a; k++)
					C[i * s_info.col_b + j] += A[i * s_info.col_a + k] * B[k * s_info.col_b + j];
			}

		// Use MPI_Gather para enviar as partes de C
		MPI_Gather(C, s_info.numlin * s_info.col_b, MPI_FLOAT, NULL, s_info.numlin * s_info.col_b, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}

	if (rank == 0)
	{
		float *AB = (float *)malloc(lin_c * col_c * sizeof(float));
		for (i = 0; i < lin_c; i++)
			for (j = 0; j < col_c; j++)
			{
				AB[i * col_c + j] = 0;
				for (k = 0; k < col_a; k++)
					AB[i * col_c + j] = AB[i * col_c + j] + A[i * col_a + k] * B[k * col_b + j];
			}

		for (i = 0; i < lin_c; i++)
			for (j = 0; j < col_c; j++)
				if (C[i * col_c + j] != AB[i * col_c + j])
					printf("Erro em %d,%d\n", i, j);
	}

	free(A);
	free(B);
	free(C);

	MPI_Finalize();

	return (0);
}