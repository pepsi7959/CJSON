#ifndef __EJSON_H
#define __EJSON_H
#include "hmap.h"

#define EJSON_KEY_COLUMN_SIZE   1024

//object
// - {}
// - {members}
//
//members
// - pair
// - pair, members
//
//pair
// - string : value
//
//array
// - []
// - [elements]
//
//elements
// - value
// - value, elements
//
//valu
// - string
// - number
// - object
// - array
// - true
// - false
// - null
//

//obj->next_pair->next_child->
enum __ejson_type {
    EJSON_TYPE_STRING = 0,
    EJSON_TYPE_NUMBER,
    EJSON_TYPE_OBJECT,
    EJSON_TYPE_ARRAY,
    EJSON_TYPE_TRUE,
    EJSON_TYPE_FALSE,
    EJSON_TYPE_NULL
};

typedef struct __ejson_obj_t{
    struct __ejson_obj_t *next;
    struct __ejson_obj_t *prev;
    struct __ejson_obj_t *child;
    struct __ejson_obj_t *head;
    
    struct __ejson_data_t *data;
    HMAP_DB *map;
    
    int type;
    char object[1024];


}ejson_obj_t;

typedef struct __ejson_data_t{
    struct __ejson_data_t *next;
    struct __ejson_data_t *prev;
    
    int type;
    char key[1024];
    char value[1024];
    void *values;
}ejson_data_t;


typedef struct __ejson_callback{
    int (*object_create)();
    int (*array_create)();
    int (*string_add)(ejson_obj_t *obj, char *key, char *value);
    int (*number_add)(ejson_obj_t *obj, char *key, int value);
    int (*boolean_add)(ejson_obj_t *obj, char *key, int boolean);
}ejson_callback;

int ejson_to_string(char **data,int len, ejson_obj_t *out);
int ejson_to_object(const char *data, int len, ejson_obj_t **out);
int ejson_to_object_get(const char *data, int len, ejson_obj_t **out, ejson_callback *json_cb);
int ejson_print(ejson_obj_t *obj);
int ejson_destroy(ejson_obj_t *obj);

#endif
