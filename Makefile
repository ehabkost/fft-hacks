CFLAGS = -g
LDFLAGS = -lsndfile -lfftw -lm

fft: fft.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
