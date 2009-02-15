#include <stdlib.h>
#include <stdio.h>
#include <sndfile.h>
#include <assert.h>
#include <fftw.h>
#include <complex.h>
#include <math.h>

#define SLICE 8000
#define SLICEIN SLICE
#define SLICEOUT SLICE

double f_cabs(fftw_complex c)
{
	return cabs(c.re + c.im*I);
}

void dofiltering(fftw_complex *fftin, fftw_complex *fftout)
{
	int i;

	for (i = 0; i < SLICEOUT; i++) {
		fftout[i].im = fftout[i].re = 0;
	}

	/* Low-pass filter */
	for (i = 0; i < SLICEOUT/4; i++) {
		fftout[i] = fftin[i];
	}
}

int main(int argc, const char *argv[])
{
	SNDFILE *in, *out;
	SF_INFO iinfo, oinfo;
	fftw_plan pi, po;

	if (argc < 3) {
		fprintf(stderr, "usage: %s input.wav output.wav\n", argv[0]);
		exit(1);
	}

	pi = fftw_create_plan(SLICEIN, FFTW_FORWARD, FFTW_ESTIMATE);
	if (!pi) {
		perror("create plan");
		exit(1);
	}
	po = fftw_create_plan(SLICEOUT, FFTW_BACKWARD, FFTW_ESTIMATE);
	if (!po) {
		perror("create plan");
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
		double inbuf[SLICEIN];
		fftw_complex inbuf_c[SLICEIN];
		fftw_complex fftin[SLICEIN];
		fftw_complex fftout[SLICEOUT];
		fftw_complex outbuf_c[SLICEOUT];
		double outbuf[SLICEOUT];
		int i, j;

		cnt = sf_read_double(in, inbuf, SLICEIN);
		if (!cnt)
			break;

		/* real to complex */
		for (i = 0; i < cnt; i++) {
			inbuf_c[i].im = 0;
			inbuf_c[i].re = inbuf[i];
		}
		/* pad with zeros, just in case */
		for (i = cnt; i < SLICEIN; i++) {
			inbuf_c[i].im = 0;
			inbuf_c[i].re = 0;
		}

		/* FFT */
		fftw_one(pi, inbuf_c, fftin);

		/* Filtering */
		dofiltering(fftin, fftout);

		/* Reverse FFT */
		fftw_one(po, fftout, outbuf_c);

		/* complex to real, and normalization */
		for (i = 0; i < SLICEOUT; i++) {
			double e;
			outbuf[i] = outbuf_c[i].re / SLICEIN;
		}

		if (sf_write_double(out, outbuf, cnt) != cnt) {
			perror("writing samples");
			exit(1);
		}
	}

	printf("data converted!\n");

	sf_close(in);
	sf_close(out);
	return 0;
}
