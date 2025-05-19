#https://www.msx.org/forum/msx-talk/openmsx/tcl-in-openmsx?page=1

proc wait_until_boot {} {
	global speed
	set speed 100
}

#toggle_vdp_busy
set save_settings_on_exit off

bind F6 cycle videosource
#set videosource GFX9000
#cycle v9990cmdtrace

set fullspeedwhenloading on
debug set_watchpoint write_io 0x18
after time 14 wait_until_boot

# Starting the emulation at Full Speed
#set throttle off										
set speed 9999
# After 18 OpenMSX clocks (?), the normal speed of the computer is set back to normal
#after time 18 "set speed 100"

# Emulate the Cursors keys as Joystick in MSX's joystick Port A
#plug joyporta keyjoystick1

# Emulate your mouse as a MSX mouse in MSX's joystick port B
plug joyportb mouse

# Plug a Simple/Covox Module in the Printer Port. Default Audio output used
plug printerport simpl

if {false} {
	# scripts/cargar_sym.tcl - Script Tcl de inicio para openMSX
	# Ruta del archivo .sym generado (relativa al directorio actual)
	set symFile [file join [pwd] "obj/program.sym"]

	# Si el archivo existe, procedemos a procesarlo
	if {[file exists $symFile]} {
		puts "Cargando símbolos desde $symFile..."
		# Abrir el archivo de símbolos para leer
		set fp [open $symFile r]
		while {[gets $fp line] >= 0} {
			# Buscar líneas con formato "Etiqueta: equ XXXXh"
			if {[regexp {^([\w\.]+):\s+equ\s+([0-9A-Fa-f]+)H} $line -> etiqueta direccionHex]} {
				# Convertir la dirección hexadecimal (sin la 'H' final) a número
				scan $direccionHex %x addr
				# Añadir un watch de RAM en esa dirección con la etiqueta como descripción
				ram_watch add $addr -desc $etiqueta -type word
			}
		}
		close $fp
		puts "Archivo .sym cargado en openMSX."
	} else {
		puts "Archivo $symFile no encontrado."
	}
}