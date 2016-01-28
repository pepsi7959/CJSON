#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

int main(){
	int ret = 0;
	ejson_obj_t *obj;
	char error[1024];
	char data[]="{\"key1\":\"value1\",\"key2\":{\"key3\":\"value3\"}}";

	if ( ejson_to_object(data, (int)strlen(data), &obj ) != 0){
		printf("ejson_to_obj return error[%s]\n", error);
		return -1;
	}

	
	return 0;
}
