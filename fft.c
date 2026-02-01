/* fft.c

 * Copyright (C) 2001-2005 Claudio Girardi
 *
 * This file is derived from xspectrum, Copyright (C) 2000 Vincent Arkesteijn
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <rfftw.h>

#include "glfer.h"
#include "fft.h"
#include "util.h"

extern opt_t opt;
extern glfer_t glfer;

fft_window_t fft_windows[] = {
    {    "/Hanning",     HANNING_WINDOW},
    {   "/Blackman",    BLACKMAN_WINDOW},
    {   "/Gaussian",    GAUSSIAN_WINDOW},
    {      "/Welch",       WELCH_WINDOW},
    {   "/Bartlett",    BARTLETT_WINDOW},
    {"/Rectangular", RECTANGULAR_WINDOW},
    {    "/Hamming",     HAMMING_WINDOW},
    {     "/Kaiser",      KAISER_WINDOW}
};

size_t num_fft_windows = sizeof(fft_windows) / sizeof(fft_windows[0]);

void compute_window(fft_params_t *params);

void prepare_audio(float *audio_buf, fft_params_t *params) {
    size_t i;
    size_t N = params->n;
    size_t n_eff = N * (1.0 - params->overlap);
    size_t n_overlap = N - n_eff;
    float sig_mean;
    float inp_val;
    float ftmp;
    int submean = params->sub_mean;

#if 0
  /* test signal */
  for (i = 0; i < N; i++) {
    /* sine wave */
    audio_buf[i] = sin(2 * M_PI * i / 9 );
  }
#endif

    /* subtract mean value */
    if (submean)  // EG made conditional to "autoscale" mode compatible with versions <0.3.5. CG ??
    {
        sig_mean = 0.0;
        for (i = 0; i < n_eff; i++) {
            sig_mean += audio_buf[i];
        }
        sig_mean /= n_eff;
        for (i = 0; i < n_eff; i++) {
            audio_buf[i] -= sig_mean;
        }
    }

    /* copy old samples into FFT buffer */
    if (glfer.first_buffer == FALSE) {
        for (i = 0; i < n_overlap; i++) {
            params->inbuf_audio[i] = params->inbuf_audio[N - n_overlap + i];
        }
    } else {
        /* delete old data from buffer */
        for (i = 0; i < n_overlap; i++) {
            params->inbuf_audio[i] = 0.0;
        }
    }

    /* acquire new samples */
    for (i = 0; i < n_eff; i++) {
        params->inbuf_audio[i + n_overlap] = audio_buf[i];
    }

    /* experiment: signal envelope
    sig_mean = 0.0;
    for (i = 0; i < N; i++) {
      params->inbuf_audio[i] = fabs(params->inbuf_audio[i]);
      sig_mean += params->inbuf_audio[i];
    }
    sig_mean /= N;
    for (i = 0; i < N; i++) {
      params->inbuf_audio[i] -= sig_mean;
    }
    */

    if (params->a > 0.0) {
        /* apply non-linear processing to the input signal */
        for (i = 0; i < N; i++) {
            inp_val = params->inbuf_audio[i];
            params->inbuf_fft[i] = inp_val / (params->a + inp_val * inp_val);
            if (params->window_type != RECTANGULAR_WINDOW) {
                /* window the entire buffer */
                params->inbuf_fft[i] *= params->window[i];
            }
        }
    } else {
        /* conventional processing */
        if (params->window_type != RECTANGULAR_WINDOW) {
            /* window the entire buffer */
            for (i = 0; i < N; i++) {
                // printf("%f\n", audio_buf[i]);
                params->inbuf_fft[i] = params->window[i] * params->inbuf_audio[i];
                // params->inbuf_fft[i] = 5 * sin(2 * M_PI * i * 576.0 / 8000.0);
            }
        } else
            for (i = 0; i < N; i++)
                params->inbuf_fft[i] = params->inbuf_audio[i];
    }

    if (params->limiter == 1) {
        for (i = 0; i < N; i++) {
            ftmp = log(fabs(params->inbuf_fft[i]));
            params->inbuf_fft[i] = (params->inbuf_fft[i] > 0 ? exp(ftmp * 0.1) : -exp(ftmp * 0.1));
        }
    }

#if 0
  /* test signal */
  for (i = 0; i < N; i++) {
    /* sine wave at 1/8 of the sampling rate, will have no leakage */
    params->inbuf_audio[i] = sin(2 * M_PI * i / 8 );
  }
#endif
}

void fft_init(fft_params_t *params) {
    params->plan = rfftw_create_plan(params->n, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);

    params->inbuf_audio = calloc(params->n, sizeof(fftw_real));
    params->inbuf_fft = calloc(params->n, sizeof(fftw_real));
    params->outbuf = calloc(params->n, sizeof(fftw_real));
    D(fprintf(stderr, "fft_init (FFTW): n = %zu\n", params->n));
    D(fprintf(stderr, "overlap = %f\n", params->overlap));
    params->window = malloc(params->n * sizeof(float));
    compute_window(params);
    params->sub_mean = opt.autoscale;  // sub_mean only if autoscale (?)
}

void fft_do(float *audio_buf, fft_params_t *params) {
    /* apply overlap, limting, windowing, etc. according to active options */
    prepare_audio(audio_buf, params);

    rfftw_one(params->plan, params->inbuf_fft, params->outbuf);
}

void fft_psd(float *psd_buf, float *phase_buf, fft_params_t *params) {
    int i;
    int N = params->n;

    if (psd_buf) {
        /* compute the power spectrum */
        /* almost verbatim from the fftw tutorial: */
        /* division by N is for normalizing the fft */
        psd_buf[0] = params->outbuf[0] * params->outbuf[0] / N; /* DC component */
        for (i = 1; i < (N + 1) / 2; i++)                       /* i < N/2 rounded up */
            psd_buf[i] = (params->outbuf[i] * params->outbuf[i] + params->outbuf[N - i] * params->outbuf[N - i]) / N;
        if (N % 2 == 0)                                                         /* N is even */
            psd_buf[N / 2] = params->outbuf[N / 2] * params->outbuf[N / 2] / N; /* Nyquist freq */
    }
    if (phase_buf) {
        /* compute the phase spectrum */
        phase_buf[0] = 0;
        for (i = 1; i < (N + 1) / 2; i++)
            phase_buf[i] = atan2(params->outbuf[i], params->outbuf[N - i]);
        if (N % 2 == 0)
            phase_buf[N / 2] = 0;
    }
}

/* Comparator MUST return -1, 0, 1 for qsort stability and performance. */
static int cmp_float(const void *num1_p, const void *num2_p) {
    float n1 = *(float *)num1_p;
    float n2 = *(float *)num2_p;

    /* Sort descending (larger values first) */
    if (n1 < n2) return 1;
    if (n1 > n2) return -1;
    return 0;
}

void compute_floor(float *psd_buf, size_t n, float *sig_pwr_p, float *floor_pwr_p, float *peak_pwr_p,
                   unsigned int *peak_bin_p) {
    size_t i;
    // int N2 = (n + 1) / 2; /* N/2 rounded up */
    size_t N2 = n;
    float *tmp_buf = NULL;
    float sig_pwr = 0.0, floor_pwr = 0.0;

    /* Compute Peak Power and Bin (Linear scan, fast) */
    *peak_pwr_p = 0.0;
    *peak_bin_p = 0;
    for (i = 0; i < N2; i++) {
        if (psd_buf[i] > *peak_pwr_p) {
            *peak_pwr_p = psd_buf[i];
            *peak_bin_p = i;
        }
        sig_pwr += psd_buf[i]; /* Sum for mean calculation */
    }

    /* total mean power in one bin, i.e. mean spectral density */
    *sig_pwr_p = sig_pwr / N2;

    /* Sorting is O(N log N) and very expensive on file reads.
       Only do it if autoscale is enabled. If disabled, floor_pwr is not strictly
       required for display leveling (fixed levels used), so we use mean power as a fallback. */
    if (opt.autoscale) {
        if ((N2 > (SIZE_MAX / sizeof(float))) || (NULL == (tmp_buf = malloc(N2 * sizeof(float))))) {
            fprintf(stderr, "compute_floor: error allocating tmp_buf, size %zu\n", N2);
            exit(EXIT_FAILURE);
        }
        /* copy psd_buf to working area */
        memcpy(tmp_buf, psd_buf, N2 * sizeof(float));

        /* sort the power estimates in each bin in descending order */
        qsort(tmp_buf, N2, sizeof(float), cmp_float);

        /* compute noise floor power from the last 5% */
        for (i = N2 * 0.95; i < N2; i++)
            floor_pwr += tmp_buf[i];

        /* total noise power, using estimate from the last 5% */
        floor_pwr /= 0.05;
        /* noise mean power in one bin, i.e. spectral density */
        floor_pwr /= N2;

        *floor_pwr_p = floor_pwr;
        free(tmp_buf);
    } else {
        /* Fallback when autoscale is off: just use mean power as floor
           to avoid 0.0 in display stats */
        *floor_pwr_p = *sig_pwr_p;
    }
}

void fft_close(fft_params_t *params) {
    FREE_MAYBE(params->inbuf_audio);
    FREE_MAYBE(params->inbuf_fft);
    rfftw_destroy_plan(params->plan);
    FREE_MAYBE(params->outbuf);
    FREE_MAYBE(params->window);
}

void compute_window(fft_params_t *params) {
    size_t i;
    float t;
    float alpha = 1.0;
    float w_pwr;
    size_t N = params->n;
    int window_type = params->window_type;

    /* Calculate FFT Windowing function */
    for (i = 0; i < N; i++) {
        switch (window_type) {
        case HANNING_WINDOW: /* Hanning */
            params->window[i] = 0.5 - 0.5 * cos(2.0 * M_PI * i / (N - 1.0));
            break;
        case BLACKMAN_WINDOW: /* Blackman */
            params->window[i] = 0.42 - 0.5 * cos(2.0 * M_PI * i / (N - 1.0)) + 0.08 * cos(4.0 * M_PI * i / (N - 1.0));
            break;
        case GAUSSIAN_WINDOW: /* Gaussian */
            alpha = 1.0;
            params->window[i] = exp(-alpha * (2.0 * i - N + 1.0) * (2.0 * i - N + 1.0) / ((N - 1.0) * (N - 1.0)));
            break;
        case WELCH_WINDOW: /* Welch */
            params->window[i] = 1.0 - ((2.0 * i - N + 1.0) / (N - 1.0)) * ((2.0 * i - N + 1.0) / (N - 1.0));
            break;
        case BARTLETT_WINDOW: /* Parzen or Bartlett (?) */
            params->window[i] = 1.0 - fabs((2.0 * i - N + 1.0) / (N - 1.0));
            break;
        case RECTANGULAR_WINDOW: /* Rectangular */
            params->window[i] = 1.0;
            break;
        case HAMMING_WINDOW: /* Hamming */
            params->window[i] = 0.54 - 0.46 * cos(2.0 * M_PI * i / (N - 1.0));
            break;
        case KAISER_WINDOW: /* Kaiser */
            t = (N - 1.0) / 2.0;
            alpha = 6.0 / t; /* from 4.0 / t to 9.0 / t */
            params->window[i] = bessel_I0(alpha * sqrt(t * t - (i - t) * (i - t))) / bessel_I0(alpha * t);
            break;
        default:
            params->window[i] = 1.0;
        }
    }
    /* normalize window to preserve power */
    w_pwr = 0.0;
    for (i = 0; i < N; i++) {
        w_pwr += params->window[i] * params->window[i];
    }
    for (i = 0; i < N; i++) {
        params->window[i] /= sqrt(w_pwr);
    }
}

void test_wind(void) {
    /* save the window function. (for the curious) */
    int i, j;
    FILE *fp;
    fft_params_t par;

    if ((fp = fopen("windfile", "wx")) == NULL) {
        printf("Error opening window output file\n");
        puts("Press <ENTER> to continue...");
        getchar();
    } else {
        for (i = 0; i < par.n; i++) {
            for (j = 1; j <= 8; j++) {
                par.window_type = j;
                compute_window(&par);
                fprintf(fp, "%f ", par.window[i]);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
}

void goertzel(float *audio_buf, float *psd_buf, float *phase_buf, fft_params_t *params) {
    size_t i, r;
    double yt1, yt2, yt3;
    float jason_f_low = 800;
    float jason_f_high = 812;
    float sample_rate = 8000;
    double cos_fac;
    size_t N = params->n;
    size_t i_low = N * jason_f_low / sample_rate;
    size_t i_high = N * jason_f_high / sample_rate;

    for (i = 0; i < N; i++) {
        params->inbuf_audio[i] = params->window[i] * audio_buf[i];
        // params->inbuf[i] = (i == 0 ? 1.0 : 0.0);
    }

    for (i = i_low; i < i_high; i++) {
        yt1 = yt2 = yt3 = 0.0;
        cos_fac = 2.0 * cos(2.0 * M_PI * i / N);
        for (r = 0; r < N; r++) {
            yt3 = yt2;
            yt2 = yt1;
            yt1 = params->inbuf_audio[r] + yt2 * cos_fac - yt3;
        }
        params->outbuf[i] = yt1 - yt2 * cos(2.0 * M_PI * i / N); /* real part */
        params->outbuf[N - i] = -yt2 * sin(2.0 * M_PI * i / N);  /* imag. part */
                                                                 /* sign of imag. part is wrong ? */
    }

    /*
      for (i = i_low; i < i_high; i++) {
        printf("G %i  %f %f\n", i, params->outbuf[i], params->outbuf[N - i]);
      }


      printf("\n");
      rfftw_one(params->plan, params->inbuf, params->outbuf);

      for (i = i_low; i < i_high; i++) {
        printf("F %i  %f %f\n", i, params->outbuf[i], params->outbuf[N - i]);
      }
    */

    if (psd_buf) {
        /* compute the power spectrum */
        /* almost verbatim from the fftw tutorial: */
        /* division by N^2 is for normalizing the fft */
        psd_buf[0] = params->outbuf[0] * params->outbuf[0] / (N * N); /* DC component */
        for (i = 1; i < (N + 1) / 2; i++)                             /* (k < N/2 rounded up) */
            psd_buf[i] =
                (params->outbuf[i] * params->outbuf[i] + params->outbuf[N - i] * params->outbuf[N - i]) / (N * N);
        if (N % 2 == 0)                                                               /* N is even */
            psd_buf[N / 2] = params->outbuf[N / 2] * params->outbuf[N / 2] / (N * N); /* Nyquist freq-> */
    }
    if (phase_buf) {
        /* compute the phase spectrum */
        phase_buf[0] = 0;
        for (i = 1; i < (N + 1) / 2; i++)
            phase_buf[i] = atan2(params->outbuf[i], params->outbuf[N - i]);
        if (N % 2 == 0)
            phase_buf[N / 2] = 0;
    }
}
