#include "conio.h"


#define ADR_BLINK 0x800

extern text_info _current_text_info;


void _applyColors();
void _setRegVDP(uint16_t portValue) __sdcccall(1);
void _fillVRAM(uint16_t vram, uint16_t len, uint8_t value) __sdcccall(0);
uint8_t getByteVRAM(uint16_t vram) __sdcccall(1);
void setByteVRAM(uint16_t vram, uint8_t value) __sdcccall(0);
void _copyVRAMtoRAM(uint16_t vram, uint16_t memory, uint16_t size) __sdcccall(0);
void _copyRAMtoVRAM(uint16_t memory, uint16_t vram, uint16_t size) __sdcccall(0);
