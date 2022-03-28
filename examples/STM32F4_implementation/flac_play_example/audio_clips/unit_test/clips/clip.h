#ifndef __CLIP_H__
#define __CLIP_H__

#include <string.h>
#include <stdio.h>

#include "siren_flac.h"

struct clip_handle {
	int size;
	int offset;
	const char *ptr;
};

struct clip_handle clip_siren = {
		.size = siren_flac_length,
		.offset = 0,
		.ptr = siren_flac,
};



#endif
