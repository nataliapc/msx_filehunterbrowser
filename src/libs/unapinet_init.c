/*
unapinet_init.c
    Funciones de inicialización y terminación para biblioteca UNAPI

Copyright (c) 2025

Este archivo es software libre bajo los términos de la licencia LGPL 2.1
*/

#include <stdlib.h>
#include <string.h>
#include "../../includes/unapi_net.h"

/* Declaraciones de funciones internas */
extern uint8_t unapinet_get_implementation_count(const char* identifier);
extern void unapinet_build_code_block(const char* identifier, uint8_t implementation_index);
extern void unapinet_set_initialized(uint8_t state);
extern void unapinet_set_implementation_count(uint8_t count);
extern uint8_t unapinet_is_initialized(void);

/* Identificador para implementaciones TCP/IP UNAPI */
#define TCP_IP_IMPL_ID "TCP/IP"

/**
 * Inicializa la biblioteca y busca implementaciones UNAPI de red disponibles
 * 
 * @return 1 si se encontró al menos una implementación, 0 si no
 */
uint8_t unapinet_init(void) __sdcccall(0)
{
	uint8_t count;

	/* Evitar inicialización múltiple */
	if (unapinet_is_initialized()) {
		return 1;
	}

	/* Buscar implementaciones TCP/IP UNAPI disponibles */
	count = unapinet_get_implementation_count(TCP_IP_IMPL_ID);
	if (count == 0) {
		return 0;
	}

	/* Seleccionamos la primera implementación */
	unapinet_build_code_block(TCP_IP_IMPL_ID, 1);
	
	/* Guardamos el número de implementaciones y marcamos como inicializado */
	unapinet_set_implementation_count(count);
	unapinet_set_initialized(1);
	
	return 1;
}

/**
 * Libera los recursos utilizados por la biblioteca
 */
void unapinet_term(void) __sdcccall(0)
{
	/* Marcamos la biblioteca como no inicializada */
	unapinet_set_initialized(0);
}