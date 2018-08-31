client : client.o md5.o
	gcc -o client client.o md5.o
client.o : client.c
	gcc -c client.c
md5.o : ./MD5Lib/md5.c  ./MD5Lib/md5.h
	gcc -c ./MD5Lib/md5.c
.PHONY : clean
clean:
	rm -f *.o client
