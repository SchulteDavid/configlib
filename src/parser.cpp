#include "parser.h"

#include <node.h>
#include <iostream>
#include <string.h>

#include <configloading.h>

using namespace config;

std::string config::getDemangledTypename(const std::type_info & typeinfo) {

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

std::string config::getFormatedTypename(const std::type_info & typeinfo) {

    if (typeinfo == typeid(int32_t))
        return std::string("int32");
    else if (typeinfo == typeid(int64_t))
        return std::string("int64");
    else if (typeinfo == typeid(float))
        return std::string("float32");
    else if (typeinfo == typeid(double))
        return std::string("float64");
    else if (typeinfo == typeid(char))
        return std::string("string");


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

        case TYPE_INT64:
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

std::shared_ptr<NodeBase> buildIntNodeList(VALUE_TYPE_E type, value_list_t * values) {

    if (type == TYPE_INT32) {

        std::vector<int32_t> tmpData(values->values.size());

        for (unsigned int i = 0; i < values->values.size(); ++i) {

            if (values->values[i].type != TYPE_INT64 && values->values[i].type != TYPE_INT32) {
                throw std::runtime_error("Wrong type in int array!");
            }

            tmpData[i] = values->values[i].v.i;

        }

        delete values;

        return std::shared_ptr<NodeBase>(new Node<int32_t>(tmpData.size(), tmpData.data()));

    } else {

        std::vector<int64_t> tmpData(values->values.size());

        for (unsigned int i = 0; i < values->values.size(); ++i) {

            if (values->values[i].type != TYPE_INT64 && values->values[i].type != TYPE_INT32) {
                throw std::runtime_error("Wrong type in long array!");
            }

            tmpData[i] = values->values[i].v.i;

        }

        delete values;

        return std::shared_ptr<NodeBase>(new Node<int64_t>(tmpData.size(), tmpData.data()));

    }

}

std::shared_ptr<NodeBase> buildFloatNodeList(VALUE_TYPE_E type, value_list_t * values) {

    if (type == TYPE_FLOAT32) {

        std::vector<float> tmpData(values->values.size());

        for (unsigned int i = 0; i < values->values.size(); ++i) {

            if (values->values[i].type != TYPE_FLOAT64 && values->values[i].type != TYPE_FLOAT32 && values->values[i].type != TYPE_INT64 && values->values[i].type != TYPE_INT32) {
                throw std::runtime_error("Wrong type in float array!");
            }

            if (values->values[i].type == TYPE_INT64 || values->values[i].type == TYPE_INT32) {

                tmpData[i] = values->values[i].v.i;

            } else {

                tmpData[i] = values->values[i].v.d;

            }

        }

        delete values;

        return std::shared_ptr<NodeBase>(new Node<float>(tmpData.size(), tmpData.data()));

    } else {

        std::vector<double> tmpData(values->values.size());

        for (unsigned int i = 0; i < values->values.size(); ++i) {

            if (values->values[i].type != TYPE_FLOAT64 && values->values[i].type != TYPE_FLOAT32 && values->values[i].type != TYPE_INT64 && values->values[i].type != TYPE_INT32) {
                std::cerr << values->values[i].type << std::endl;
                throw std::runtime_error("Wrong type in double array!");
            }

            if (values->values[i].type == TYPE_INT64 || values->values[i].type == TYPE_INT32) {

                tmpData[i] = values->values[i].v.i;

            } else {

                tmpData[i] = values->values[i].v.d;

            }

        }

        delete values;

        return std::shared_ptr<NodeBase>(new Node<double>(tmpData.size(), tmpData.data()));

    }

}

std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> buildCompoundNodeList(value_list_t * values) {

    std::vector<std::shared_ptr<NodeCompound>> nodes(values->values.size());

    for (unsigned int i = 0; i < values->values.size(); ++i) {

        nodes[i] = buildCompoundNode(values->values[i]);

    }

    delete values;

    return std::shared_ptr<Node<std::shared_ptr<NodeCompound>>>(new config::Node<std::shared_ptr<config::NodeCompound>>(nodes.size(), nodes.data()));

}

node_t * buildNodeList(VALUE_TYPE_E type, char * name, value_list_t * values) {

    std::shared_ptr<NodeBase> node;
    std::string nodeName(name);

    switch (type) {

        case TYPE_INT32:
        case TYPE_INT64:
            node = buildIntNodeList(type, values);
            break;

        case TYPE_FLOAT32:
        case TYPE_FLOAT64:
            node = buildFloatNodeList(type, values);
            break;

        case TYPE_COMPOUND:
            node = buildCompoundNodeList(values);
            break;

        default:
            throw std::runtime_error("No such array supported");

    }

    node_t * res = new node_t();
    res->n = node;
    res->name = nodeName;

    free(name);

    return res;

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

std::ostream & operator<< (std::ostream & stream, std::shared_ptr<config::NodeBase> node) {

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

extern "C" void flushConfBuffer();

std::shared_ptr<NodeCompound> config::parseFile(std::string fname) {

    std::cout << "Parsing " << fname << std::endl;

    //
    confin = fopen(fname.c_str(), "r");

    if (!confin) {
      throw std::runtime_error("No Such file");
    }

    node_list_t * tmpList;
    confline = 1;

    //flushConfBuffer();

    std::cout << "Starting parser" << std::endl;

    confparse(&tmpList);

    std::cout << "conf data gotten" << std::endl;

    fclose(confin);

    //flushConfBuffer();

    std::shared_ptr<NodeCompound> node(new NodeCompound());
    for (node_t n : tmpList->nodes) {

        node->addChild(n.name, n.n);


        std::cout << n.name << std::endl;

    }

    std::cout << "Done parsing " << fname << std::endl;

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

value_list_t * valueListCreate(value_t v) {

    value_list_t * ls = new value_list_t();
    ls->values.push_back(v);

    return ls;

}

value_list_t * valueListAppend(value_list_t * ls, value_t v) {

    ls->values.push_back(v);

    return ls;

}

int confline = 1;
extern "C" char * conftext;

void onParserError(char * msg) {

  //throw std::runtime_error(msg);
  std::cerr << "Error line " << confline << " : " << std::string(msg) << " conftext: " << std::string(conftext) << std::endl;
  exit(1);

}
