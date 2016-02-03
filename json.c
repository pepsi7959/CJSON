#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "json.h"


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

#define READ_KEY(data,len,i,ch,out)                       \
    do{                                                   \
        int bi = 0;                                       \
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

#define READ_VALUE(data,len,i,ch,out) READ_KEY(data,len,i,ch,out)


static ejson_data_t * new_data(){
    
    ejson_data_t *n_data = (ejson_data_t *)calloc(1, sizeof(ejson_data_t));
    if( n_data == NULL){
        return NULL;
    }
    n_data->next = n_data->prev = NULL;
    n_data->key[0] = 0;
    n_data->value[0] = 0;
    n_data->type = 0;
    
    return n_data;
}

static ejson_obj_t * new_obj(char *obj_name){
    
    ejson_obj_t *n_obj = (ejson_obj_t *)calloc(1, sizeof(ejson_obj_t));
    if( n_obj == NULL){
        return NULL;
    }
    
    n_obj->next = NULL;
    n_obj->prev = NULL;
    n_obj->child = NULL;
    n_obj->head = NULL;
    n_obj->data = NULL;
    n_obj->name[0] = 0;
    n_obj->value[0] = 0;
    n_obj->type = 0;
    strcpy(n_obj->object, obj_name);
    
    return n_obj;
}

int _ejson_to_array(char *obj_name, const char *data, int len, int *index, ejson_obj_t **out, int type, int *stack_level, ejson_callback *json_cb, int is_have_child){
    int i = *index;
    int have_child = 0;
    *stack_level = *stack_level + 1 ;
    printf("--------> Create new arrays %d\n", *stack_level);
    char buff[2048]; buff[0] = 0;
    
    ejson_obj_t *n_obj = NULL;
    n_obj = new_obj(obj_name);
    
    if( n_obj == NULL ){
        return -1;
    }
               
    n_obj->type = EJSON_TYPE_ARRAY;
    
    //NEXT CHILD
    if( !is_have_child ){                               //is_have_child = 0
        if( *out != NULL){
            n_obj->head = *out;
            (*out)->child = n_obj;
            n_obj->next = n_obj->prev = n_obj;
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
            ejson_data_t *n_data = new_data();
                if( n_data == NULL ){
                    return -1;
            }
            
            READ_VALUE(data,len,i,'"',buff);
            printf("---- [value] : %s\n", buff);

            strcpy(n_data->key, "array");
            strcpy(n_data->value, buff);
            n_data->type = EJSON_TYPE_STRING;
            JSON_LIST_APPEND(n_obj->data, n_data);
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
 
    int i = *index;
    int have_child = 0;
    *stack_level = *stack_level + 1 ;
    printf("--------> Create new object %d\n", *stack_level);
    char value[2048];value[0] = 0;
    char key[2048];key[0] = 0;
    
    ejson_obj_t *n_obj = NULL;
    
    n_obj = new_obj(obj_name);
    if(n_obj == NULL){
        return -1;
    }
    
    //NEXT CHILD
    if( !is_have_child ){                                   //is_have_child = 0
        if(*out != NULL){
            n_obj->head = *out;
            (*out)->child = n_obj;
            n_obj->next = n_obj->prev = n_obj;
        }else{
            JSON_LIST_APPEND(*out, n_obj);
        }
    }else{
        JSON_LIST_APPEND((*out)->child, n_obj);     //is_have_child = 1
    }
    
    while( i < len){
        //GET KEY
        value[0] = '\0';
        READ_KEY(data, len, i,'"', key);
        printf("---- [key] : %s\n", key);
        TRIM_SPACE(data,len,i);
        printf("---- [delim] = %c\n", data[i]);             //delimeter type
        
        ejson_data_t *n_data = new_data();
        if( n_data == NULL ){
            return -1;
        }
        
        strcpy(n_data->key, key);
        
        i++;
        TRIM_SPACE(data,len,i);

         
        if( n_obj != NULL ){                  
            strcpy(n_obj->name, key);             
        }  
            
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
            READ_VALUE(data,len,i,'"', value);
            printf("---- [value] : %s\n", value);
            
            if( json_cb != NULL && json_cb->string_add != NULL ) {
                json_cb->string_add(n_obj, key, value); 
            }
            
            strcpy(n_obj->value, value);   
            strcpy(n_data->value, value);
            n_data->type = EJSON_TYPE_STRING;
            JSON_LIST_APPEND(n_obj->data, n_data);
            //ADD_VALUE(n_obj, key, buff, 0);
            
            
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
                strcpy(n_data->value, "true");
                n_data->type = EJSON_TYPE_TRUE;
                JSON_LIST_APPEND(n_obj->data, n_data);
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
                strcpy(n_data->value, "false");
                n_data->type = EJSON_TYPE_FALSE;
                JSON_LIST_APPEND(n_obj->data, n_data);
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
                strcpy(n_data->value, "null");
                n_data->type = EJSON_TYPE_NULL;
                JSON_LIST_APPEND(n_obj->data, n_data);
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
            strcpy(n_data->value, value);
            n_data->type = EJSON_TYPE_NUMBER;
            JSON_LIST_APPEND(n_obj->data, n_data);
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

int ejson_print_data( ejson_data_t *data ){
    ejson_data_t *p_data = data;
    while(p_data){
        printf("    ----> type : %d\n", p_data->type);
        printf("    ----> name : %s\n", p_data->key);
        printf("    ~~~~> value : %s\n", p_data->value);
        p_data = p_data->next;
        if(p_data == data)
            return 0;
    }
    return  -1;
}

int _ejson_print(ejson_obj_t *obj, int limit ,int *level){
        ejson_obj_t * member = obj;
    
    while(member){
        printf("[%d]====>obj : %s\n", *level, member->object);
        ejson_print_data( member->data );
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
