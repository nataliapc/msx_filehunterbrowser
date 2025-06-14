/*
	Copyright (c) 2025 Natalia Pujol Cremades
	info@abitwitches.com

	See LICENSE file.
*/
#pragma once
#include <stdint.h>


// Struct to hold the request type
typedef struct {
	char value[4];
	char extension[5];
} ReqType_t;

// Struct to hold the MSX type
typedef struct {
	char name[4];
	char value[8];
} ReqMSX_t;

// Struct to hold the search term
#define SEARCH_MAX_SIZE		20
typedef struct {
	char value[SEARCH_MAX_SIZE+1];
} ReqSearch_t;

// Struct to hold the full request
typedef struct {
	ReqType_t  *type;
	ReqMSX_t   *msx;
	ReqSearch_t search;
	char       *download[4];
} Request_t;

// Struct to hold the section information
typedef struct {
	char       name[6];
	ReqType_t *type;
	char       key;
	uint8_t    posx;
	char      *description;
} Panel_t;

// Struct to hold a list item information
typedef struct {
	uint32_t name;			// Pointer to the name of the item
	uint16_t size;			// Parsed size
	uint8_t  loadMethod;	// [1 byte] Only for CAS
} ListItem_t;
