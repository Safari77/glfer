/* audio.c
 *
 * Copyright (C) 2001-2007 Claudio Girardi
 * Refactored for PipeWire/PulseAudio by Sami Farin (C) 2026
 *
 * This file is derived from xspectrum, Copyright (C) 2000 Vincent Arkesteijn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "audio.h"
#include "util.h"

/* Default to NULL so PipeWire uses the system default source if not specified */
#define DEFAULT_DEV_NAME    NULL
#define DEFAULT_SAMPLE_RATE 48000

static pa_simple *s_handle = NULL;
static int out_len;
static float *buff_f = NULL;
static int old_p = 0, old_len = 0;
static int current_sample_rate;

int audio_init(char *device, int *sample_rate, int len) {
    out_len = len;

    // If device is an empty string, set it to NULL for PulseAudio default
    if (device && strlen(device) == 0) {
        device = NULL;
    }

    if (!*sample_rate)
        *sample_rate = DEFAULT_SAMPLE_RATE;
    current_sample_rate = *sample_rate;

    /* Define the stream format: 16-bit Mono */
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16NE,
        .rate = 48000,
        .channels = 1
    };

    pa_sample_spec custom_ss = ss;
    custom_ss.rate = *sample_rate;

    int error;
    /* device here can be the PipeWire/Pulse source name, or NULL for default */
    s_handle = pa_simple_new(NULL,               // Use default server
                             "glfer",            // Application name
                             PA_STREAM_RECORD,
                             device,             // Device name (e.g. NULL or pulse source string)
                             "Spectrum Analysis",// Stream description
                             &custom_ss,         // Sample format
                             NULL,               // Use default channel map
                             NULL,               // Use default buffering attributes
                             &error);

    if (!s_handle) {
        fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(error));
        return -1;
    }

    /* Reset buffers */
    old_p = 0;
    old_len = 0;

    return 1; // Return a dummy positive FD
}

void audio_read(float **buf_out, size_t *n_out) {
    int error;
    /* We read in blocks of out_len to satisfy the app's buffer logic */
    short int buf16[out_len];

    if (!buff_f) {
        buff_f = (float *)calloc(out_len * 2, sizeof(float));
    }

    /* Move old data */
    for (int i = 0; i < old_len; i++)
        buff_f[i] = buff_f[old_p + i];
    old_p = 0;

    /* Read from PipeWire */
    if (pa_simple_read(s_handle, buf16, sizeof(buf16), &error) < 0) {
        fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(error));
        return;
    }

    /* Convert 16-bit PCM to Float */
    for (int i = 0; i < out_len; i++) {
        buff_f[i + old_len] = (float)buf16[i] / 32768.0f;
    }

    int total_samples = out_len + old_len;
    *buf_out = buff_f;
    *n_out = total_samples / out_len;

    old_p = (*n_out) * out_len;
    old_len = total_samples % out_len;
}

void audio_close(void) {
    if (s_handle) {
        pa_simple_free(s_handle);
        s_handle = NULL;
    }
    FREE_MAYBE(buff_f);
}
