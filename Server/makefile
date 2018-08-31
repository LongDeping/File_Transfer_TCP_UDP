server : server.o md5.o
	gcc -o server server.o md5.o -lpthread
server.o : server.c
	gcc -c server.c 
md5.o : ./MD5Lib/md5.c  ./MD5Lib/md5.h
	gcc -c ./MD5Lib/md5.c
.PHONY : clean
clean:
	rm -f *.o server
