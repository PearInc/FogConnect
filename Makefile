all : x68_linux_64  mips

.PHONY : x68_linux_64 mips

x68_linux_64:
	make -C ./x86/linux/64

mips:
	make -C ./mips

.PHONY : clean

clean :
	make -C ././x86/linux/64 clean
	make -C ./mips clean





