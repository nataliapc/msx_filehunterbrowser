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
