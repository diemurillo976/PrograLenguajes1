#Para compilar se va al directorio de los archivos y se escribe en la terminal: make
#Esto compilara ambos archivos y generara los ejecutables
#Luego para correr los ejecutables se debe abrir 1 terminal para el server y 1 terminal por cada cliente
#Para ejecutar el codigo del servidor: ./server 
#Para ejecutar el codigo del cliente: ./client 
#PD: si se desea registrar un cliente se debe poner: ./client username, donde username es el nombre de usuario deseado
#Recordar cambiar el puerto del cliente en el archivo portNumber.ini si se corren varios clientes
all: server client

server: Server.c
	gcc Server.h Server.c -o server
	
client: Client.c
	gcc Client.h Client.c -o client