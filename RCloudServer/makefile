
GDB = -g

all : ser

ser : ser.o work.o
	gcc -o ser ser.o work.o -levent -lmysqlclient 

ser.o : ser.c
	gcc -c ser.c ${GDB}

work.o : work.c
	gcc -c work.c ${GDB}

clean:
	rm -rf ser *.o
