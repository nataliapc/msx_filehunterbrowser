/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdint.h>
#include "heap.h"
#include "msx_const.h"


void *malloc(uint16_t size) {
	if ((uint16_t)heap_top + size >= varTPALIMIT) return 0x0000;
	uint8_t *ret = heap_top;
	heap_top += size;
	return (void*)ret;
}

void free(uint16_t size) {
	heap_top -= size;
}
