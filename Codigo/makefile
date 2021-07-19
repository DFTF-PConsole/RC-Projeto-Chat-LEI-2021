all: cliente.out servidor.out

cliente.out: cliente.o global.o
	gcc -std=c11 -Wall -Wextra -o cliente.out cliente.o global.o
	
servidor.out: servidor.o global.o
	gcc -std=c11 -Wall -Wextra -o servidor.out servidor.o global.o
	
cliente.o: cliente.c header_comum.h header_cliente.h
	gcc -std=c11 -Wall -Wextra -c cliente.c
	
servidor.o: servidor.c header_comum.h header_servidor.h
	gcc -std=c11 -Wall -Wextra -c servidor.c
	
global.o: global.c header_comum.h header_global.h
	gcc -std=c11 -Wall -Wextra -c global.c