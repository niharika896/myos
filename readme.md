assemble:
```
i686-elf-as boot.s -o boot.o
i686-elf-as gdt_flush.s -o gdt_flush.o
```

compile:
```
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -c gdt.c -o gdt.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
```

link:
```
i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o gdt_flush.o kernel.o gdt.o -lgcc
```

generate iso:
```
grub-mkrescue -o myos.iso isodir
```



