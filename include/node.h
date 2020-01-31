#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <ostream>
#include <sstream>

#include <cxxabi.h>

/** Returns the demangled name of the type given as template argument **/
std::string getDemangledTypename(const std::type_info & typeinfo);

class NodeBase {

    public:
        NodeBase() {

        }

        virtual ~NodeBase() {

        }

        virtual std::string getTypeName() {
            return "NONE";
        }

        virtual std::string getValueString() {
            return "NONE";
        }

    protected:

    private:



};

template <typename T> class Node : public NodeBase {

    public:

        Node(size_t elemCount, const T * data) {
            this->elementCount = elemCount;
            this->data = std::shared_ptr<T>(new T[elemCount], [] (T * p) {delete[] p;});

            for (unsigned int i = 0; i < elementCount; ++i) {
                this->data.get()[i] = data[i];
            }

        }

        virtual ~Node() {

        }

        T & operator[] (int i) {
            return data.get()[i];
        }

        const T & getElement(unsigned int i) {
            return data.get()[i];
        }

        std::shared_ptr<T> getRawData() {
            return data;
        }

        std::string getTypeName() {
            return getDemangledTypename(typeid(T));
        }

        std::string getValueString() {
            std::stringstream stream;
            stream << "[";
            for (unsigned int i = 0; i < elementCount-1; ++i) {
                stream << data.get()[i] << ", ";
            }
            stream << data.get()[elementCount-1] << "]";
            return stream.str();
        }

    protected:

        size_t elementCount;
        std::shared_ptr<T> data;

};

std::ostream & operator<< (std::ostream & stream, std::shared_ptr<NodeBase> node);

class NodeCompound : public NodeBase {

    public:
        NodeCompound() {

        }

        virtual ~NodeCompound() {

        }

        std::shared_ptr<NodeBase> & operator[] (std::string name) {
            return this->children[name];
        }

        void addChild(std::string name, std::shared_ptr<NodeBase> node) {
            this->children[name] = node;
        }

        std::shared_ptr<NodeBase> getChild(std::string name) {
            if (this->children.find(name) == children.end()) {
                throw std::runtime_error(std::string("No such child Node '").append(name).append("'"));
            }
            return children[name];
        }

        template <typename T> std::shared_ptr<Node<T>> getNode(std::string name) {
            if (this->children.find(name) == children.end()) {
                throw std::runtime_error(std::string("No such child Node '").append(name).append("'"));
            }
            std::shared_ptr<Node<T>> tmp = std::dynamic_pointer_cast<Node<T>>(children[name]);
            if (!tmp) {

                std::string msg("Child Node '");
                msg.append(name).append("' has wrong type: ");
                msg.append(children[name]->getTypeName());
                msg.append(" but requested was: ");
                msg.append(getDemangledTypename(typeid(T)));

                throw std::runtime_error(msg);
            }
            return tmp;
        }

        std::string getTypeName() {
            return "comp";
        }

        std::unordered_map<std::string, std::shared_ptr<NodeBase>> getChildMap() {
            return children;
        }

        std::string getValueString() {
            std::stringstream stream;
            stream << "{" << std::endl;
            for (auto it : children) {
                stream << it.first << " : " << it.second << std::endl;
            }
            stream << "}";
            return stream.str();
        }

    private:

        std::unordered_map<std::string, std::shared_ptr<NodeBase>> children;

};
std::ostream & operator<< (std::ostream & stream, std::shared_ptr<NodeCompound> node);

#endif
