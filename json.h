#ifndef __EJSON_H
#define __EJSON_H

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

typedef struct __ejson_obj_t{
	struct __ejson_obj_t *next_pair;
	struct __ejson_obj_t *prev_pair;

	int type;
	char name[1024];
	char value[1024];
	struct __ejson_obj_t *next_child;
	struct __ejson_obj_t *prev_child;

}ejson_obj_t;

int ejson_to_string(char **data,int len, ejson_obj_t *out);
int ejson_to_object(const char *data, int len, ejson_obj_t **out);

#endif
