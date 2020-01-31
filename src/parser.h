#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

typedef struct node_t node_t;
typedef struct node_list_t node_list_t;

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

std::shared_ptr<NodeCompound> parseFile(std::string fname);

extern "C" {

#endif // __cplusplus

typedef enum value_type_e {

    TYPE_INT32,
    TYPE_INT64,
    TYPE_FLOAT32,
    TYPE_FLOAT64,
    TYPE_STRING,
    TYPE_COMPOUND

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

node_t * buildNode(VALUE_TYPE_E type, char * name, value_t value);
node_list_t * nodeListCreate(node_t * elem);
node_list_t * nodeListAppend(node_list_t * ls, node_t * elem);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PARSER_H_INCLUDED
