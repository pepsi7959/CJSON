#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "json.h"


int ejson_to_string(char **data, int len, ejson_obj_t *out){
		
	return 0;
}

static ejson_obj_t * new_obj(){
	return (ejson_obj_t *)calloc(1, sizeof(ejson_obj_t));
}

#define JSON_MEMBERS_LIST_APPEND(_first,_item)                              \
{                                                                           \
    if ((_first) == NULL)                                                   \
    {                                                                       \
        (_first) = (_item)->prev_member = (_item)->next_member = (_item);   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        (_item)->prev_member = (_first)->prev_member;                       \
        (_item)->next_member = (_first);                                    \
        (_first)->prev_member->next_member = (_item);                       \
        (_first)->prev_member = (_item);                                    \
    }                                                                       \
}
#define JSON_MEMBERS_LIST_REMOVE(_first,_item)                              \
{                                                                           \
   if ((_first) == (_item))                                                 \
   {                                                                        \
        if ((_first)->next_member == (_first))                              \
            (_first) = NULL;                                                \
        else                                                                \
        {                                                                   \
            (_first) = (_item)->next_member;                                \
            (_item)->next_member->prev_member = (_item)->prev_member;       \
            (_item)->prev_member->next_member = (_item)->next_member;       \
        }                                                                   \
   }                                                                        \
   else                                                                     \
   {                                                                        \
        (_item)->next_member->prev_member = (_item)->prev_member;           \
        (_item)->prev_member->next_member = (_item)->next_member;           \
   }                                                                        \
   (_item)->prev_member = (_item)->next_member = NULL;                      \
}

#define JSON_CHILD_LIST_APPEND(_first,_item)                                \
{                                                                           \
    if ((_first) == NULL)                                                   \
    {                                                                       \
        (_first) = (_item)->prev_child = (_item)->next_child = (_item);     \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        (_item)->prev_child = (_first)->prev_child;                         \
        (_item)->next_child = (_first);                                     \
        (_first)->prev_child->next_child = (_item);                         \
        (_first)->prev_child = (_item);                                     \
    }                                                                       \
}
#define JSON_CHILD_LIST_REMOVE(_first,_item)                                \
{                                                                           \
   if ((_first) == (_item))                                                 \
   {                                                                        \
        if ((_first)->next_child == (_first))                               \
            (_first) = NULL;                                                \
        else                                                                \
        {                                                                   \
            (_first) = (_item)->next_child;                                 \
            (_item)->next_child->prev_child = (_item)->prev_child;          \
            (_item)->prev_child->next_child = (_item)->next_child;          \
        }                                                                   \
   }                                                                        \
   else                                                                     \
   {                                                                        \
        (_item)->next_child->prev_child = (_item)->prev_child;              \
        (_item)->prev_child->next_child = (_item)->next_child;              \
   }                                                                        \
   (_item)->prev_child = (_item)->next_child = NULL;                        \
}

#define TRIM_SPACE(data,len,i) while(i< len && (data[i] == ' ' || data[i] == '\n' || data[i] == '\t' || data[i] == '\r')) i++;

#define READ_KEY(data,len,i,ch,out)						  \
	do{													  \
		int bi = 0;										  \
		TRIM_SPACE(data,len,i)							  \
		if (data[i++] == '"'){							  \
			do{											  \
				if(data[i] == '\\'){					  \
					out[bi++] = data[i++];				  \
					out[bi++] = data[i++];				  \
				}else{									  \
					out[bi++] = data[i++];				  \
				}										  \
			}while(i < len && data[i] != '"');			  \
			out[bi] = 0;								  \
			i++;										  \
		}else{											  \
			printf("Invalid format key");				  \
			break;										  \
		}												  \
	}while(0);

#define READ_VALUE(data,len,i,ch,out) READ_KEY(data,len,i,ch,out)
int _ejson_to_array(const char *data, int len, int *index, ejson_obj_t **out, int type, int *stack_level){
    int i = *index;
	*stack_level = *stack_level + 1 ;
	printf("AAAAAAAAAAAA [[[[   %d   ]]]] AAAAAAAAAAAA\n", *stack_level);
	char buff[2048];

    while( i < len ){
        if( data[i] == '}' ){
            i++;
            *index = i;
            printf("[[EXIT]] array by } \n");
            return 2;
        }
        else if( data[i] == ']' ){
            i++;
            *index = i;
            printf("[[EXIT]] array by ] \n");
            return 2;
        }
        else if ( data[i] == ',' ){
            i++;
            continue;
        }
        else if ( data[i] == '{' ){
            i++;
            *index = i;
            printf("{{ new object }}\n");
            _ejson_to_object(data,len,&i,out,1, stack_level);
        }
        else if ( data[i] == '"'){
            READ_VALUE(data,len,i,'"',buff);
            printf("[value]==> %s\n", buff);
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
    printf("[[EXIT]] {{Array}}\n");
}

int _ejson_to_object(const char *data, int len, int *index, ejson_obj_t **out, int type, int *stack_level){
	int i = *index;
	*stack_level = *stack_level + 1 ;
	printf("@@@@@@@@@@@@@@@@ [[[[   %d   ]]]] @@@@@@@@@@@@@@\n", *stack_level);
	char buff[2048];

	while( i < len){
		//GET KEY
        buff[0] = '\0';
		READ_KEY(data, len, i,'"', buff);
		printf("[key]==> %s\n", buff);
		TRIM_SPACE(data,len,i);
		printf("%c\n", data[i]);                    //delimeter type             
	
		i++;
		TRIM_SPACE(data,len,i);

       	if( buff[0] != '\0' ){
            ejson_obj_t *n_obj = new_obj();
            if(n_obj == NULL){
                return -1;
            }
            //APPEND *out = n_obj;
        }
        
		//GET TYPE or VALUE
		if( data[i] == '{' ){						//members type
			i++;
			*index = i;
			printf("{{ New object }}\n");
			_ejson_to_object(data, len, &i, out, 1, stack_level);
		}
        else if( data[i] == '"' ){					//string type
			READ_VALUE(data,len,i,'"',buff);
			printf("[value]==> %s\n", buff);
			TRIM_SPACE(data,len,i)
			if( data[i] == ',' ){
				i++;
				continue;
			}else if( data[i] == '}' ){
				i++;
                *index = i;
                printf("[[EXIT]] object by } \n");
				return 1;
			}
		}
        else if( data[i] == ']' ){                  //end of arrays
                i++;
                *index = i;
				return 2;
		}
        else if( data[i] == '}' ){                  //end of object
                i++;
                *index = i;
                printf("[[EXIT]] by } \n");
				return 1;
		}
        else if( data[i] == '[' ){					//array type
			i++;
            _ejson_to_array(data, len, &i, out, 1, stack_level);
            TRIM_SPACE(data, len , i);
            if( data[i] == ',' ){
				i++;
				continue;
			}
		}
        else if( data[i] == 't' ){					//true type
            i++;
			if(data[i++] == 'r' && data[i++] =='u' && data[i++] == 'e'){
                printf("true\n");
				TRIM_SPACE(data,len,i)
				if( data[i] == ',' ){
					i++;
					continue;
				}else if( data[i] == '}' ){
					*index = i;
					i++;
					return 1;
				}
			}else{
				printf("Invalid formet true\n");
				return -1;
			}
		}
        else if( data[i] == 'f' ){					//false type
			i++;
			if(data[i++] == 'a' && data[i++] =='l' && data[i++] == 's' && data[i++] == 'e'){
				printf("false\n");
                TRIM_SPACE(data,len,i)
				if( data[i] == ',' ){
					i++;
					continue;
				}else if( data[i] == '}' ){
					*index = i;
					i++;
					return 1;
				}
			}else{
				printf("Invalid formet true\n");
				return -1;
			}
		}
        else if( data[i] == 'n' ){					//null type
			i++;
			if(data[i++] == 'u' && data[i++] =='l' && data[i++] == 'l'){
				printf("null\n");
				TRIM_SPACE(data,len,i)
				if( data[i] == ',' ){
					i++;
					continue;
				}else if( data[i] == '}' ){
					*index = i;
					i++;
					return 1;
				}
			}else{
				printf("Invalid formet true\n");
				return -1;
			}
		}
        else if( data[i] == ',' ){                   //new members
			i++;
		}
        else if( data[i] >= '0' && data[i] <= '9' ){ //number type
			int bi = 0;
			buff[bi++] = data[i++];
			while( i < len && (data[i] >= '0' && data[i] <= '9' )){
				buff[bi++] = data[i++];
			}
			buff[bi] = 0;
			printf("%s\n", buff);
			TRIM_SPACE(data,len,i)
			if( data[i] == ',' ){
				i++;
				continue;
			}else if( data[i] == '}' ){
                i++;
				*index = i;
                printf("[[EXIT]] object by } in loop number type\n");
				return 1;
			}
		}
        else{
			printf("Invalid format json\n");
			return -1;
		}
		break;
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
            printf("start decode\n");
			_ejson_to_object(data, len, &i, out, 1,&si);
		}
	}
	return 0;
}
