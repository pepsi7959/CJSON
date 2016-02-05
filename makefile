all: test

clean: 
	rm test_json

test: test_json.c json.c
	gcc -g test_json.c -o test_json json.c ./hash_map/hmap.c -I./hash_map
json : json.c
	gcc -g json.c ./hash_map/hmap.c -I. -I./hash_map -o json.o

