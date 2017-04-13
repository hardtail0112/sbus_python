gcc -fpic -o sbus.o -c sbus.c
gcc -fpic -I/usr/include/python2.7/ -o sbusWrapper.o -c sbusWrapper.c 
gcc -shared sbus.o sbusWrapper.o -o sbusmodule.so
