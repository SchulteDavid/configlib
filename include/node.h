#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <ostream>
#include <sstream>

#include <cxxabi.h>

namespace config {

/** Returns the demangled name of the type given as template argument **/
std::string getDemangledTypename(const std::type_info & typeinfo);

std::string getFormatedTypename(const std::type_info & typeinfo);

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

        virtual void saveToFile(std::ostream & stream, std::string name, bool printName = true) = 0;

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

        size_t getElementCount() {
        	return elementCount;
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

        void saveToFile(std::ostream & stream, std::string name, bool printName = true) {

            stream << getFormatedTypename(typeid(T)) << " " << name << " = ";

            if (elementCount == 1) {

                stream << data.get()[0];

            } else {

                stream << "[";

                for (unsigned int i = 0; i < elementCount-1; ++i) {

                    stream << data.get()[i] << ", ";

                }

                stream << data.get()[elementCount-1];

                stream << "]";

            }

        }

    protected:

        size_t elementCount;
        std::shared_ptr<T> data;

};

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

        std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> getNodeCompound(std::string name) {

            if (this->children.find(name) == children.end()) {
                throw std::runtime_error(std::string("No such child Node '").append(name).append("'"));
            }

            std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> tmp = std::dynamic_pointer_cast<Node<std::shared_ptr<NodeCompound>>>(children[name]);
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

        void saveToFile(std::ostream & stream, std::string name, bool printName = true) {

            if (printName)
            stream << "comp " << name << " = ";

            stream << "{" << std::endl;

            for (auto it : children) {

                it.second->saveToFile(stream, it.first);
                stream << std::endl;

            }

            stream << "}";

        }

        void saveAsFile(std::ostream & stream) {

            for (auto it : children) {

                it.second->saveToFile(stream, it.first);
                stream << std::endl;

            }

        }

    private:

        std::unordered_map<std::string, std::shared_ptr<NodeBase>> children;

};

template <> class Node<std::shared_ptr<NodeCompound>> : public NodeBase {

    public:

        Node(size_t elemCount, const std::shared_ptr<NodeCompound> * data) {
            this->elementCount = elemCount;
            this->data = std::shared_ptr<std::shared_ptr<NodeCompound>>(new std::shared_ptr<NodeCompound>[elemCount], [] (std::shared_ptr<NodeCompound> * p) {delete[] p;});

            for (unsigned int i = 0; i < elementCount; ++i) {
                this->data.get()[i] = data[i];
            }

        }

        virtual ~Node() {

        }

        std::shared_ptr<NodeCompound> & operator[] (int i) {
            return data.get()[i];
        }

        const std::shared_ptr<NodeCompound> & getElement(unsigned int i) {
            return data.get()[i];
        }

        std::shared_ptr<std::shared_ptr<NodeCompound>> getRawData() {
            return data;
        }

        std::string getTypeName() {
            return getDemangledTypename(typeid(std::shared_ptr<NodeCompound>));
        }

        size_t getElementCount() {
        	return elementCount;
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

        void saveToFile(std::ostream & stream, std::string name, bool printName = true) {

            stream << "comp " << name << " = ";

            if (elementCount == 1) {

                data.get()[0]->saveToFile(stream, "", false);

            } else {

                stream << "[";

                for (unsigned int i = 0; i < elementCount-1; ++i) {

                    data.get()[i]->saveToFile(stream, "", false);
                    stream << ", " << std::endl;

                }

                data.get()[elementCount-1]->saveToFile(stream, "", false);

                stream << "]";

            }

        }

    protected:

        size_t elementCount;
        std::shared_ptr<std::shared_ptr<NodeCompound>> data;

};

template <> class Node<char> : public NodeBase {

    public:

        Node(size_t elemCount, const char * data) {
            this->elementCount = elemCount;
            this->data = new char[elementCount];

            for (unsigned int i = 0; i < elementCount; ++i) {
                this->data[i] = data[i];
            }

        }

        virtual ~Node() {

            delete data;

        }

        char & operator[] (int i) {
            return data[i];
        }

        char & getElement(unsigned int i) {
            return data[i];
        }

        const char * getRawData() {
            return data;
        }

        std::string getTypeName() {
            return getDemangledTypename(typeid(std::shared_ptr<NodeCompound>));
        }

        size_t getElementCount() {
        	return elementCount;
        }

        std::string getValueString() {
            return std::string(data);
        }

        void saveToFile(std::ostream & stream, std::string name, bool printName = true) {

            if (printName) {
                stream << "string " << name << " = ";
            }

            stream << "\"" << std::string(data) << "\"";

        }

    protected:

        size_t elementCount;
        char * data;

};

}
std::ostream & operator<< (std::ostream & stream, std::shared_ptr<config::NodeBase> node);
std::ostream & operator<< (std::ostream & stream, std::shared_ptr<config::NodeCompound> node);

#endif
