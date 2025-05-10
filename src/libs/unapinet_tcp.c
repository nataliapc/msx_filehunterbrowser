/*
unapinet_tcp.c
    Implementación de funciones TCP para biblioteca UNAPI

Copyright (c) 2025

Este archivo es software libre bajo los términos de la licencia LGPL 2.1
*/

#include <stdlib.h>
#include <string.h>
#include "../../includes/unapi_net.h"

/* Declaraciones de funciones internas */
extern void unapinet_call(uint8_t function, Z80_registers* regs);
extern Z80_registers* unapinet_get_registers(void);
extern uint8_t unapinet_check_initialized(void);

/**
 * Abre una conexión TCP 
 * 
 * @param params Parámetros de la conexión
 * @param connNumber Puntero para devolver el número de conexión
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_open(unapiTcpConnectionParams_t* params, uint8_t* connNumber) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;
	uint16_t serverNameLen = 0;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!params || !connNumber) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();

	/* Configurar parámetros para la conexión */
	/* Dirección IP remota */
	regs->Bytes.L = params->remoteIP.ip[0];
	regs->Bytes.H = params->remoteIP.ip[1];
	regs->Bytes.E = params->remoteIP.ip[2];
	regs->Bytes.D = params->remoteIP.ip[3];
	
	/* Puertos y flags */
	regs->UWords.IX = params->remotePort;
	regs->UWords.IY = params->localPort;
	regs->Bytes.C = params->flags;
	
	/* Timeout */
	regs->UWords.BC = params->timeout;
	
	/* Nombre del servidor para conexiones TLS (si es necesario) */
	if ((params->flags & UNAPI_TCP_FLAG_TLS) && params->serverName) {
		/* Incluir el nombre del servidor para validación de certificado */
		serverNameLen = strlen(params->serverName) + 1; /* +1 para incluir el cero final */
		
		/* HL debe apuntar al nombre del servidor */
		regs->Words.HL = (uint16_t)params->serverName;
	}

	/* Llamar a TCPIP_TCP_OPEN */
	unapinet_call(TCPIP_TCP_OPEN, regs);
	
	/* Verificar resultado */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		return regs->Bytes.A;
	}
	
	/* Devolver el número de conexión */
	*connNumber = regs->Bytes.B;
	
	return UNAPI_ERR_OK;
}

/**
 * Cierra una conexión TCP de forma ordenada
 * 
 * @param connNumber Número de conexión a cerrar
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_close(uint8_t connNumber) __sdcccall(0)
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
	
	/* Llamar a TCPIP_TCP_CLOSE */
	unapinet_call(TCPIP_TCP_CLOSE, regs);
	
	return regs->Bytes.A;
}

/**
 * Aborta una conexión TCP inmediatamente
 * 
 * @param connNumber Número de conexión a abortar
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_abort(uint8_t connNumber) __sdcccall(0)
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
	
	/* Llamar a TCPIP_TCP_ABORT */
	unapinet_call(TCPIP_TCP_ABORT, regs);
	
	return regs->Bytes.A;
}

/**
 * Obtiene el estado actual de una conexión TCP
 * 
 * @param connNumber Número de conexión
 * @param info Puntero a estructura donde guardar la información
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_state(uint8_t connNumber, unapiTcpInfo_t* info) __sdcccall(0)
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
	
	/* Llamar a TCPIP_TCP_STATE */
	unapinet_call(TCPIP_TCP_STATE, regs);
	
	/* Verificar resultado */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		return regs->Bytes.A;
	}
	
	/* Llenar la estructura con la información */
	info->state = regs->Bytes.B;
	info->availableData = regs->UWords.DE;
	info->sendSpace = regs->UWords.HL;
	
	/* Obtener información adicional - dirección IP y puerto remoto */
	/* Para esto se necesita usar TCPIP_GET_IPINFO */
	regs->Bytes.B = UNAPI_IP_REMOTE;
	unapinet_call(TCPIP_GET_IPINFO, regs);
	
	if (regs->Bytes.A == UNAPI_ERR_OK) {
		info->remoteIP.ip[0] = regs->Bytes.L;
		info->remoteIP.ip[1] = regs->Bytes.H;
		info->remoteIP.ip[2] = regs->Bytes.E;
		info->remoteIP.ip[3] = regs->Bytes.D;
	} else {
		/* Si hay error, poner IP a ceros */
		memset(&(info->remoteIP), 0, sizeof(unapiIpAddress_t));
	}
	
	/* No hay forma directa de obtener los puertos en la especificación UNAPI */
	/* Los dejamos como desconocidos */
	info->localPort = 0;
	info->remotePort = 0;
	
	return UNAPI_ERR_OK;
}

/**
 * Envía datos por una conexión TCP
 * 
 * @param connNumber Número de conexión
 * @param data Datos a enviar
 * @param length Longitud de los datos
 * @param flags Flags para el envío (URGENTE, PUSH)
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_send(uint8_t connNumber, const void* data, uint16_t length, uint8_t flags) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;
	uint16_t sent = 0;
	uint16_t chunk;
	static const uint16_t MAX_CHUNK = 512; /* Tamaño máximo de envío en cada llamada */

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!data || length == 0) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Enviar los datos en fragmentos para evitar problemas de buffer */
	while (sent < length) {
		/* Calcular tamaño del fragmento actual */
		chunk = (length - sent) > MAX_CHUNK ? MAX_CHUNK : (length - sent);
		
		/* Configurar parámetros para la llamada */
		regs->Bytes.B = connNumber;
		regs->Bytes.C = flags;
		regs->Words.HL = (uint16_t)((uint8_t*)data + sent);
		regs->Words.DE = chunk;
		
		/* Llamar a TCPIP_TCP_SEND */
		unapinet_call(TCPIP_TCP_SEND, regs);
		
		/* Si hay error, terminar */
		if (regs->Bytes.A != UNAPI_ERR_OK) {
			return regs->Bytes.A;
		}
		
		/* Actualizar cantidad enviada */
		sent += chunk;
		
		/* Si hay más datos, dar tiempo al adaptador */
		if (sent < length) {
			unapinet_process_events();
		}
	}
	
	return UNAPI_ERR_OK;
}

/**
 * Recibe datos de una conexión TCP
 * 
 * @param connNumber Número de conexión
 * @param buffer Buffer donde recibir los datos
 * @param maxLength Tamaño máximo a recibir
 * @param receivedLength Puntero para devolver la cantidad recibida
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_receive(uint8_t connNumber, void* buffer, uint16_t maxLength, uint16_t* receivedLength) __sdcccall(0)
{
	Z80_registers* regs;
	uint8_t result;

	/* Verificar inicialización */
	result = unapinet_check_initialized();
	if (result != UNAPI_ERR_OK) {
		return result;
	}

	/* Validar parámetros */
	if (!buffer || !receivedLength || maxLength == 0) {
		return UNAPI_ERR_INV_PARAM;
	}

	regs = unapinet_get_registers();
	
	/* Configurar parámetros para la llamada */
	regs->Bytes.B = connNumber;
	regs->Words.DE = (uint16_t)buffer;
	regs->Words.HL = maxLength;
	
	/* Llamar a TCPIP_TCP_RCV */
	unapinet_call(TCPIP_TCP_RCV, regs);
	
	/* Verificar resultado */
	if (regs->Bytes.A != UNAPI_ERR_OK) {
		*receivedLength = 0;
		return regs->Bytes.A;
	}
	
	/* Devolver la cantidad recibida (BC contiene los bytes recibidos) */
	*receivedLength = regs->UWords.BC;
	
	return UNAPI_ERR_OK;
}