/*
unapinet_dns.c
    Implementación de funciones para resolución DNS para UNAPI

Copyright (c) 2025

Este archivo es software libre bajo los términos de la licencia LGPL 2.1
*/

#include <string.h>
#include <stdlib.h>
#include "../../includes/unapi_net.h"

/* Declaraciones de funciones internas */
extern void unapinet_call(uint8_t function, Z80_registers* regs);
extern Z80_registers* unapinet_get_registers(void);
extern uint8_t unapinet_check_initialized(void);

/**
 * Resuelve un nombre de host a una dirección IP
 * 
 * @param hostname Nombre de host a resolver
 * @param address Puntero donde guardar la dirección IP
 * @return Código de error UNAPI
 */
uint8_t unapinet_dns_resolve(const char* hostname, unapiIpAddress_t* address) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;
	uint16_t retryCount = 0;
	static const uint16_t MAX_RETRIES = 100; /* Tiempo de espera máximo aproximado */

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	if (!hostname || !address) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();

	/* Verificar si es una dirección IP (usa TCPIP_DNS_Q_NEW) */
	if (hostname[0] >= '0' && hostname[0] <= '9') {
		regs->Words.HL = (uint16_t)hostname;
		/* Flag = 1 indica que esperamos que sea una IP */
		regs->Bytes.B = 1;
		unapinet_call(TCPIP_DNS_Q_NEW, regs);
	} else {
		/* Usar DNS_Q_NEW, flag = 0 indica que es un nombre a resolver */
		regs->Words.HL = (uint16_t)hostname;
		regs->Bytes.B = 0;
		unapinet_call(TCPIP_DNS_Q_NEW, regs);
		
		/* Si no se reconoce DNS_Q_NEW, intentamos con DNS_Q (versiones anteriores) */
		if (regs->Bytes.A == UNAPI_ERR_NOT_IMP) {
			regs->Words.HL = (uint16_t)hostname;
			unapinet_call(TCPIP_DNS_Q, regs);
		}
	}

	/* Verificar si hubo error */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		return regs->Bytes.A;
	}

	/* Para implementaciones asíncronas (debería ser bloqueante según la especificación, 
	   pero algunas implementaciones pueden ser asíncronas) */
	/* Esperar a que la consulta DNS termine */
	while (retryCount < MAX_RETRIES) {
		/* Llamar a DNS_S para verificar estado */
		regs->Bytes.B = 0;
		unapinet_call(TCPIP_DNS_S, regs);

		/* Si no está en progreso, salimos */
		if (regs->Bytes.A != UNAPI_ERR_OK || regs->Bytes.B != 1) {
			break;
		}

		/* Esperar un poco y verificar eventos */
		unapinet_process_events();
		retryCount++;

		/* Delay simple para dar tiempo al procesador */
		__asm
			push bc
			ld bc, #1000
		1$:
			dec bc
			ld a, b
			or c
			jr nz, 1$
			pop bc
		__endasm;
	}

	/* Si hay error o tiempo de espera excedido */
	if (regs->Bytes.A != UNAPI_ERR_OK || regs->Bytes.B != 0) {
		if (retryCount >= MAX_RETRIES) {
			return UNAPI_ERR_DNS;
		}
		return regs->Bytes.A;
	}

	/* Copiar la IP resultante */
	address->ip[0] = regs->Bytes.L;
	address->ip[1] = regs->Bytes.H;
	address->ip[2] = regs->Bytes.E;
	address->ip[3] = regs->Bytes.D;

	return UNAPI_ERR_OK;
}

/**
 * Convierte una cadena con formato IP "A.B.C.D" a estructura unapiIpAddress_t
 * 
 * @param ipString Cadena con la dirección IP
 * @param address Puntero a estructura donde guardar la dirección
 * @return 1 si la conversión fue correcta, 0 si la cadena no es una IP válida
 */
uint8_t unapinet_string_to_ip(const char* ipString, unapiIpAddress_t* address) __sdcccall(0)
{
	uint8_t i, j, val;
	uint8_t dots = 0;
	char c;
	
	if (!ipString || !address) {
		return 0;
	}
	
	/* Inicializar a cero */
	for (i = 0; i < 4; i++) {
		address->ip[i] = 0;
	}
	
	/* Parsear una dirección IP */
	i = 0; /* Posición en la cadena */
	j = 0; /* Componente de la IP (0-3) */
	val = 0;
	
	while (j < 4) {
		c = ipString[i++];
		
		if (c >= '0' && c <= '9') {
			/* Acumular dígito */
			val = (val * 10) + (c - '0');
			if (val > 255) {
				return 0; /* Valor fuera de rango */
			}
		} else if (c == '.' || c == '\0') {
			/* Fin de un componente o de la cadena */
			address->ip[j++] = val;
			val = 0;
			
			if (c == '.') {
				dots++;
				if (dots > 3) {
					return 0; /* Demasiados puntos */
				}
			}
			
			if (c == '\0') {
				break;
			}
		} else {
			return 0; /* Carácter no válido */
		}
	}
	
	/* Validar que haya exactamente 4 componentes y 3 puntos */
	if (j == 4 && dots == 3) {
		return 1;
	}
	
	return 0;
}

/**
 * Convierte una estructura unapiIpAddress_t a cadena con formato "A.B.C.D"
 * 
 * @param address Dirección IP a convertir
 * @param buffer Buffer donde escribir la cadena resultante (mínimo 16 bytes)
 * @return Puntero al buffer proporcionado
 */
char* unapinet_ip_to_string(const unapiIpAddress_t* address, char* buffer) __sdcccall(0)
{
	uint8_t i, j, digit;
	uint8_t val, started;
	uint8_t pos = 0;
	
	if (!address || !buffer) {
		if (buffer) {
			buffer[0] = '\0';
		}
		return buffer;
	}
	
	/* Convertir cada componente */
	for (i = 0; i < 4; i++) {
		val = address->ip[i];
		
		/* Manejar caso especial de cero */
		if (val == 0) {
			buffer[pos++] = '0';
		} else {
			/* Convertir valor a ascii, máximo 3 dígitos */
			started = 0;
			for (j = 100; j > 0; j /= 10) {
				digit = val / j;
				val = val % j;
				
				if (digit > 0 || started) {
					buffer[pos++] = digit + '0';
					started = 1;
				}
			}
		}
		
		/* Añadir punto excepto al final */
		if (i < 3) {
			buffer[pos++] = '.';
		}
	}
	
	/* Terminar la cadena */
	buffer[pos] = '\0';
	
	return buffer;
}