Start of my kernel project in C, using Qemu.



got better at assembly. Will write bad C code, as long as it works

need a cross gcc and cross gdb
```bash
git clone https://github.com/PoutineSyropErable/install_cross_gcc ~/cross-gcc
git clone https://github.com/PoutineSyropErable/install_cross_gdb ~/cross-gdb-new
```


```bash
sudo pacman -S qemu-full edk2-ovmf mtools dosfstools --needed
sudo pacman -S gcc-multilib --needed # might not bee needed. 

yay -S gcc-ia16
```


Use BOOTBOOT for uefi support in the future
Doing the 32 -> 64 bit trampoline is retarded. As it seems linking is a pain. 
Even if I do understand how the code is done, Doing the linking for it will be a pain and adventure in and off itself. 
So, 64 bit can go **** itself. 

You can have 16 bit and 32 bit in the same 

I'mma use bootboot for a uefi from scratch. And then I'll just port everything I wrote for that. 

File system driver using 

qemu-system-x86_64 \
    -m 512M \
    -machine q35 \
    -device ahci,id=ahci,bus=pcie.0,addr=0x90000000 \
    -drive file=disk.img,if=none,id=drive0,format=raw \
    -device ide-hd,drive=drive0,bus=ahci.0

or something

