/* rcfile.c
 *
 * Copyright (C) 2001-2002 Claudio Girardi
 *           (C) 2006      Edouard Griffiths for modifications implementing
 *                         averaging mode
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
#include <string.h>
#include <stdlib.h>
#include <time.h> /* for time() and ctime() */
#include <glib-2.0/glib.h>

#include "util.h"
#include "rcfile.h"
#include "glfer.h"

extern opt_t opt;

/* rc filename; it is set after the first call to rc_file_parse() */
static char *rc_filename;

/* skip_equals, skips to the first non-whitespace character after equals sign
 */
static int skip_equals(char **buf) {
    int i;

    for (i = 0; i < 200; i++) {
        if (**buf == '=') {
            (*buf)++;
            while (**buf == ' ')
                (*buf)++;
            return (TRUE);
        }
        (*buf)++;
    }

    fprintf(stderr, "error in rcfile, expected equals\n");
    return (FALSE);
}

static int rc_file_parse(const char *filename) {
    FILE *fp;
    char buf[4096], *i;

    if (rc_filename)
        g_free(rc_filename);

    /* save the filename for when we write back the preferences */
    rc_filename = g_strdup(filename);

    fp = fopen(filename, "r");

    if (!fp) /* problems opening file */
        return -1;

    while (fgets(buf, 256, fp)) {
        i = &buf[0];
        /* skip spaces */
        while ((*i == ' ') || (*i == '\t'))
            i++;
        /* don't process empty lines */
        if ((*i == '#') || (*i == '\n'))
            continue;

        /* work out what the setting is */
        if (!strncasecmp(i, "mode", 4)) {
            if (skip_equals(&i)) {
                opt.mode = atoi(i);
            }
        } else if (!strncasecmp(i, "scale_type", 10)) {
            if (skip_equals(&i)) {
                opt.scale_type = atoi(i);
            }
        } else if (!strncasecmp(i, "data_block_size", 15)) {
            if (skip_equals(&i)) {
                opt.data_block_size = atoi(i);
            }
        } else if (!strncasecmp(i, "data_blocks_overlap", 19)) {
            if (skip_equals(&i)) {
                opt.data_blocks_overlap = atof(i);
            }
        } else if (!strncasecmp(i, "mtm_w", 5)) {
            if (skip_equals(&i)) {
                opt.mtm_w = atof(i);
            }
        } else if (!strncasecmp(i, "mtm_k", 5)) {
            if (skip_equals(&i)) {
                opt.mtm_k = atoi(i);
            }
        } else if (!strncasecmp(i, "hparma_t", 8)) {
            if (skip_equals(&i)) {
                opt.hparma_t = atof(i);
            }
        } else if (!strncasecmp(i, "hparma_p_e", 10)) {
            if (skip_equals(&i)) {
                opt.hparma_p_e = atof(i);
            }
        } else if (!strncasecmp(i, "window_type", 11)) {
            if (skip_equals(&i)) {
                opt.window_type = atoi(i);
            }
        } else if (!strncasecmp(i, "sample_rate", 11)) {
            if (skip_equals(&i)) {
                opt.sample_rate = atoi(i);
            }
        } else if (!strncasecmp(i, "offset_freq", 11)) {
            if (skip_equals(&i)) {
                opt.offset_freq = atof(i);
            }
        } else if (!strncasecmp(i, "dot_time", 8)) {
            if (skip_equals(&i)) {
                opt.dot_time = atof(i);
            }
        } else if (!strncasecmp(i, "beacon_mode", 11)) {
            if (skip_equals(&i)) {
                if (!strncasecmp(i, "ON", 2))
                    opt.beacon_mode = TRUE;
                else if (!strncasecmp(i, "OFF", 3))
                    opt.beacon_mode = FALSE;
                else
                    fprintf(stderr, "error in rcfile %s: %s\n", filename, buf);
            }
        } else if (!strncasecmp(i, "beacon_pause", 12)) {
            if (skip_equals(&i)) {
                opt.beacon_pause = atof(i);
            }
        } else if (!strncasecmp(i, "beacon_tx_pause", 15)) {
            if (skip_equals(&i)) {
                if (!strncasecmp(i, "ON", 2))
                    opt.beacon_tx_pause = TRUE;
                else if (!strncasecmp(i, "OFF", 3))
                    opt.beacon_tx_pause = FALSE;
                else
                    fprintf(stderr, "error in rcfile %s: %s\n", filename, buf);
            }
        } else if (!strncasecmp(i, "dfcw_gap_time", 13)) {
            if (skip_equals(&i)) {
                opt.dfcw_gap_time = atof(i);
            }
        } else if (!strncasecmp(i, "tx_mode", 7)) {
            if (skip_equals(&i)) {
                if (!strncasecmp(i, "QRSS", 4))
                    opt.tx_mode = QRSS;
                else if (!strncasecmp(i, "DFCW", 4))
                    opt.tx_mode = DFCW;
                else
                    fprintf(stderr, "error in rcfile %s: %s\n", filename, buf);
            }
        } else if (!strncasecmp(i, "dash_dot_ratio", 14)) {
            if (skip_equals(&i)) {
                opt.dash_dot_ratio = atof(i);
            }
        } else if (!strncasecmp(i, "ptt_delay", 9)) {
            if (skip_equals(&i)) {
                opt.ptt_delay = atof(i);
            }
        } else if (!strncasecmp(i, "sidetone_freq", 13)) {
            if (skip_equals(&i)) {
                opt.sidetone_freq = atof(i);
            }
        } else if (!strncasecmp(i, "sidetone", 8)) {
            if (skip_equals(&i)) {
                if (!strncasecmp(i, "ON", 2))
                    opt.sidetone = TRUE;
                else if (!strncasecmp(i, "OFF", 3))
                    opt.sidetone = FALSE;
                else
                    fprintf(stderr, "error in rcfile %s: %s\n", filename, buf);
            }
        } else if (!strncasecmp(i, "dfcw_dot_freq", 13)) {
            if (skip_equals(&i)) {
                opt.dfcw_dot_freq = atof(i);
            }
        } else if (!strncasecmp(i, "dfcw_dash_freq", 14)) {
            if (skip_equals(&i)) {
                opt.dfcw_dash_freq = atof(i);
            }
        } else if (!strncasecmp(i, "ctrl_device", 11)) {
            if (skip_equals(&i)) {
                char *iend;
                /* Get the ending position.  */
                for (iend = i; *iend && *iend != '\n'; iend++)
                    ;
                opt.ctrl_device = (char *)malloc(iend - i + 1);
                memcpy(opt.ctrl_device, i, iend - i);
                opt.ctrl_device[iend - i] = '\0';
            }
        } else if (!strncasecmp(i, "device_type", 11)) {
            if (skip_equals(&i)) {
                if (!strncasecmp(i, "DEV_SERIAL", 10))
                    opt.device_type = DEV_SERIAL;
                else if (!strncasecmp(i, "DEV_PARALLEL", 12))
                    opt.device_type = DEV_PARALLEL;
                else
                    fprintf(stderr, "error in rcfile %s: %s\n", filename, buf);
            }
        } else if (!strncasecmp(i, "audio_device", 11)) {
            if (skip_equals(&i)) {
                char *iend;
                /* Get the ending position.  */
                for (iend = i; *iend && *iend != '\n'; iend++)
                    ;
                opt.audio_device = (char *)malloc(iend - i + 1);
                memcpy(opt.audio_device, i, iend - i);
                opt.audio_device[iend - i] = '\0';
            }
        } else if (!strncasecmp(i, "thr_level", 5)) {
            if (skip_equals(&i)) {
                opt.thr_level = atof(i);
            }
        } else if (!strncasecmp(i, "autoscale", 9)) {
            if (skip_equals(&i)) {
                opt.autoscale = atoi(i);
            }
        } else if (!strncasecmp(i, "max_level_db", 12)) {
            if (skip_equals(&i)) {
                opt.max_level_db = atof(i);
            }
        } else if (!strncasecmp(i, "min_level_db", 12)) {
            if (skip_equals(&i)) {
                opt.min_level_db = atof(i);
            }
        } else if (!strncasecmp(i, "palette", 7)) {
            if (skip_equals(&i)) {
                opt.palette = atoi(i);
            }
        } else if (!strncasecmp(i, "avg_mode", 8)) {
            if (skip_equals(&i)) {
                opt.averaging = atoi(i);
            }
        } else if (!strncasecmp(i, "avg_nsamples", 12)) {
            if (skip_equals(&i)) {
                opt.avgsamples = atoi(i);
            }
        } else if (!strncasecmp(i, "avg_min_avgband", 15)) {
            if (skip_equals(&i)) {
                opt.min_avgband = atof(i);
            }
        } else if (!strncasecmp(i, "avg_max_avgband", 15)) {
            if (skip_equals(&i)) {
                opt.max_avgband = atof(i);
            }

        } else
            fprintf(stderr, "error in rcfile %s: %s\n", filename, buf);
    }

    fclose(fp);
    /* everything went ok */
    return 0;
}

/* parse rcfile */
int parse_rcfile(void) {
    const gchar *confname;
    int result;

    confname = g_build_filename(g_get_user_config_dir(), PACKAGE_NAME ".conf", NULL);
    result = rc_file_parse(confname);
    return result;
}

/* rc_file_write, writes the user's preferences back to file */
int rc_file_write(void) {
    FILE *fp;
    time_t date;

    time(&date);

    GString *content = g_string_new(NULL);
    /* write header */
    g_string_append_printf(content, "# This is the startup file for " PACKAGE_STRING "\n");
    g_string_append_printf(content, "# automatically generated on %s\n", ctime(&date));
    g_string_append_printf(content, "# Lines starting with \'#\' are ignored\n\n");
    g_string_append_printf(content, "mode = %i\n", opt.mode);
    g_string_append_printf(content, "scale_type = %i\n", opt.scale_type);

    g_string_append_printf(content, "data_block_size = %zu\n", opt.data_block_size);
    g_string_append_printf(content, "data_blocks_overlap = %f\n", opt.data_blocks_overlap);

    g_string_append_printf(content, "mtm_w = %f\n", opt.mtm_w);
    g_string_append_printf(content, "mtm_k = %i\n", opt.mtm_k);

    g_string_append_printf(content, "hparma_t = %i\n", opt.hparma_t);
    g_string_append_printf(content, "hparma_p_e = %i\n", opt.hparma_p_e);

    g_string_append_printf(content, "window_type = %i\n", opt.window_type);
    g_string_append_printf(content, "sample_rate = %i\n", opt.sample_rate);
    g_string_append_printf(content, "offset_freq = %f\n", opt.offset_freq);
    g_string_append_printf(content, "dot_time = %f\n", opt.dot_time);
    g_string_append_printf(content, "beacon_mode = %s\n", opt.beacon_mode == TRUE ? "ON" : "OFF");
    g_string_append_printf(content, "beacon_pause = %f\n", opt.beacon_pause);
    g_string_append_printf(content, "beacon_tx_pause = %s\n", opt.beacon_tx_pause == TRUE ? "ON" : "OFF");

    g_string_append_printf(content, "dfcw_gap_time = %f\n", opt.dfcw_gap_time);
    g_string_append_printf(content, "tx_mode = %s\n", opt.tx_mode == QRSS ? "QRSS" : "DFCW");
    g_string_append_printf(content, "dash_dot_ratio = %f\n", opt.dash_dot_ratio);
    g_string_append_printf(content, "ptt_delay = %f\n", opt.ptt_delay);
    g_string_append_printf(content, "sidetone_freq = %f\n", opt.sidetone_freq);
    g_string_append_printf(content, "sidetone = %s\n", opt.sidetone == TRUE ? "ON" : "OFF");
    g_string_append_printf(content, "dfcw_dot_freq = %f\n", opt.dfcw_dot_freq);
    g_string_append_printf(content, "dfcw_dash_freq = %f\n", opt.dfcw_dash_freq);
    g_string_append_printf(content, "ctrl_device = %s\n", opt.ctrl_device);
    g_string_append_printf(content, "device_type = %s\n",
                           opt.device_type == DEV_SERIAL ? "DEV_SERIAL" : "DEV_PARALLEL");
    g_string_append_printf(content, "audio_device = %s\n", opt.audio_device);
    g_string_append_printf(content, "thr_level = %f\n", opt.thr_level);
    g_string_append_printf(content, "autoscale = %i\n", opt.autoscale);
    g_string_append_printf(content, "max_level_db = %f\n", opt.max_level_db);
    g_string_append_printf(content, "min_level_db = %f\n", opt.min_level_db);
    g_string_append_printf(content, "palette = %i\n", opt.palette);

    g_string_append_printf(content, "avg_mode = %i\n", opt.averaging);
    g_string_append_printf(content, "avg_nsamples = %i\n", opt.avgsamples);
    g_string_append_printf(content, "avg_min_avgband = %f\n", opt.min_avgband);
    g_string_append_printf(content, "avg_max_avgband = %f\n", opt.max_avgband);

    GError *error = NULL;
    if (g_file_set_contents(rc_filename, content->str, -1, &error)) {
        g_print("Config file written successfully: %s\n", rc_filename);
    } else {
        g_printerr("Error writing config file: %s\n", error->message);
        g_clear_error(&error);
    }
    g_string_free(content, TRUE);

    return (0);
}

void rc_file_done(void) {
    g_free(rc_filename);
}
