/*
unapi_net.h
    Biblioteca para manejo de red a través de UNAPI para MSX
    Soporta tanto Ethernet como WiFi (ESP8266)

Copyright (c) 2025 

Este archivo es software libre: puedes redistribuirlo y/o modificarlo
bajo los términos de la Licencia Pública General Reducida GNU tal y como
la publicó la Free Software Foundation, en la versión 2.1 de la licencia
o (a tu elección) cualquier versión posterior.

Este archivo se distribuye con la esperanza de que sea útil,
pero SIN NINGUNA GARANTÍA; incluso sin la garantía implícita de
COMERCIALIZACIÓN o IDONEIDAD PARA UN PROPÓSITO PARTICULAR. Consulta la
Licencia Pública General Reducida GNU para más detalles.

Deberías haber recibido una copia de la Licencia Pública General Reducida GNU
junto con este programa. Si no es así, consulta <https://www.gnu.org/licenses/>
*/

#ifndef _UNAPI_NET_H
#define _UNAPI_NET_H

#include <stdint.h>

/* Constantes generales */
#define UNAPI_NET_MAX_CONNECTIONS 4      /* Número máximo de conexiones simultáneas */
#define UNAPI_NET_BUFFER_SIZE 512        /* Tamaño del buffer para envío/recepción */
#define UNAPI_NET_MAX_HOSTNAME 64        /* Longitud máxima de nombre de host */

/* Códigos de error estándar UNAPI */
typedef enum {
    UNAPI_ERR_OK = 0,             /* Sin error */
    UNAPI_ERR_NOT_IMP,            /* Función no implementada */
    UNAPI_ERR_NO_NETWORK,         /* Red no disponible */
    UNAPI_ERR_NO_DATA,            /* No hay datos disponibles */
    UNAPI_ERR_INV_PARAM,          /* Parámetro inválido */
    UNAPI_ERR_QUERY_EXISTS,       /* Ya existe una consulta DNS en proceso */
    UNAPI_ERR_INV_IP,             /* Dirección IP no válida */
    UNAPI_ERR_NO_DNS,             /* No hay servidor DNS configurado */
    UNAPI_ERR_DNS,                /* Error en servidor DNS */
    UNAPI_ERR_NO_FREE_CONN,       /* No hay conexiones libres disponibles */
    UNAPI_ERR_CONN_EXISTS,        /* La conexión ya existe */
    UNAPI_ERR_NO_CONN,            /* La conexión no existe */
    UNAPI_ERR_CONN_STATE,         /* Estado incorrecto de conexión */
    UNAPI_ERR_BUFFER,             /* Buffer insuficiente */
    UNAPI_ERR_LARGE_DGRAM,        /* Datagrama demasiado grande */
    UNAPI_ERR_INV_OPER            /* Operación no válida */
} unapiErrCode_t;

/* Estado de la conexión TCP */
typedef enum {
    UNAPI_TCP_STATE_CLOSED = 0,
    UNAPI_TCP_STATE_LISTEN,
    UNAPI_TCP_STATE_SYN_SENT,
    UNAPI_TCP_STATE_SYN_RECEIVED,
    UNAPI_TCP_STATE_ESTABLISHED,
    UNAPI_TCP_STATE_FIN_WAIT1,
    UNAPI_TCP_STATE_FIN_WAIT2,
    UNAPI_TCP_STATE_CLOSE_WAIT,
    UNAPI_TCP_STATE_CLOSING,
    UNAPI_TCP_STATE_LAST_ACK,
    UNAPI_TCP_STATE_TIME_WAIT
} unapiTcpState_t;

/* Tipo de conexión para TCP */
typedef enum {
    UNAPI_TCP_ACTIVE = 0,           /* Conexión activa */
    UNAPI_TCP_PASSIVE_S,            /* Conexión pasiva con socket remoto especificado */
    UNAPI_TCP_PASSIVE_U,            /* Conexión pasiva con socket remoto sin especificar */
    UNAPI_TCP_TLS_ACTIVE = 4        /* Conexión activa con TLS/SSL */
} unapiTcpType_t;

/* Estado de la red */
typedef enum {
    UNAPI_NET_CLOSED = 0,
    UNAPI_NET_OPENING,
    UNAPI_NET_OPEN,
    UNAPI_NET_CLOSING
} unapiNetState_t;

/* Estructura para información IP */
typedef struct {
    uint8_t ip[4];                  /* Dirección IP (formato A.B.C.D) */
} unapiIpAddress_t;

/* Estructura para el estado de una conexión TCP */
typedef struct {
    uint8_t state;                  /* Estado actual de la conexión */
    uint16_t localPort;             /* Puerto local */
    uint16_t remotePort;            /* Puerto remoto */
    unapiIpAddress_t remoteIP;      /* Dirección IP remota */
    uint16_t availableData;         /* Bytes disponibles para recepción */
    uint16_t sendSpace;             /* Espacio disponible para envío */
} unapiTcpInfo_t;

/* Estructura para el estado de una conexión UDP */
typedef struct {
    uint16_t localPort;             /* Puerto local */
    uint8_t pendingDgrams;          /* Número de datagramas pendientes */
    uint16_t nextDgramSize;         /* Tamaño del siguiente datagrama */
} unapiUdpInfo_t;

/* Estructura para los parámetros de apertura TCP */
typedef struct {
    unapiIpAddress_t remoteIP;      /* Dirección IP remota */
    uint16_t remotePort;            /* Puerto remoto */
    uint16_t localPort;             /* Puerto local (0xFFFF = aleatorio) */
    uint8_t flags;                  /* Flags de conexión */
    uint16_t timeout;               /* Timeout en segundos (0 = por defecto) */
    char* serverName;               /* Nombre del servidor para TLS (solo si flags tiene TLS activo) */
} unapiTcpConnectionParams_t;

/* Flags para conexiones TCP */
#define UNAPI_TCP_FLAG_NORMAL    0x00  /* Sin flags especiales */
#define UNAPI_TCP_FLAG_URGENT    0x01  /* Datos urgentes en envío */
#define UNAPI_TCP_FLAG_PUSH      0x02  /* Envío con flag PUSH activado */
#define UNAPI_TCP_FLAG_TLS       0x04  /* Conexión con TLS/SSL */
#define UNAPI_TCP_FLAG_VERIFY    0x08  /* Verificar certificado TLS */

/* Estructura para las capacidades UNAPI */
typedef struct {
    uint16_t flags;                 /* Capacidades básicas */
    uint16_t featureFlags;          /* Características adicionales */
    uint8_t maxTcpConn;             /* Máximo número de conexiones TCP */
    uint8_t maxUdpConn;             /* Máximo número de conexiones UDP */
    uint8_t linkLevelProtocol;      /* Protocolo de nivel de enlace */
    uint16_t maxInDgramSize;        /* Tamaño máximo de datagrama entrante */
    uint16_t maxOutDgramSize;       /* Tamaño máximo de datagrama saliente */
    uint16_t flags2;                /* Segundo conjunto de capacidades */
} unapiCapabilities_t;

/* Códigos de funciones TCP/IP UNAPI */
typedef enum {
    TCPIP_GET_CAPAB = 1,            /* Obtener capacidades del adaptador */
    TCPIP_GET_IPINFO = 2,           /* Obtener información IP */
    TCPIP_NET_STATE = 3,            /* Obtener estado de la red */
    TCPIP_SEND_ECHO = 4,            /* Enviar ping (echo) */
    TCPIP_RCV_ECHO = 5,             /* Recibir respuesta de ping */
    TCPIP_DNS_Q = 6,                /* Consulta DNS */
    TCPIP_DNS_Q_NEW = 206,          /* Consulta DNS (nueva versión) */
    TCPIP_DNS_S = 7,                /* Estado de la consulta DNS */
    TCPIP_UDP_OPEN = 8,             /* Abrir conexión UDP */
    TCPIP_UDP_CLOSE = 9,            /* Cerrar conexión UDP */
    TCPIP_UDP_STATE = 10,           /* Estado de la conexión UDP */
    TCPIP_UDP_SEND = 11,            /* Enviar datagrama UDP */
    TCPIP_UDP_RCV = 12,             /* Recibir datagrama UDP */
    TCPIP_TCP_OPEN = 13,            /* Abrir conexión TCP */
    TCPIP_TCP_CLOSE = 14,           /* Cerrar conexión TCP */
    TCPIP_TCP_ABORT = 15,           /* Abortar conexión TCP */
    TCPIP_TCP_STATE = 16,           /* Estado de la conexión TCP */
    TCPIP_TCP_SEND = 17,            /* Enviar datos por TCP */
    TCPIP_TCP_RCV = 18,             /* Recibir datos por TCP */
    TCPIP_TCP_FLUSH = 19,           /* Descartar datos TCP */
    TCPIP_CONFIG_AUTOIP = 25,       /* Configurar obtención automática de IP */
    TCPIP_CONFIG_IP = 26,           /* Configurar dirección IP */
    TCPIP_WAIT = 29                 /* Esperar eventos de red */
} unapiFunction_t;

/* Tipos de direcciones IP para GET_IPINFO */
typedef enum {
    UNAPI_IP_LOCAL = 1,             /* Dirección IP local */
    UNAPI_IP_REMOTE = 2,            /* Dirección IP remota */
    UNAPI_IP_NETMASK = 3,           /* Máscara de subred */
    UNAPI_IP_GATEWAY = 4,           /* Gateway por defecto */
    UNAPI_IP_DNS_PRIMARY = 5,       /* Servidor DNS primario */
    UNAPI_IP_DNS_SECONDARY = 6      /* Servidor DNS secundario */
} unapiIpType_t;

/* ======================================================================
   FUNCIONES DE LA BIBLIOTECA
   ====================================================================== */

/**
 * Inicializa la biblioteca y busca implementaciones UNAPI de red disponibles
 * 
 * @return 1 si se encontró al menos una implementación, 0 si no
 */
uint8_t unapinet_init(void) __sdcccall(0);

/**
 * Libera los recursos utilizados por la biblioteca
 */
void unapinet_term(void) __sdcccall(0);

/**
 * Obtiene las capacidades de la implementación UNAPI
 * 
 * @param capabilities Puntero a estructura donde guardar las capacidades
 * @return Código de error UNAPI
 */
uint8_t unapinet_get_capabilities(unapiCapabilities_t* capabilities) __sdcccall(0);

/**
 * Obtiene el estado actual de la red
 * 
 * @return Estado actual de la red
 */
uint8_t unapinet_get_net_state(void) __sdcccall(0);

/**
 * Obtiene una dirección IP del tipo especificado
 * 
 * @param type Tipo de dirección IP a obtener
 * @param address Puntero donde guardar la dirección
 * @return Código de error UNAPI
 */
uint8_t unapinet_get_ip_address(uint8_t type, unapiIpAddress_t* address) __sdcccall(0);

/**
 * Resuelve un nombre de host a una dirección IP
 * 
 * @param hostname Nombre de host a resolver
 * @param address Puntero donde guardar la dirección IP
 * @return Código de error UNAPI
 */
uint8_t unapinet_dns_resolve(const char* hostname, unapiIpAddress_t* address) __sdcccall(0);

/**
 * Abre una conexión TCP 
 * 
 * @param params Parámetros de la conexión
 * @param connNumber Puntero para devolver el número de conexión
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_open(unapiTcpConnectionParams_t* params, uint8_t* connNumber) __sdcccall(0);

/**
 * Cierra una conexión TCP de forma ordenada
 * 
 * @param connNumber Número de conexión a cerrar
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_close(uint8_t connNumber) __sdcccall(0);

/**
 * Aborta una conexión TCP inmediatamente
 * 
 * @param connNumber Número de conexión a abortar
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_abort(uint8_t connNumber) __sdcccall(0);

/**
 * Obtiene el estado actual de una conexión TCP
 * 
 * @param connNumber Número de conexión
 * @param info Puntero a estructura donde guardar la información
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_state(uint8_t connNumber, unapiTcpInfo_t* info) __sdcccall(0);

/**
 * Envía datos por una conexión TCP
 * 
 * @param connNumber Número de conexión
 * @param data Datos a enviar
 * @param length Longitud de los datos
 * @param flags Flags para el envío (URGENTE, PUSH)
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_send(uint8_t connNumber, const void* data, uint16_t length, uint8_t flags) __sdcccall(0);

/**
 * Recibe datos de una conexión TCP
 * 
 * @param connNumber Número de conexión
 * @param buffer Buffer donde recibir los datos
 * @param maxLength Tamaño máximo a recibir
 * @param receivedLength Puntero para devolver la cantidad recibida
 * @return Código de error UNAPI
 */
uint8_t unapinet_tcp_receive(uint8_t connNumber, void* buffer, uint16_t maxLength, uint16_t* receivedLength) __sdcccall(0);

/**
 * Abre una conexión UDP
 * 
 * @param localPort Puerto local (0 = aleatorio)
 * @param connNumber Puntero para devolver el número de conexión
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_open(uint16_t localPort, uint8_t* connNumber) __sdcccall(0);

/**
 * Cierra una conexión UDP
 * 
 * @param connNumber Número de conexión a cerrar
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_close(uint8_t connNumber) __sdcccall(0);

/**
 * Obtiene el estado de una conexión UDP
 * 
 * @param connNumber Número de conexión
 * @param info Puntero a estructura donde guardar la información
 * @return Código de error UNAPI
 */
uint8_t unapinet_udp_state(uint8_t connNumber, unapiUdpInfo_t* info) __sdcccall(0);

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
                          uint16_t remotePort, const void* data, uint16_t length) __sdcccall(0);

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
                            uint16_t* receivedLength) __sdcccall(0);

/**
 * Cede tiempo al adaptador para procesar eventos de red
 */
void unapinet_process_events(void) __sdcccall(0);

/**
 * Establece si la configuración IP será automática o manual
 * 
 * @param autoMode Modo: 0=manual, 1=auto (local+máscara+gateway), 2=auto (DNS), 3=auto (todo)
 * @return Código de error UNAPI
 */
uint8_t unapinet_set_auto_ip_config(uint8_t autoMode) __sdcccall(0);

/**
 * Configura una dirección IP específica
 * 
 * @param type Tipo de dirección IP a configurar
 * @param address Dirección IP a establecer
 * @return Código de error UNAPI
 */
uint8_t unapinet_set_ip_address(uint8_t type, const unapiIpAddress_t* address) __sdcccall(0);

/**
 * Convierte una cadena con formato IP "A.B.C.D" a estructura unapiIpAddress_t
 * 
 * @param ipString Cadena con la dirección IP
 * @param address Puntero a estructura donde guardar la dirección
 * @return 1 si la conversión fue correcta, 0 si la cadena no es una IP válida
 */
uint8_t unapinet_string_to_ip(const char* ipString, unapiIpAddress_t* address) __sdcccall(0);

/**
 * Convierte una estructura unapiIpAddress_t a cadena con formato "A.B.C.D"
 * 
 * @param address Dirección IP a convertir
 * @param buffer Buffer donde escribir la cadena resultante
 * @return Puntero al buffer proporcionado
 */
char* unapinet_ip_to_string(const unapiIpAddress_t* address, char* buffer) __sdcccall(0);

#endif /* _UNAPI_NET_H */