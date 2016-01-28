#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int read_file(char *buff, int size ,const char *pathname){
	if( buff == NULL){
		return -1;
	}
	FILE *file;

	printf("filename : %s \n", pathname);
	file = fopen(pathname, "r");
	int r = fscanf(file, "%s", buff);
	return 0;
}

int main(int argc, char *argv[]){
	int ret = 0;
	ejson_obj_t *obj;
	char error[1024];
	char data[1024];
	if( argc == 2){
		read_file(data, 1024, argv[1]);
	}else{
		sprintf(data, "{\"key1\":\"value1\",\"key1.2\":[\"v1\",\"v2\",{\"A\":\"AV\"}],\"key2\":{\"key3\":\"value3\"}}");
	}

	if ( ejson_to_object(data, (int)strlen(data), &obj ) != 0){
		printf("ejson_to_obj return error[%s]\n", error);
		return -1;
	}

	
	return 0;
}