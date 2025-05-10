/*
unapinet_udp.c
    Implementación de funciones UDP para biblioteca UNAPI

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
 * Abre una conexión UDP
 * 
 * @param localPort Puerto local (0 = aleatorio)
 * @param connNumber Puntero para devolver el número de conexión
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_open(uint16_t localPort, uint8_t* connNumber) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!connNumber) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Configurar parámetros para la llamada */
	regs->UWords.HL = localPort;
	regs->Bytes.C = 1; /* Conexión permanente */
	
	/* Llamar a TCPIP_UDP_OPEN */
	unapinet_call(TCPIP_UDP_OPEN, regs);
	
	/* Verificar resultado */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		return regs->Bytes.A;
	}
	
	/* Devolver el número de conexión */
	*connNumber = regs->Bytes.B;
	
	return UNAPI_ERR_OK;
}

/**
 * Cierra una conexión UDP
 * 
 * @param connNumber Número de conexión a cerrar
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_close(uint8_t connNumber) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	regs = unapinet_get_registers();
	
	/* Configurar número de conexión */
	regs->Bytes.B = connNumber;
	
	/* Llamar a TCPIP_UDP_CLOSE */
	unapinet_call(TCPIP_UDP_CLOSE, regs);
	
	return regs->Bytes.A;
}

/**
 * Obtiene el estado de una conexión UDP
 * 
 * @param connNumber Número de conexión
 * @param info Puntero a estructura donde guardar la información
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_state(uint8_t connNumber, unapiUdpInfo_t* info) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!info) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Configurar número de conexión */
	regs->Bytes.B = connNumber;
	
	/* Llamar a TCPIP_UDP_STATE */
	unapinet_call(TCPIP_UDP_STATE, regs);
	
	/* Verificar resultado */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		return regs->Bytes.A;
	}
	
	/* Llenar la estructura con la información */
	info->localPort = regs->UWords.HL;
	info->pendingDgrams = regs->Bytes.B;
	info->nextDgramSize = regs->UWords.DE;
	
	return UNAPI_ERR_OK;
}

/**
 * Envía un datagrama UDP
 * 
 * @param connNumber Número de conexión
 * @param remoteIP Dirección IP destino
 * @param remotePort Puerto destino
 * @param data Datos a enviar
 * @param length Longitud de los datos
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_send(uint8_t connNumber, const unapiIpAddress_t* remoteIP, 
                          uint16_t remotePort, const void* data, uint16_t length) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!remoteIP || !data || length == 0) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Configurar parámetros para la llamada */
	regs->Bytes.B = connNumber;
	regs->Bytes.L = remoteIP->ip[0];
	regs->Bytes.H = remoteIP->ip[1];
	regs->Bytes.E = remoteIP->ip[2];
	regs->Bytes.D = remoteIP->ip[3];
	regs->UWords.IX = remotePort;
	regs->UWords.IY = (uint16_t)data;
	regs->UWords.BC = length;
	
	/* Llamar a TCPIP_UDP_SEND */
	unapinet_call(TCPIP_UDP_SEND, regs);
	
	return regs->Bytes.A;
}

/**
 * Recibe un datagrama UDP
 * 
 * @param connNumber Número de conexión
 * @param remoteIP Puntero para devolver la dirección IP origen
 * @param remotePort Puntero para devolver el puerto origen
 * @param buffer Buffer donde recibir los datos
 * @param maxLength Tamaño máximo a recibir
 * @param receivedLength Puntero para devolver la cantidad recibida
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_receive(uint8_t connNumber, unapiIpAddress_t* remoteIP, 
                            uint16_t* remotePort, void* buffer, uint16_t maxLength,
                            uint16_t* receivedLength) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!remoteIP || !remotePort || !buffer || !receivedLength || maxLength == 0) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Configurar parámetros para la llamada */
	regs->Bytes.B = connNumber;
	regs->UWords.DE = (uint16_t)buffer;
	regs->UWords.HL = maxLength;
	
	/* Llamar a TCPIP_UDP_RCV */
	unapinet_call(TCPIP_UDP_RCV, regs);
	
	/* Verificar resultado */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		*receivedLength = 0;
		return regs->Bytes.A;
	}
	
	/* Guardar la dirección IP origen */
	remoteIP->ip[0] = regs->Bytes.L;
	remoteIP->ip[1] = regs->Bytes.H;
	remoteIP->ip[2] = regs->Bytes.E;
	remoteIP->ip[3] = regs->Bytes.D;
	
	/* Guardar el puerto origen */
	*remotePort = regs->UWords.IX;
	
	/* Guardar la cantidad de datos recibidos */
	*receivedLength = regs->UWords.BC;
	
	return UNAPI_ERR_OK;
}