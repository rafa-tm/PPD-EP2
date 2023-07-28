/*
** Programa : multiplicacao de matrizes
** Objetivo: paralelizacao com MPI
*/

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

float *A, *B, *C;

// Informações a serem enviadas pelo rank 0 aos demais processos
struct info {
  int lin_a;    // número de linhas da matriz A
  int col_a;    // número de colunas da matriz A
  int lin_b;    // número de linhas da matriz B
  int col_b;    // número de colunas da matriz B
  int inic;     // número da linha inicial que o processo irá calcular
  int numlin;   // número de linhas que serão calculadas pelo processo
} s_info;

int
main(int argc, char *argv[])
{
	int lin_a,col_a,lin_b,col_b,lin_c,col_c;
	int i,j,k, t;
	int numtasks, rank;
	int result;
	int numlin, resto;

	// Todos os processos iniciam a biblioteca e determinam seus ranks na aplicação

	result = MPI_Init(&argc,&argv);
	if (result != MPI_SUCCESS) {
		fprintf(stderr,"Erro iniciando MPI: %d\n",result);
		MPI_Abort(MPI_COMM_WORLD, result);
	}
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);


/*
  Atividades do processo com rank 0:
  ---------------------------------
  - determinar dimensões das matrizes
  - alocar espaço e carregar os dados das matrizes na memória local
  - determinar como será a divisão do trabalho entre os processos
  - enviar a cada processo as informações sobre as matrizes e sobre os cálculos que este irá realizar
  - enviar a cada processo as informações das matrizes pertinentes para os cálculos
  - receber resultados dos cálculos

  Atividades dos processos rank > 0:
  ---------------------------------
  - Aguardar (e receber) informações sobre as matrizes e a divisão do trabalho
  - Receber os dados pertinentes das matrizes, posicionando-os em memória para os cálculos
  - Enviar os valores processados de volta ao processo de rank 0
*/

	if(rank==0) {

    /********** Atividades do processo com rank 0 *************************/

    // Nó com rank 0 faz a leitura das dimensõe das matrizes

		setbuf(stdout,NULL); // para forçar a exibição imediata dos textos no terminal

		printf("Linhas A: ");
		scanf("%d",&lin_a);
		printf("Colunas A / Linhas B: ");
		scanf("%d",&col_a);
		lin_b=col_a;
		printf("Colunas B: ");
		scanf("%d",&col_b);
		printf("\n");
		lin_c=lin_a;
		col_c=col_b;

  	// Nó rank 0 aloca espaço para as matrizes e as preenche de forma aleatória.

		// Alocacao dinâmica das matrizes, com linhas em sequência
		A=(float *)malloc(lin_a*col_a*sizeof(float));
		B=(float *)malloc(lin_b*col_b*sizeof(float));
		C=(float *)malloc(lin_c*col_c*sizeof(float));

		// Inicia gerador de números aleatórios. Comentar comando a seguir se quiser
		// gerar sempre os mesmos valores para uniformidade nos cálculos.
		srandom(time(NULL));

		for(i=0; i < lin_a * col_a; i++)
			A[i]=(float)rand() / (float)RAND_MAX;

		for(i=0; i < lin_b * col_b; i++)
			B[i]=(float)rand() / (float)RAND_MAX;

	  // Envio das matrizes para os processos.
    // O que enviar depende de como os cálculos serão divididos.
    // Aqui, considerando divisão das linhas de C

		// preenche informações sobre as matrizes
		s_info.lin_a = lin_a;
		s_info.col_a = col_a;
		s_info.lin_b = lin_b;
		s_info.col_b = col_b;

		// informações sobre a linha inicial e o número de linhas depende do rank

		// Determina quantas e quais linhas cada processo (rank) vai calcular
		// rank 0 não irá participar dos cálculos...

		numlin = lin_a / (numtasks -1);
		resto = lin_a % (numtasks -1);

		// Determina linha inicial e número de linhas para cada processo rank > 0
    // e lhe envia linhas apropriadas

		for(t=1; t < numtasks; t++) {

			// Determina informações sobre linhas a calcular pelo processo
			s_info.numlin = numlin;
			if(t <= resto)
				s_info.numlin += 1;  // resto primeiros processos recebem 1 linha a mais
			s_info.inic = (t-1) * numlin;
			if(resto) {
				if(t<=resto)
					s_info.inic += t-1;    // resto primeiros processos recebem 1 linha a mais
				else
					s_info.inic += resto;   // resto primeiros processos recebem 1 linha a mais
			}
      // Envia informações de controle para demais processos rank > 0

			MPI_Send( &s_info, sizeof(s_info), MPI_FLOAT, t, 1, MPI_COMM_WORLD);

			// Envia linhas de A aos processos rank = t (>0)
			// Enviar 1 linha de cada vez ou todas em sequência?
			// Fazer o recebimento correspondende nos demais ranks

			for (i=s_info.inic; i < s_info.inic + s_info.numlin; i++)
				MPI_Send( &A[i*lin_a], col_a, MPI_FLOAT, t, 1, MPI_COMM_WORLD);
    }

  	// Todos os processos precisam da matriz B inteira.
	 	// Como vai enviá-la, replicando ou em Bcast?
	  // Bcast da matriz inteira ou linha por linha?
	  for (i=0; i < lin_b; i++)
		  MPI_Bcast (&B[i*col_b], col_b, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Recebe resultados finais. Rank 0 recebe linhas de C
	  // É possível receber as linhas de C fora de ordem?

  	for(t=1; t < numtasks; t++) {

	  	// Determina informações sobre linhas que foram calculadas por cada processo rank > 0
		 	// Poderia ter salvo essas infos, já calculadas no envio, num vetor de parâmetros...

	  	s_info.numlin = numlin;
	  	if(t <= resto)
	  		s_info.numlin += 1;  // resto primeiros processos recebem 1 linha a mais
	  	s_info.inic = (t-1) * numlin;
	  	if(resto) {
	  		if(t<=resto)
	  			s_info.inic += t-1;    // resto primeiros processos recebem 1 linha a mais
	  		else
	  			s_info.inic += resto;   // resto primeiros processos recebem 1 linha a mais
	  	}

	  	// Recebe as linhas de C calculadas em cara process rank > 0

	  	for (i=s_info.inic; i < s_info.inic + s_info.numlin; i++)
	  		MPI_Recv( &C[i*col_c], col_c, MPI_FLOAT, t, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

	} else {

    /********** Atividades dos processos com rank > 0 *************************/

  	// Recebimento das matrizes e parâmetros para os cálculos

		// Recebem informações sobre as matrizes
		MPI_Recv (&s_info, sizeof(s_info), MPI_FLOAT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	
		printf("%d recebeu: A[%d,%d], B[%d,%d], inic: %d, numlin: %d\n",
			rank, s_info.lin_a, s_info.col_a, s_info.lin_b, s_info.col_b, s_info.inic, s_info.numlin);
	
		// Alocacam espaços para as matrizes.
		// Alocam todo o espaço ou apenas para conter as linhas que irão manipular?
		// Matrizes A e C precisam de espaço apenas para as linhas que serão manipuladas pelo processo

		// Alocacao dinâmica das matrizes, com linhas em sequência
		B = (float *)malloc( s_info.lin_b * s_info.col_b * sizeof(float));
		// A = (float *)malloc( s_info.lin_a * s_info.col_a * sizeof(float));
		A = (float *)malloc( s_info.numlin * s_info.col_a * sizeof(float));
		// C = (float *)malloc( s_info.lin_a * s_info.col_b * sizeof(float));
		C = (float *)malloc( s_info.numlin * s_info.col_b * sizeof(float));

		// Recebe linhas de A (s_info.numlin), posicionando-as na matriz alocada
		for (i=0; i < s_info.numlin; i++)
			MPI_Recv( &A[i*s_info.col_a], s_info.col_a, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Recebe as linhas de B enviadas em Bcast
		for (i=0; i < s_info.lin_b; i++)
			MPI_Bcast (&B[i*s_info.col_b], s_info.col_b, MPI_FLOAT, 0, MPI_COMM_WORLD);

  	// cálculo da multiplicacao, feito pelos processos de rank > 0

		// Cada processo calcula s_info.numlin linhas

		for ( i=0; i < s_info.numlin; i++)
			for ( j=0; j < s_info.col_b; j++) {
				C[ i * s_info.col_b +j ] = 0;
				for ( k=0; k < s_info.col_a; k++)
					C[ i * s_info.col_b +j ] += A[ i * s_info.col_a +k ] * B[ k * s_info.col_b +j ];
			}

		// Envia linhas da matriz C calculadas localmente ao process rank 0

		for (i=0; i < s_info.numlin; i++)
			MPI_Send( &C[i*s_info.lin_a], s_info.col_b, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
	}

  // Se quiser testar os resultados produzidos de forma paralela
  if(rank==0) {
   // Cálculo sequencial para comparações
  	float *AB=(float *)malloc(lin_c*col_c*sizeof(float));
  	for(i=0; i < lin_c; i++)
  		for(j=0; j < col_c; j++) {
  			AB[i*col_c+j]=0;
  			for(k=0; k < col_a; k++)
  				AB[i*col_c+j] = AB[i*col_c+j] + A[i*col_a+k] * B[k*col_b+j];
  		}
   // Comparação da matriz calculada sequencialmente com a matriz calculada em paralelo
  	for(i=0;i<lin_c;i++)
  		for(j=0;j<col_c;j++)
  			if(C[i*col_c+j] != AB[i*col_c+j])
  				printf("Erro em %d,%d\n",i,j);
  }

	// Todos os processos

  // Libera áreas de memória
  free(A); free(B); free(C);

	MPI_Finalize();

	return(0);
}