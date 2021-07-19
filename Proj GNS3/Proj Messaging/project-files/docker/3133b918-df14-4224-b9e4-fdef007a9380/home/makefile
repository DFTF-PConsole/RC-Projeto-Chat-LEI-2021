all: cliente.out

cliente.out: cliente.o global.o
	gcc -std=c11 -Wall -Wextra -o cliente.out cliente.o global.o
	
cliente.o: cliente.c header_comum.h header_cliente.h
	gcc -std=c11 -Wall -Wextra -c cliente.c
	
global.o: global.c header_comum.h header_global.h
	gcc -std=c11 -Wall -Wextra -c global.c
