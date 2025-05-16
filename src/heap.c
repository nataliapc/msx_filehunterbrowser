/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#include <stdint.h>
#include "heap.h"
#include "msx_const.h"

#define PUSHSTACK_SIZE 2

static void *_pushStack[PUSHSTACK_SIZE];
static const uint8_t _pushIndex = 0;


void *malloc(uint16_t size) {
	if ((uint16_t)heap_top + size >= varTPALIMIT) return 0x0000;
	char *ret = heap_top;
	heap_top += size;
	return (void*)ret;
}

void free(uint16_t size) {
	heap_top -= size;
}

void *heapPush()
{
	if (_pushIndex < PUSHSTACK_SIZE) {
		_pushStack[_pushIndex] = heap_top;
		(*((uint8_t*)&_pushIndex))++;
		return (void*)heap_top;
	} else {
		return (void*)0;
	}
}

void *heapPop()
{
	char *ret = heap_top;

	if (_pushIndex > 0) {
		(*((uint8_t*)&_pushIndex))--;
		heap_top = _pushStack[_pushIndex];
		return ret;
	} else {
		return (void*)0;
	}
}