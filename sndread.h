/*
 *
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

#pragma once

#include <sndfile.h>

int open_sndfile(char *fname, size_t n, int *speed);
void close_sndfile(void);
void sndfile_read(float **buf_out, size_t *n_out);

extern SNDFILE *sndfile;  // Handle to the open audio file
extern SF_INFO sfinfo;    // Holds audio metadata (channels, samplerate, etc.)
