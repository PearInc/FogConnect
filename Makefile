all : x68_linux_64

.PHONY : x68_linux_64

x68_linux_64:
	make -C ./examples

.PHONY : clean

clean :
	make -C ./examples clean

