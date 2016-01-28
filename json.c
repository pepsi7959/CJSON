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
/*
int read_str(const char *d, int l, ...){
//
return 0;
}
*/

#define TRIM_SPACE(data,len,i) while(i< len && (data[i]==' ' || data[i] == '\n'))i++;

#define READ_KEY(data,len,i,ch,out)						  \
	do{													  \
		int bi = 0;										  \
		TRIM_SPACE(data,len,i)							  \
		if (data[i++] == '"'){							  \
			do{											  \
				out[bi++] = data[i++];					  \
			}while(i < len && data[i] != '"');			  \
			out[bi] = 0;								  \
			i++;										  \
		}else{											  \
			printf("Invalid format key");				  \
			break;										  \
		}												  \
	}while(0);
#define READ_VALUE(data,len,i,ch,out) READ_KEY(data,len,i,ch,out)
int ejson_to_object(const char *data, int len, ejson_obj_t **out){
	printf("data: %s\n", data);
	int i = 0;
	char buff[1024];
	while(i < len){
		if(data[i++] == '{'){
			ejson_obj_t *n_obj = new_obj();
			if(n_obj == NULL){
				return -1;
			}
			
			READ_KEY(data, len, i,'"', buff);
			printf("key : %s\n", buff);
			TRIM_SPACE(data,len,i);
			printf("delim: %c\n", data[i]);
			
			i++;
			TRIM_SPACE(data,len,i);

				
			if(data[i] == '{' ){						//members
			}else if(data[i] == '"' ){					//string
				READ_VALUE(data,len,i,'"',buff);
				printf("value : %s\n", buff);
				if( data[i] == ',' ){
					i++;
					continue;
				}
			}else if(data[i] == '[' ){					//array
			}else if(data[i] == 't' ){					//true
			}else if(data[i] == 'f' ){					//false
			}else if(data[i] == 'n' ){					//false
					
			}else{
				printf("Invalid format value");
			}
			break;
		}
	}
	return 0;
}
