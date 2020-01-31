#include "parser.h"

#include <node.h>
#include <iostream>
#include <string.h>

std::string getDemangledTypename(const std::type_info & typeinfo) {

    size_t demangle_size = 256;
    char * demagleSpace = (char *) malloc(sizeof(char) * demangle_size);
    int status;
    char * ret = abi::__cxa_demangle(typeinfo.name(), demagleSpace, &demangle_size, &status);

    std::string str;

    if (!status) {
        str = std::string(demagleSpace);
    } else {
        str = std::string(typeinfo.name());
    }

    free(demagleSpace);

    return str;

}

std::shared_ptr<NodeBase> buildIntNode(VALUE_TYPE_E type, value_t value) {

    switch (value.type) {

        case TYPE_INT64:
            if (type == TYPE_INT32) {
                int i = value.v.i;
                return std::shared_ptr<NodeBase>(new Node<int32_t>(1, &i));
            } else {
                return std::shared_ptr<NodeBase>(new Node<int64_t>(1, &value.v.i));
            }

        default:

            throw std::runtime_error("Cannot convert to desired type!");

    }

}

std::shared_ptr<NodeBase> buildFloatNode(VALUE_TYPE_E type, value_t value) {

    switch (value.type) {

        case TYPE_FLOAT64:
            if (type == TYPE_FLOAT32) {
                float i = value.v.d;
                return std::shared_ptr<NodeBase>(new Node<float>(1, &i));
            } else {
                return std::shared_ptr<NodeBase>(new Node<double>(1, &value.v.d));
            }
            break;

        case TYPE_INT32:
            if (type == TYPE_FLOAT32) {
                float i = value.v.i;
                return std::shared_ptr<NodeBase>(new Node<float>(1, &i));
            } else {
                double d = value.v.i;
                return std::shared_ptr<NodeBase>(new Node<double>(1, &d));
            }

        default:

            throw std::runtime_error("Cannot convert to desired type!");

    }

}

std::shared_ptr<NodeBase> buildStringNode(VALUE_TYPE_E type, value_t value) {

    if (value.type != TYPE_STRING) {
        throw std::runtime_error("Cannot convert to desired type!");
    }

    std::string tmpStr(value.v.str);
    tmpStr = tmpStr.substr(1, tmpStr.length()-2);
    std::shared_ptr<Node<char>> res(new Node<char>(tmpStr.length() + 1, tmpStr.c_str()));

    free(value.v.str);

    return res;

}

std::shared_ptr<NodeCompound> buildCompoundNode(value_t value) {

    if (value.type != TYPE_COMPOUND) {
        throw std::runtime_error("Cannot convert to desired type!");
    }

    std::shared_ptr<NodeCompound> node(new NodeCompound());
    for (node_t n : value.v.ls->nodes) {

        node->addChild(n.name, n.n);

    }

    delete value.v.ls;

    return node;

}

node_t * buildNode(VALUE_TYPE_E type, char * name, value_t value) {

    std::shared_ptr<NodeBase> node;
    std::string nodeName(name);

    switch (type) {

        case TYPE_INT32:
        case TYPE_INT64:
            node = buildIntNode(type, value);
            break;

        case TYPE_FLOAT32:
        case TYPE_FLOAT64:
            node = buildFloatNode(type, value);
            break;

        case TYPE_STRING:
            node = buildStringNode(type, value);
            break;

        case TYPE_COMPOUND:
            node = buildCompoundNode(value);
            break;

    }

    node_t * res = new node_t();
    res->n = node;
    res->name = nodeName;

    free(name);

    return res;

}

std::ostream & operator<< (std::ostream & stream, std::shared_ptr<NodeBase> node) {

    return stream << node->getTypeName() << " -> " << node->getValueString();

}

std::ostream & operator<< (std::ostream & stream, std::shared_ptr<NodeCompound> node) {

    for (auto it : node->getChildMap()) {

        stream << it.first << " : " << it.second << std::endl;

    }

    return stream;

}

extern "C" int confparse(node_list_t ** ls);
extern "C" FILE * confin;

std::shared_ptr<NodeCompound> parseFile(std::string fname) {

    confin = fopen(fname.c_str(), "r");
    node_list_t * tmpList;
    confparse(&tmpList);

    fclose(confin);

    std::shared_ptr<NodeCompound> node(new NodeCompound());
    for (node_t n : tmpList->nodes) {

        node->addChild(n.name, n.n);

    }

    delete tmpList;

    return node;

}

node_list_t * nodeListCreate(node_t * elem) {

    node_list_t * ls = new node_list_t();
    ls->nodes.push_back(*elem);

    delete elem;

    return ls;

}

node_list_t * nodeListAppend(node_list_t * ls, node_t * elem) {

    ls->nodes.push_back(*elem);
    delete elem;

    return ls;

}
