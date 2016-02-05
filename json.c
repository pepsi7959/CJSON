#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "json.h"
#include "hmap.h"
#define JSON_LIST_APPEND(_first,_item)                                      \
{                                                                           \
    if ((_first) == NULL)                                                   \
    {                                                                       \
        (_first) = (_item)->prev = (_item)->next = (_item);                 \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        (_item)->prev = (_first)->prev;                                     \
        (_item)->next = (_first);                                           \
        (_first)->prev->next = (_item);                                     \
        (_first)->prev = (_item);                                           \
    }                                                                       \
}
#define JSON_LIST_REMOVE(_first,_item)                                      \
{                                                                           \
   if ((_first) == (_item))                                                 \
   {                                                                        \
        if ((_first)->next == (_first))                                     \
            (_first) = NULL;                                                \
        else                                                                \
        {                                                                   \
            (_first) = (_item)->next;                                       \
            (_item)->next->prev = (_item)->prev;                            \
            (_item)->prev->next = (_item)->next;                            \
        }                                                                   \
   }                                                                        \
   else                                                                     \
   {                                                                        \
        (_item)->next->prev = (_item)->prev;                                \
        (_item)->prev->next = (_item)->next;                                \
   }                                                                        \
   (_item)->prev = (_item)->next = NULL;                                    \
}


#define TRIM_SPACE(data,len,i) while(i< len && (data[i] == ' ' || data[i] == '\n' || data[i] == '\t' || data[i] == '\r')) i++;

#define READ_KEY(data,len,i,ch,out,bi)                    \
    do{                                                   \
        bi = 0;                                           \
        TRIM_SPACE(data,len,i)                            \
        if (data[i++] == '"'){                            \
            do{                                           \
                if(data[i] == '\\'){                      \
                    out[bi++] = data[i++];                \
                    out[bi++] = data[i++];                \
                }else{                                    \
                    out[bi++] = data[i++];                \
                }                                         \
            }while(i < len && data[i] != '"');            \
            out[bi] = 0;                                  \
            i++;                                          \
        }else{                                            \
            printf("Invalid format key");                 \
            break;                                        \
        }                                                 \
    }while(0);

#define READ_VALUE(data, len, i, ch, out, out_len) READ_KEY(data, len, i, ch, out, out_len)

static const char *ejson_type_to_string(int json_type){
    switch( json_type ){
        case EJSON_TYPE_STRING:
            return "STRING";
        case EJSON_TYPE_NUMBER:
            return "NUMBER";
        case EJSON_TYPE_OBJECT:
            return "OBJECT";
        case EJSON_TYPE_ARRAY:
            return "ARRAY";
        case EJSON_TYPE_TRUE:
            return "TRUE";
        case EJSON_TYPE_FALSE:
            return "FALSE";
        case EJSON_TYPE_NULL:
            return "NULL";
        default :
            return "UNKOWN";
    }
}

static int ejson_map_add(ejson_obj_t *json_obj, void *key, int k_len, void *data){
    if ( hmap_add_tuple_with_data(&(json_obj->map), key, k_len, data, 1) != 0 ){
        return -1;
    }
    return 0;
}

static void ejson_data_print( void *data){
    ejson_data_t *json_data  = (ejson_data_t*) data;
    printf("data[%s]\n", json_data->value);
}

static ejson_data_t * new_data(){
    
    ejson_data_t *n_data = (ejson_data_t *)calloc(1, sizeof(ejson_data_t));
    if( n_data == NULL){
        return NULL;
    }
    n_data->next     = n_data->prev = NULL;
    n_data->key[0]   = 0;
    n_data->value[0] = 0;
    n_data->type     = 0;
    n_data->values   = NULL;
    n_data->print    = ejson_data_print;  
    
    return n_data;
}

static ejson_obj_t * new_obj(char *obj_name){
    
    ejson_obj_t *n_obj = (ejson_obj_t *)calloc(1, sizeof(ejson_obj_t));
    if( n_obj == NULL){
        return NULL;
    }
    
    n_obj->next      = NULL;
    n_obj->prev      = NULL;
    n_obj->child     = NULL;
    n_obj->head      = NULL;
    n_obj->data      = NULL;
    n_obj->object[0] = 0;
    n_obj->type      = 0;
    strcpy(n_obj->object, obj_name);
    HMAP_DB *map = NULL;
    hmap_init(EJSON_KEY_COLUMN_SIZE, &map);
    if( map == NULL){
        free(n_obj);
        return NULL;
    }
    n_obj->map = map;
    
    return n_obj;
}

int _ejson_to_array(char *obj_name, const char *data, int len, int *index, ejson_obj_t **out, int type, int *stack_level, ejson_callback *json_cb, int is_have_child){
    
    printf("--------> Create new arrays %d\n", *stack_level);
    char buff[2048]; 
    int i = *index;
    int have_child      = 0;
    int buff_len        = 0;
    int number_of_value = 0;
    ejson_obj_t *n_obj = NULL;
    
    *stack_level = *stack_level + 1 ;
    buff[0] = 0;
    n_obj = new_obj(obj_name);
    
    if( n_obj == NULL ){
        return -1;
    }
               
    n_obj->type = EJSON_TYPE_ARRAY;
    
    //NEXT CHILD
    if( !is_have_child ){                               //is_have_child = 0
        if( *out != NULL){
            n_obj->head     = *out;
            (*out)->child   = n_obj;
            n_obj->next     = n_obj->prev = n_obj;
        }else{
            JSON_LIST_APPEND(*out, n_obj)
        }
    }else{
        JSON_LIST_APPEND((*out)->child, n_obj);//is_have_child = 1
    }

    
    while( i < len ){
        if( data[i] == '}' ){
            i++;
            *index = i;
            printf("~~~~ [[EXIT]] array by } \n");
            return 2;
        }
        else if( data[i] == ']' ){
            i++;
            *index = i;
            printf("~~~~ [[EXIT]] array by ] \n");
            return 2;
        }
        else if ( data[i] == ',' ){
            i++;
            continue;
        }
        else if ( data[i] == '{' ){
            i++;
            *index = i;
            printf("---- Create new object under array\n");
            if( !have_child ){
                _ejson_to_object("object", data, len, &i, &n_obj, 1, stack_level, json_cb, have_child);           //have_child = 0
                have_child = 1;
            }else{
                _ejson_to_object("object", data, len, &i, &n_obj, 1, stack_level, json_cb, have_child);  //have_child = 1
            }
        }
        else if ( data[i] == '"'){
            
            READ_VALUE(data, len, i, '"', buff, buff_len);
            printf("---- [value] : %s\n", buff);

            //Add to map
            char key_name[1024];
            int index_array = sprintf(key_name, "%d", number_of_value);
            
            //Add to map
            ejson_data_t *json_data = new_data();
            if( json_data == NULL ){
                return -1;
            }
            strcpy(json_data->value, buff);
            ejson_map_add(n_obj, key_name, index_array, json_data);
            number_of_value++;
            
            TRIM_SPACE(data,len,i)
            if( data[i] == ',' ){
                i++;
                *index = i;
                //continue;
            }
        }
        else{
            i++;
        }
    }
    *index = i;
    printf("~~~~ [[EXIT]] return from array\n");
    return 0;
}

int _ejson_to_object(char *obj_name,const char *data, int len, int *index, ejson_obj_t **out, int type, int *stack_level, ejson_callback *json_cb, int is_have_child){

    #define ADD_VALUE(obj, key, value, type)            \
    do{                                                 \
        if( obj != NULL ){                              \
            strcpy(obj->name, key);                     \
            strcpy(obj->value, value);                  \
            JSON_LIST_APPEND(*out, obj);        \
        }                                               \
    }while(0)
    
    printf("--------> Create new object %d\n", *stack_level);

    char value[2048];
    char key[2048];
    int key_len     = 0;
    int value_len   = 0;
    int have_child  = 0;
    int i = *index;
    ejson_obj_t *n_obj = NULL;
    
    value[0]        = 0;
    key[0]          = 0;
    *stack_level = *stack_level + 1 ;
    n_obj = new_obj(obj_name);
    
    if(n_obj == NULL){
        return -1;
    }
    
    //NEXT CHILD
    if( !is_have_child ){                                   //is_have_child = 0
        if(*out != NULL){
            n_obj->head     = *out;
            (*out)->child   = n_obj;
            n_obj->next     = n_obj->prev = n_obj;
        }else{
            JSON_LIST_APPEND(*out, n_obj);
        }
    }else{
        JSON_LIST_APPEND((*out)->child, n_obj);     //is_have_child = 1
    }
    
    while( i < len){
        //GET KEY
        value[0] = '\0';
        READ_KEY(data, len, i,'"', key, key_len);
        printf("---- [key] : %s\n", key);
        TRIM_SPACE(data,len,i);
        printf("---- [delim] = %c\n", data[i]);             //delimeter type
        i++;
        TRIM_SPACE(data,len,i);

        //GET TYPE or VALUE
        if( data[i] == ']' ){                               //end of arrays
                i++;
                *index = i;
                return 2;
        }
        else if( data[i] == '[' ){                          //array type
            i++;
            if( !have_child ){
                _ejson_to_array(key, data, len, &i, &n_obj, 1, stack_level, json_cb, have_child);
                have_child = 1;
            }else{
                _ejson_to_array(key, data, len, &i, &n_obj, 1, stack_level, json_cb, have_child);
            }
            TRIM_SPACE(data, len , i);
            if( data[i] == ',' ){
                i++;
                continue;
            }
        }
        else if( data[i] == '}' ){                      //end of object
                i++;
                *index = i;
                printf("~~~~ [[EXIT]] object by } \n");
                return 1;
        }
        else if( data[i] == '{' ){                      //members type
            i++;
            *index = i;
            if ( !have_child ){                        
                _ejson_to_object(key, data, len, &i, &n_obj, 1, stack_level, json_cb, have_child); //have_child = 0
                have_child = 1;
            }else{
                _ejson_to_object(key, data, len, &i, &n_obj, 1, stack_level, json_cb, have_child); //have_child = 1
            }
        }
        else if( data[i] == '"' ){                      //string type
            READ_VALUE(data, len, i, '"', value, value_len);
            printf("---- [value] : %s\n", value);
            
            if( json_cb != NULL && json_cb->string_add != NULL ) {
                json_cb->string_add(n_obj, key, value); 
            }

            //Add to map
            ejson_data_t *json_data = new_data();
            if( json_data == NULL ){
                return -1;
            }
            json_data->type = EJSON_TYPE_STRING;
            strcpy(json_data->value, value);
            ejson_map_add(n_obj, key, key_len, json_data);
            
            TRIM_SPACE(data,len,i)
            if( data[i] == ',' ){
                i++;
                continue;
            }else if( data[i] == '}' ){
                i++;
                *index = i;
                printf("~~~~ [[EXIT]] object value by } \n");
                return 1;
            }
        }
        else if( data[i] == 't' ){                      //true type
            i++;
            if(data[i++] == 'r' && data[i++] =='u' && data[i++] == 'e'){
                printf("~~~~ [value] : true\n");
                //Add to map
                ejson_data_t *json_data = new_data();
                if( json_data == NULL ){
                    return -1;
                }
                json_data->type = EJSON_TYPE_TRUE;
                strcpy(json_data->value, "true");
                ejson_map_add(n_obj, key, key_len, json_data);
                TRIM_SPACE(data,len,i)
                if( data[i] == ',' ){
                    i++;
                    continue;
                }else if( data[i] == '}' ){
                    i++;
                    *index = i;
                    return 1;
                }
            }else{
                printf("Invalid formet true\n");
                return -1;
            }
        }
        else if( data[i] == 'f' ){                      //false type
            i++;
            if(data[i++] == 'a' && data[i++] =='l' && data[i++] == 's' && data[i++] == 'e'){
                printf("~~~~ [value] : false\n");
                //Add to map
                ejson_data_t *json_data = new_data();
                if( json_data == NULL ){
                    return -1;
                }
                json_data->type = EJSON_TYPE_FALSE;
                strcpy(json_data->value, "false");
                ejson_map_add(n_obj, key, key_len, json_data);
                TRIM_SPACE(data,len,i)
                if( data[i] == ',' ){
                    i++;
                    continue;
                }else if( data[i] == '}' ){
                    i++;
                    *index = i;
                    return 1;
                }
            }else{
                printf("Invalid formet true\n");
                return -1;
            }
        }
        else if( data[i] == 'n' ){                      //null type
            i++;
            if(data[i++] == 'u' && data[i++] =='l' && data[i++] == 'l'){
                printf("~~~~ [value] : null\n");
                //Add to map
                ejson_data_t *json_data = new_data();
                if( json_data == NULL ){
                    return -1;
                }
                json_data->type = EJSON_TYPE_NULL;
                strcpy(json_data->value, "null");
                ejson_map_add(n_obj, key, key_len, json_data);
                TRIM_SPACE(data,len,i)
                if( data[i] == ',' ){
                    i++;
                    continue;
                }else if( data[i] == '}' ){
                    i++;
                    *index = i;
                    return 1;
                }
            }else{
                printf("Invalid formet true\n");
                return -1;
            }
        }
        else if( data[i] == ',' ){                      //new members
            i++;
        }
        else if( data[i] >= '0' && data[i] <= '9' ){    //number type
            int bi = 0;
            value[bi++] = data[i++];
            while( i < len && (data[i] >= '0' && data[i] <= '9' )){
                value[bi++] = data[i++];
            }
            value[bi] = 0;
            printf("~~~~ [value] : %s\n", value);
            //Add to map
            ejson_data_t *json_data = new_data();
            if( json_data == NULL ){
                return -1;
            }
            json_data->type = EJSON_TYPE_NUMBER;
            strcpy(json_data->value, value);
            ejson_map_add(n_obj, key, key_len, json_data);
            TRIM_SPACE(data,len,i)
            if( data[i] == ',' ){
                i++;
                continue;
            }else if( data[i] == '}' ){
                i++;
                *index = i;
                printf("---- [[EXIT]] object by } in loop number type\n");
                return 1;
            }
        }
        else{
            printf("Invalid format json\n");
            return -1;
        }
        break;
    }
    *index = i;
    return 0;
}

ejson_to_object_get(const char *data, int len, ejson_obj_t **out, ejson_callback *json_cb){
    printf("data: %s\n", data);
    int i = 0;
    int si = 0;
    char buff[2048];
    TRIM_SPACE(data, len, i)
    while(i < len){
        //CHECK TYPE
        if(data[i++] == '{'){
            printf("start decode [%d < %d]\n", i , len);
            _ejson_to_object("root", data, len, &i, out, 1, &si, json_cb, 0);
        }
    }
    return 0;
}

int ejson_to_object(const char *data, int len, ejson_obj_t **out){
    printf("data: %s\n", data);
    int i = 0;
    int si = 0;
    char buff[2048];
    TRIM_SPACE(data, len, i)
    while(i < len){
        //CHECK TYPE
        if(data[i++] == '{'){
            printf("start decode [%d < %d]\n", i , len);
            _ejson_to_object("root", data, len, &i, out, 1, &si, NULL, 0);
        }
    }
    return 0;
}

int ejson_print_data( HMAP_DB *map){
    TUPLE *t_list_tuple = map->list_tuple;
    ejson_data_t *p_data = NULL;
    while(t_list_tuple){
        p_data = (ejson_data_t *)(t_list_tuple->data);
        
        printf("    ====> type : %s\n", ejson_type_to_string(p_data->type) );
        printf("    ----> name : %s\n", t_list_tuple->key);
        printf("    ~~~~> value : %s\n", p_data->value);
        t_list_tuple = t_list_tuple->next;
        if(t_list_tuple == map->list_tuple)
            return 0;
    }
    return  -1;
}

int _ejson_print(ejson_obj_t *obj, int limit ,int *level){
        ejson_obj_t * member = obj;
    
    while(member){
        printf("[%d]====>obj : %s\n", *level, member->object);
        ejson_print_data( member->map );
        if( member->child ){
            *level = *level + 1;
            if( (*level) > limit)
                return -1;
            _ejson_print(member->child, limit, level);
        }
        
        member = member->next;
        if(member == obj){
            break;
        }
    }
     *level = *level - 1;
    return 0;
}

int ejson_print(ejson_obj_t *obj){
    int level = 0;
    int limit = 50;
     _ejson_print(obj, limit, &level);
}

int _ejson_destroy(ejson_obj_t *obj){
    if( obj == NULL){
        return -1;
    }
    ejson_obj_t *t_obj = obj;
    
    while(t_obj){
        if(t_obj->child){
            _ejson_destroy(t_obj->child);
        }
        JSON_LIST_REMOVE(obj,t_obj);
        hmap_destroy(&(t_obj->map));
        free(t_obj);
        t_obj = obj;
    }
    
    return 0;
}

int ejson_destroy(ejson_obj_t *obj){
    return _ejson_destroy(obj);
}