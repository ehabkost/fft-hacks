#include <stdlib.h>
#include <stdio.h>
#include <sndfile.h>
#include <assert.h>

#define SLICE 1000

int main(int argc, const char *argv[])
{
	SNDFILE *in, *out;
	SF_INFO iinfo, oinfo;

	if (argc < 3) {
		fprintf(stderr, "usage: %s input.wav output.wav\n", argv[0]);
		exit(1);
	}

	printf("opening input:\n");
	iinfo.format = 0;
	in = sf_open(argv[1], SFM_READ, &iinfo);
	if (!in) {
		perror("open input");
		exit(1);
	}

	printf("%d channels, %d samplerate\n", iinfo.channels, iinfo.samplerate);
	assert(iinfo.channels == 1);

	printf("opening output:\n");
	oinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_32;
	oinfo.samplerate = iinfo.samplerate;
	oinfo.channels = iinfo.channels;
	out = sf_open(argv[2], SFM_WRITE, &oinfo);
	if (!out) {
		perror("open output");
		exit(1);
	}

	for (;;) {
		sf_count_t cnt;
		double buf[SLICE];
		int i, j;

		cnt = sf_read_double(in, buf, SLICE);
		if (!cnt)
			break;
		for (i = 0; i < cnt; i++) {
			buf[i] /= 2.0;
		}
		if (sf_write_double(out, buf, cnt) != cnt) {
			perror("writing samples");
			exit(1);
		}
	}

	printf("data converted!\n");

	sf_close(in);
	sf_close(out);
	return 0;
}
