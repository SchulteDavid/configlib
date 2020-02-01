#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

typedef struct node_t node_t;
typedef struct node_list_t node_list_t;
typedef struct value_list_t value_list_t;

typedef enum value_type_e {

    TYPE_INT32,
    TYPE_INT64,
    TYPE_FLOAT32,
    TYPE_FLOAT64,
    TYPE_STRING,
    TYPE_COMPOUND,
    TYPE_LIST

} VALUE_TYPE_E ;

typedef struct value_t {

    VALUE_TYPE_E type;
    union {
        double d;
        long i;
        char * str;
        node_list_t * ls;
    } v;

} value_t;

#ifdef __cplusplus

#include <memory>
#include <string>
#include <vector>
#include <node.h>

struct node_t {

    std::string name;
    std::shared_ptr<NodeBase> n;

};

struct node_list_t {

    std::vector<node_t> nodes;

};

struct value_list_t {

    std::vector<value_t> values;

};

std::shared_ptr<NodeCompound> parseFile(std::string fname);

extern "C" {

#endif // __cplusplus

node_t * buildNode(VALUE_TYPE_E type, char * name, value_t value);
node_t * buildNodeList(VALUE_TYPE_E type, char * name, value_list_t * values);
node_list_t * nodeListCreate(node_t * elem);
node_list_t * nodeListAppend(node_list_t * ls, node_t * elem);

value_list_t * valueListCreate(value_t v);
value_list_t * valueListAppend(value_list_t * ls, value_t v);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PARSER_H_INCLUDED
