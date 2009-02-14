CFLAGS = -g
LDFLAGS = -lsndfile

fft: fft.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
