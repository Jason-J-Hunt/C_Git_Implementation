all: WTF WTFServer

WTFServer:	WTFServer.c
	gcc -g -o WTFServer WTFServer.c -lpthread

WTF: WTF.c
	gcc -g -o WTF WTF.c -lssl -lcrypto
clean:
	rm -f WTF WTFServer