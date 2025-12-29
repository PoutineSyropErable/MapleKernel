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
