# define rk32
# 	target remote :1234 
# end
#
# define rk64
# 	set architecture i386:x86-64
# 	target remote :1234 
# end
#
#
# define activate_dashboard 
# 	source /usr/share/gdb-dashboard/.gdbinit
# end

define dc2 
	continue 
	continue
end








cd-src
file build/myos.elf
# rk32
# tbreak _start
# tbreak kernel_main
# c
# activate_dashboard

