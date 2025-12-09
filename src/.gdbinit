define rk32
	target remote :1234 


end

define rk64
	set architecture i386:x86-64
	target remote :1234 
end

define dc2 
	continue 
	continue
end

define activate_dashboard 
	source /usr/share/gdb-dashboard/.gdbinit
end



file build/myos.bin
# tbreak _start
tbreak kernel_main
# rk32
# dc2
# activate_dashboard

