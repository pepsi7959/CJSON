all: test

clean: 
	rm test_json

test: test_json.c json.c
	gcc test_json.c -o test_json json.c
json : json.c
	gcc json.c -I. -o json.o

