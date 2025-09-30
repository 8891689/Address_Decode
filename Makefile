.PHONY: default clean

default:
	gcc -static -lpthread -m64 -march=native -mtune=native -Wall -Wextra -Ofast -ftree-vectorize -funroll-all-loops -flto base58.c bech32.c cashaddr.c main.c sha256.c -o decode

clean:
	rm -f decode
