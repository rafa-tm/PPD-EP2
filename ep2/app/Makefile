CC=gcc

all: clean sequential pthread

sequential:
	$(CC) laplace_seq.c -o laplace_seq

pthread:
	$(CC) laplace_pth.c -o laplace -lpthread

clean:
	rm -f laplace_seq laplace
