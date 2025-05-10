/*
unapinet_common.c
    Utilidades comunes y datos internos para biblioteca UNAPI

Copyright (c) 2025

Este archivo es software libre bajo los términos de la licencia LGPL 2.1
*/

#include <stdlib.h>
#include <string.h>
#include "unapi_net.h"
#include "asm.h"

/* Estructura para el bloque de código UNAPI */
typedef struct {
	uint8_t UnapiCallCode[11];	/* Código para llamar al punto de entrada UNAPI */
	uint8_t UnapiReadCode[13];	/* Código para leer un byte desde la implementación */
} unapi_code_block_t;

/* Variables globales internas */
static unapi_code_block_t s_codeBlock;      /* Bloque de código UNAPI para llamadas */
static Z80_registers s_regs;                /* Registros Z80 para llamadas UNAPI */
static uint8_t s_initialized = 0;           /* Estado de inicialización de la biblioteca */
static uint8_t s_implementationCount = 0;   /* Número de implementaciones disponibles */

/* 
 * Ejecuta una llamada a función UNAPI con los registros proporcionados
 */
void unapinet_call(uint8_t function, Z80_registers* regs)
{
	regs->Bytes.A = function;
	UnapiCall(&s_codeBlock, function, regs, REGS_MAIN, REGS_MAIN);
}

/*
 * Lee un byte de la memoria de la implementación UNAPI
 */
uint8_t unapinet_read(uint16_t address)
{
	return UnapiRead(&s_codeBlock, address);
}

/*
 * Obtiene el número de implementaciones UNAPI disponibles para el identificador dado
 */
uint8_t unapinet_get_implementation_count(const char* identifier)
{
	return UnapiGetCount(identifier);
}

/*
 * Construye un bloque de código para comunicarse con una implementación UNAPI específica
 */
void unapinet_build_code_block(const char* identifier, uint8_t implementation_index)
{
	UnapiBuildCodeBlock(identifier, implementation_index, &s_codeBlock);
}

/*
 * Obtiene la referencia al bloque de código UNAPI interno
 */
void* unapinet_get_code_block()
{
	return &s_codeBlock;
}

/*
 * Obtiene la referencia a la estructura de registros Z80 interna
 */
Z80_registers* unapinet_get_registers()
{
	return &s_regs;
}

/*
 * Verifica si la biblioteca está inicializada
 */
uint8_t unapinet_is_initialized()
{
	return s_initialized;
}

/*
 * Establece el estado de inicialización
 */
void unapinet_set_initialized(uint8_t state)
{
	s_initialized = state;
}

/*
 * Establece el número de implementaciones detectadas
 */
void unapinet_set_implementation_count(uint8_t count)
{
	s_implementationCount = count;
}

/*
 * Obtiene el número de implementaciones detectadas
 */
uint8_t unapinet_get_implementation_count_cached()
{
	return s_implementationCount;
}

/*
 * Verifica que la biblioteca esté inicializada, devuelve error si no lo está
 */
uint8_t unapinet_check_initialized()
{
	if (!s_initialized) {
		return UNAPI_ERR_NOT_IMP;
	}
	return UNAPI_ERR_OK;
}