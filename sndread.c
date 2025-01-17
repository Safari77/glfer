/*

 * Copyright (C) 2001 Claudio Girardi
 * This file is derived from bplay, (C) David Monro 1996
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "util.h"
#include <sndfile.h>

#include "sndread.h"

#define MSPEED  1
#define MBITS   2
#define MSTEREO 4

int speed, bits, stereo; /* Audio parameters */
static float *floatbuff = NULL;
static size_t out_len = 1024;  // FIXME

SNDFILE *sndfile;  // Handle to the open audio file
SF_INFO sfinfo;    // Holds audio metadata (channels, samplerate, etc.)

int open_sndfile(char *fname, size_t n, int *speed) {
    // Sets size of data block to be read
    out_len = n;
    fprintf(stderr, "open_sndfile %s n=%u  speed=%u\n", fname, n, *speed);

    if (sndfile) {
        fprintf(stderr, "Closing sndfile %p\n", sndfile);
        sf_close(sndfile);
        sndfile = NULL;
    }
    // Initialize SF_INFO to zero before opening the file
    memset(&sfinfo, 0, sizeof(SF_INFO));

    // Open the audio file with libsndfile
    sndfile = sf_open(fname, SFM_READ, &sfinfo);
    if (!sndfile) {
        fprintf(stderr, "Error opening file %s: %s\n", fname, sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }

    // Set metadata values
    *speed = sfinfo.samplerate;
    stereo = (sfinfo.channels > 1) ? 1 : 0;
    // Bits per sample
    if (sfinfo.format & SF_FORMAT_PCM_16) {
        bits = 16;
    } else if (sfinfo.format & SF_FORMAT_PCM_24) {
        bits = 24;
    } else if (sfinfo.format & SF_FORMAT_PCM_32) {
        bits = 32;
    } else if (sfinfo.format & SF_FORMAT_PCM_U8 || sfinfo.format & SF_FORMAT_PCM_S8) {
        bits = 8;
    } else {
        bits = 0;  // Not a fixed-width PCM format
    }

    fprintf(stderr, "Audio format: %d channels, %d Hz, %d-bit PCM or float ...\n", sfinfo.channels, sfinfo.samplerate,
            bits);

    return 0;  // Success
}

/* reads the audio data */
void sndfile_read(float **buf_out, size_t *n_out) {
    if (!floatbuff) {
        if (out_len > (SIZE_MAX / sizeof(float) / sfinfo.channels)) {
            fprintf(stderr, "Integer overflow avoided in sndfile_read calloc\n");
            exit(EXIT_FAILURE);
        }
        floatbuff = (float *)calloc(out_len * sfinfo.channels, sizeof(float));
        if (!floatbuff) {
            fprintf(stderr, "Memory allocation failed\n");
            sf_close(sndfile);
            exit(EXIT_FAILURE);
        }
    }
    // Read audio samples (returns frames, not samples)
    size_t frames_read = sf_readf_float(sndfile, floatbuff, out_len);
    //*n_out = frames_read * sfinfo.channels; // Total number of samples
    *n_out = frames_read ? 1 : 0;
    *buf_out = floatbuff;

    if (frames_read == 0) {
        fprintf(stderr, "End of file or read error\n");
    }
}

void close_sndfile() {
    sf_close(sndfile);
    FREE_MAYBE(floatbuff);
}
