#!/usr/bin/python3.8
# https://gitlab.com/brossaip/msx_bricks/-/blob/main/create_sym_debug.py?ref_type=heads
# Li passem el paràmetre del nom sense extensió i ell obre els fitxers .lst i .map per crear un .sym per ser llegit per l'openmsx-debugger

import sys

if len(sys.argv) == 2:
    nomFitx = sys.argv[1]
else:
    raise Exception("The root of the file name is needed")

mapFitx = open(nomFitx + '.map', "r")
# Creem un diccionari de les posicions de totes les funcions que hi ha al .map per després poder-les consultar
linies = mapFitx.readlines()

symFitx = open(nomFitx + '_opmdeb.sym', 'w')

trobatVars = False
for linia in linies:
    if linia.startswith('_DATA') or linia.startswith('_HEADER') or linia.startswith('_INITIALIZED'):
        trobatVars = True
    if trobatVars == True and linia.find('_') > 0:
        address = linia[0:13].strip()
        name = linia[15:48].strip()
        symFitx.write(f"{name}: equ {address}H\n")
    if trobatVars == True and linia.startswith('_') and not linia.startswith('_DATA') and not linia.startswith('_HEADER') and not linia.startswith('_INITIALIZED'):
        trobatVars = False

# trobatCode = False
# dicMap = {}
# for linia in linies:
#     if linia.find('_CODE') >= 0 and linia.find('_CODE') <= 6:
#         trobatCode = True
#     if trobatCode == True and linia.find('_') > 0:
#         dicMap[linia[15:48].strip()] = linia[0:13].strip()
#     if trobatCode == True and linia.find('Linker') > 0:
#         break

# lstFitx = open(nomFitx + '.lst', "r")

# linia = lstFitx.readline()
# comencenFuncions = False
# adrecaBase = 0
# while linia:
#     if linia.find('; Function') >= 0:
#         comencenFuncions = True
#         linia = lstFitx.readline()
#         linia = lstFitx.readline()
#         nomFunc = linia[40:-3]
#         comptaBytesInit = int(linia[0:12], 16)
#         try:
#             adrecaBase = dicMap[nomFunc]
#             symFitx.write(f"{nomFunc}: equ {adrecaBase}H\n")
#             adrecaBase = int(adrecaBase, 16)
#         except KeyError:
#             adrecaBase = adrecaBase + comptaBytesInit
#     if linia.find(';') >= 0 and linia.find(':') > 0 and comencenFuncions:
#         text = linia[40:].strip()
#         linia2 = lstFitx.readline()
#         try:
#             posBytesStatement = int(linia2[0:12], 16)
#         except ValueError:
#             # Després hi ha una funció, la detectarem amb el de dalt
#             pass
#         symFitx.write(f"{text}: equ {hex(adrecaBase + posBytesStatement - comptaBytesInit)[2:]}H\n")

#     linia = lstFitx.readline()

symFitx.close()
mapFitx.close()
#lstFitx.close()
