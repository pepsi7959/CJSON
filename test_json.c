#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "hmap.h"
#include "json.h"

int read_file(char *buff, int size ,const char *pathname){
    if( buff == NULL){
        return -1;
    }
    FILE *file;

    printf("filename : %s \n", pathname);
    file = fopen(pathname, "r");
    int r = fscanf(file, "%s", buff);
    fclose(file);
    return 0;
}

int object_create(){
    
return 0;
}

int array_create(){
    return 0;    
}

int string_add(){
    return 0;
}

int number_add(){
    return 0;
}

int boolean_add(){
    return 0;
}


int main(int argc, char *argv[]){
    int ret = 0;
    ejson_obj_t *obj = NULL;
    ejson_callback json_cb;
    
    char error[1024];
    char data[2048];
    if( argc == 2){
        read_file(data, 2048, argv[1]);
    }else{
        sprintf(data, "{\"key1\":\"value1\",\"key1.2\":[\"v1\",\"v2\",{\"A\":\"AV\"}],\"key2\":{\"key3\":\"value3\"}}");
    }

    if ( ejson_to_object(data, (int)strlen(data), &obj ) != 0){
        printf("ejson_to_obj return error[%s]\n", error);
        return 0;
    }
    
    hmap_print_list(obj->map);
    TUPLE *f_tuple;
    hmap_search(obj->map, "key1", 4, &f_tuple); 
    if( f_tuple != NULL ){
        printf("find key1 :%s\n", (char *)f_tuple->data);
        
    }
    
   // Display all
   ejson_print(obj);
   ejson_destroy(obj);
/*
	getchar();
    //Set json callback
    json_cb.object_create = object_create;
    json_cb.array_create = array_create;
    json_cb.string_add = string_add;
    json_cb.number_add = number_add;
    json_cb.boolean_add = boolean_add;
    
    if ( ejson_to_object_get(data, (int)strlen(data), &obj, &json_cb) != 0){
        printf("ejson_to_obj return error[%s]\n", error);
        return 0;
    }
    
*/
    return 0;
}

