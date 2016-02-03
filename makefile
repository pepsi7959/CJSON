all: test

clean: 
	rm test_json

test: test_json.c json.c
	gcc -g test_json.c -o test_json json.c hmap.c
json : json.c
	gcc -g json.c hmap.c -I. -o json.o

