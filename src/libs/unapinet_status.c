/*
unapinet_status.c
    Implementación de funciones para obtener información sobre las capacidades y estado de red

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
 * Obtiene las capacidades de la implementación UNAPI
 * 
 * @param capabilities Puntero a estructura donde guardar las capacidades
 * @return Código de error UNAPI
 */
uint8_t unapinet_get_capabilities(unapiCapabilities_t* capabilities) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	if (!capabilities) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();

	/* Obtener capacidades básicas */
	regs->Bytes.B = TCPIP_GET_CAPAB_FLAGS;
	unapinet_call(TCPIP_GET_CAPAB, regs);
	capabilities->flags = regs->UWords.HL;
	capabilities->featureFlags = regs->UWords.DE;
	capabilities->linkLevelProtocol = regs->Bytes.B;

	/* Obtener información de conexiones */
	regs->Bytes.B = TCPIP_GET_CAPAB_CONN;
	unapinet_call(TCPIP_GET_CAPAB, regs);
	capabilities->maxTcpConn = regs->Bytes.B;
	capabilities->maxUdpConn = regs->Bytes.C;

	/* Obtener información de tamaños de datagramas */
	regs->Bytes.B = TCPIP_GET_CAPAB_DGRAM;
	unapinet_call(TCPIP_GET_CAPAB, regs);
	capabilities->maxInDgramSize = regs->UWords.HL;
	capabilities->maxOutDgramSize = regs->UWords.DE;

	/* Obtener capacidades secundarias si están disponibles */
	regs->Bytes.B = TCPIP_GET_SECONDARY_CAPAB_FLAGS;
	unapinet_call(TCPIP_GET_CAPAB, regs);
	if (regs->Bytes.A == UNAPI_ERR_OK) {
		capabilities->flags2 = regs->UWords.HL;
	} else {
		capabilities->flags2 = 0;
	}

	return UNAPI_ERR_OK;
}

/**
 * Obtiene el estado actual de la red
 * 
 * @return Estado actual de la red
 */
uint8_t unapinet_get_net_state(void) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return UNAPI_NET_CLOSED;
	}

	regs = unapinet_get_registers();

	/* Llamar a TCPIP_NET_STATE */
	unapinet_call(TCPIP_NET_STATE, regs);
	
	/* Si hay error, asumimos estado cerrado */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		return UNAPI_NET_CLOSED;
	}

	return regs->Bytes.B;
}

/**
 * Obtiene una dirección IP del tipo especificado
 * 
 * @param type Tipo de dirección IP a obtener
 * @param address Puntero donde guardar la dirección
 * @return Código de error UNAPI
 */
uint8_t unapinet_get_ip_address(uint8_t type, unapiIpAddress_t* address) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	if (!address || type < UNAPI_IP_LOCAL || type > UNAPI_IP_DNS_SECONDARY) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();

	/* Llamada a TCPIP_GET_IPINFO */
	regs->Bytes.B = type;
	unapinet_call(TCPIP_GET_IPINFO, regs);

	if (regs->Bytes.A != UNAPI_ERR_OK) {
		return regs->Bytes.A;
	}

	/* Copiar la dirección IP del resultado */
	address->ip[0] = regs->Bytes.L;
	address->ip[1] = regs->Bytes.H;
	address->ip[2] = regs->Bytes.E;
	address->ip[3] = regs->Bytes.D;

	return UNAPI_ERR_OK;
}

/**
 * Cede tiempo al adaptador para procesar eventos de red
 */
void unapinet_process_events(void) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return;
	}

	regs = unapinet_get_registers();

	/* Llamar a TCPIP_WAIT */
	unapinet_call(TCPIP_WAIT, regs);
}