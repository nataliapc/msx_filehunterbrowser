/*
unapinet_config.c
    Implementación de funciones de configuración IP para biblioteca UNAPI

Copyright (c) 2025

Este archivo es software libre bajo los términos de la licencia LGPL 2.1
*/

#include <stdlib.h>
#include "../../includes/unapi_net.h"

/* Declaraciones de funciones internas */
extern void unapinet_call(uint8_t function, Z80_registers* regs);
extern Z80_registers* unapinet_get_registers(void);
extern uint8_t unapinet_check_initialized(void);

/**
 * Establece si la configuración IP será automática o manual
 * 
 * @param autoMode Modo: 0=manual, 1=auto (local+máscara+gateway), 2=auto (DNS), 3=auto (todo)
 * @return Código de error UNAPI
 */
uint8_t unapinet_set_auto_ip_config(uint8_t autoMode) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar el modo */
	if (autoMode > 3) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Configurar parámetros para la llamada */
	regs->Bytes.B = 1; /* Establecer configuración */
	regs->Bytes.C = autoMode;
	
	/* Llamar a TCPIP_CONFIG_AUTOIP */
	unapinet_call(TCPIP_CONFIG_AUTOIP, regs);
	
	return regs->Bytes.A;
}

/**
 * Configura una dirección IP específica
 * 
 * @param type Tipo de dirección IP a configurar
 * @param address Dirección IP a establecer
 * @return Código de error UNAPI
 */
uint8_t unapinet_set_ip_address(uint8_t type, const unapiIpAddress_t* address) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!address || type < UNAPI_IP_LOCAL || type > UNAPI_IP_DNS_SECONDARY) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Configurar parámetros para la llamada */
	regs->Bytes.B = type;
	regs->Bytes.L = address->ip[0];
	regs->Bytes.H = address->ip[1];
	regs->Bytes.E = address->ip[2];
	regs->Bytes.D = address->ip[3];
	
	/* Llamar a TCPIP_CONFIG_IP */
	unapinet_call(TCPIP_CONFIG_IP, regs);
	
	return regs->Bytes.A;
}