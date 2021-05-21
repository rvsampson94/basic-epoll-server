all: main.c
	gcc -fno-stack-protector -no-pie -z execstack -o main main.c