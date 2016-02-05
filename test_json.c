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

int json_object_d02_connect_get(ejson_obj_t * json_obj){
    ejson_obj_t *t_json_obj = json_obj->child;
    TUPLE *f_tuple          = NULL;
    while(t_json_obj){
       //find d02 connection
       hmap_search(t_json_obj->map, "key", 3, &f_tuple); 
        if( f_tuple != NULL ){
            printf("        - connection : %s\n", ((ejson_data_t *)(f_tuple->data))->value);
        }
        
        hmap_search(t_json_obj->map, "value", 5, &f_tuple); 
        if( f_tuple != NULL ){
            printf("        - connection : %s\n", ((ejson_data_t *)(f_tuple->data))->value);
        }
        
        t_json_obj = t_json_obj->next;
        if(t_json_obj == json_obj->child)
            break;
    }
    return 0; 
}

int json_object_d02_get(ejson_obj_t * json_obj){
    ejson_obj_t *t_json_obj = json_obj;
    TUPLE *f_tuple          = NULL;
    while(t_json_obj){
       //find d02 connection
       hmap_search(t_json_obj->map, "key", 3, &f_tuple); 
        if( f_tuple != NULL ){
            printf("    - d02 name : %s\n", ((ejson_data_t *)(f_tuple->data))->value);
            json_object_d02_connect_get(t_json_obj->child);
        }
        
        t_json_obj = t_json_obj->next;
        if(t_json_obj == json_obj)
            break;
    }
    return 0;
}

int json_object_cluster_get(ejson_obj_t * json_obj){
    ejson_obj_t *t_json_obj = json_obj;
    TUPLE *f_tuple          = NULL;
    while( t_json_obj ){
        
       //find cluster
       hmap_search(t_json_obj->map, "key", 3, &f_tuple); 
        if( f_tuple != NULL ){
            printf("cluster name : %s\n", ((ejson_data_t *)(f_tuple->data))->value);
            //find d02
            json_object_d02_get(t_json_obj->child->child);
        }
        
        t_json_obj = t_json_obj->next;
        if(t_json_obj == json_obj)
            break;
    }
    return 0;
}


int main(int argc, char *argv[]){
    int ret = 0;
    ejson_obj_t *obj = NULL;
    ejson_callback json_cb;
    
    char error[1024];
    char data[40960];
    if( argc == 2){
        read_file(data, 40960, argv[1]);
    }else{
        sprintf(data, "{\"key1\":\"value1\",\"key1.2\":[\"v1\",\"v2\",{\"A\":\"AV\"}],\"key2\":{\"key3\":\"value3\"}}");
    }

    if ( ejson_to_object(data, (int)strlen(data), &obj ) != 0){
        printf("ejson_to_obj return error[%s]\n", error);
        return 0;
    }
    
    //hmap_print_list(obj->map);
    TUPLE *f_tuple = NULL;
    
   // Display all
   ejson_print(obj);
   hmap_search(obj->child->child->child->next->map, "key", 3, &f_tuple); 
   printf("object name : %s\n", obj->object);
    if( f_tuple != NULL ){
        printf("find key :%s\n", ((ejson_data_t *)(f_tuple->data))->value);
        
    }
    
   json_object_cluster_get(obj->child->child->child);
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

