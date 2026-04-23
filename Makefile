.PHONY: main build run

main: build run

build:
	mkdir -p bin
	gcc main.c -g -o ./bin/main

run:
	./bin/main $(ARGS)
