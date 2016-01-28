all: test

clean: 
	rm test_json

test: test_json.c json.c
	gcc -g test_json.c -o test_json json.c
json : json.c
	gcc -g json.c -I. -o json.o

