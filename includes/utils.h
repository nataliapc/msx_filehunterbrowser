/*
	Copyright (c) 2024 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>


void die(const char *s, ...);

uint8_t getRomByte(uint16_t address) __sdcccall(1);

void dzx0_standard(void *src, void *dst) __sdcccall(1);

uint16_t loadFile(char *filename, void *destaddress, uint16_t size);
uint16_t loadFullFile(char *filename, void *destaddress);


char* formatSize(char *dst, uint32_t size);
void memncpy(char *dst, char *src, char c, uint16_t size);
void fillBlink(uint8_t x, uint8_t y, uint8_t lines, uint8_t len, bool enabled);
void putstrxy(uint8_t x, uint8_t y, char *str);
uint8_t scanf(char *str, uint16_t maxLen);
char* strReplaceChar(char *str, char find, char replace);


#define MODE_ANK		0
#define MODE_KANJI0		1
#define MODE_KANJI1		2
#define MODE_KANJI2		3
#define MODE_KANJI3		4
bool detectKanjiDriver() __z88dk_fastcall;
char getKanjiMode() __sdcccall(1);
void setKanjiMode(uint8_t mode) __z88dk_fastcall;


#endif//__UTILS_H__
