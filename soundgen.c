#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "wavfile.h"

#define TRUE 1
#define FALSE 0

struct Tone {
	float duration;
	float frequency;
	float amplitude_perc;
};

short * calculate_tone(double duration, double frequency, double amplitude) {

        int n_samples = (int) WAVFILE_SAMPLES_PER_SECOND * duration;
        int volume = (int) 32768 * (amplitude / 100);

        short * waveform = malloc(sizeof(short) * n_samples);

        for (int i=0; i<n_samples;i++) {
                double t = (double) i / WAVFILE_SAMPLES_PER_SECOND;
                waveform[i] = (int) volume * sin(frequency * t * 2 * M_PI);
        }

        return waveform;

}

int main(int argc, char * argv[]) {

	int opt_play = FALSE;

	if (argc < 3) {
		printf("Not enough arguments.\n");
		printf("Usage: %s tones_file wav_output_file [--play]\n", argv[0]);
		return 1;
	}

	if (argc == 3) {
		if (access(argv[1], F_OK) == -1) {
			printf("File %s does not exist.\n", argv[1]);
			return 1;
		}
	}

	if (argc > 3) {
		if (strcmp(argv[3], "--play") == 0) {
			opt_play = TRUE;
		}
	}

	FILE * tones_fp = fopen(argv[1], "r");

	if (tones_fp == NULL) {
		printf("Could not open file %s for reading.\n", argv[1]);
		return 1;
	}

	char line[512];

	char **lines = NULL;

	int n_tones = 0;

	while (fgets(line, sizeof(line), tones_fp)) {
		lines = (char **) realloc(lines, (n_tones+1) * sizeof(char *));
		lines[n_tones++] = strdup(line);
	}

	struct Tone tones[n_tones];

	float d, f, a = 0.0;
	char sd[256], sf[256], sa[256];

	float total_duration = 0.0;


	for (int i=0; i<n_tones; i++) {
		sscanf(lines[i], "%s %s %s\n", sd, sf, sa);
		d = atof(sd);
		f = atof(sf);
		a = atof(sa);
		tones[i].duration = d;
		tones[i].frequency = f;
		tones[i].amplitude_perc = a;
		total_duration += d;
	}

	int total_samples = (int) (total_duration * WAVFILE_SAMPLES_PER_SECOND);

	short * final_waveform = malloc(total_samples * sizeof(short));

	printf("%d\n", total_samples);

	int samples_done = 0;

	for (int i=0; i<n_tones; i++) {
		int n_samples = (int) WAVFILE_SAMPLES_PER_SECOND * tones[i].duration;
		short * wavepart = calculate_tone(tones[i].duration, tones[i].frequency, tones[i].amplitude_perc);
		memcpy(final_waveform + samples_done, wavepart, n_samples * sizeof(short));
		samples_done += n_samples;
		printf("Tone %d: d=%f f=%f a=%f n_samp=%d samp_done=%d\n",
				i, tones[i].duration, tones[i].frequency, tones[i].amplitude_perc, n_samples, samples_done);
	}

	FILE * out_fp = wavfile_open(argv[2]);

	if (out_fp == NULL) {
		printf("Could not open file %s for writing.\n", argv[2]);
		return 1;
	}

	wavfile_write(out_fp, final_waveform, total_samples);

	wavfile_close(out_fp);

	fclose(tones_fp);

	char commandbuffer[512];

	sprintf(commandbuffer, "aplay '%500s'", argv[2]);

	if (opt_play == TRUE) {
		system(commandbuffer);
	}

}
