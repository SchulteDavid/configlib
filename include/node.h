#ifndef _NODE_H
#define _NODE_H

#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <ostream>
#include <sstream>
#include <vector>
#include <iostream>

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

    virtual void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) = 0;

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

    T & operator[] (size_t i) {
      return data.get()[i];
    }

    const T & getElement(size_t i) {
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

    void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) {

      for (int i = 0; i < indent; ++i)
	stream << "  ";

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

  class NodeCompound;

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

    std::shared_ptr<NodeCompound> & operator[] (size_t i) {
      return data.get()[i];
    }

    const std::shared_ptr<NodeCompound> & getElement(size_t i) {
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

    void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) {

      for (int i = 0; i < indent; ++i)
	stream << "  ";

      stream << "comp " << name << " = ";

      /*if (elementCount == 1) {

	std::dynamic_pointer_cast<NodeBase>(data.get()[0])->saveToFile(stream, "", false, indent);

	} else {*/

      stream << "[" << std::endl;

      for (unsigned int i = 0; i < elementCount-1; ++i) {

	std::dynamic_pointer_cast<NodeBase>(data.get()[i])->saveToFile(stream, "", false, indent+1);
	stream << ", " << std::endl;

      }

      std::dynamic_pointer_cast<NodeBase>(data.get()[elementCount-1])->saveToFile(stream, "", false, indent+1);

      stream << std::endl;
      for (int i = 0; i < indent; ++i)
	stream << "  ";
      stream << "]";

      //}

    }

  protected:

    size_t elementCount;
    std::shared_ptr<std::shared_ptr<NodeCompound>> data;

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

    bool hasChild(std::string name) {
      return this->children.find(name) != this->children.end();
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

    std::shared_ptr<NodeCompound> getNodeCompound(std::string name) {

      if (this->children.find(name) == children.end()) {
	throw std::runtime_error(std::string("No such child Node '").append(name).append("'"));
      }

      std::shared_ptr<NodeCompound> tmp = std::dynamic_pointer_cast<NodeCompound>(children[name]);
      std::cout << children[name]->getTypeName() << std::endl;
      /*if (!tmp) {
	throw std::runtime_error("Error while getting child compound!");
	}*/
      return tmp;

    }

    std::shared_ptr<NodeCompound> getSubcompound(std::string name) {

      if (this->children.find(name) == children.end()) {
	throw std::runtime_error(std::string("No such child Node '").append(name).append("'"));
      }

      std::cout << "Demangled name " << getDemangledTypename(typeid(children[name])) << std::endl;

      Node<std::shared_ptr<NodeCompound>> * node = (Node<std::shared_ptr<NodeCompound>> *) children[name].get();

      std::cout << "Node has " << node->getElementCount() << " elements" << std::endl;

      return node->getElement(0);

    }

    std::string getTypeName() {
      return getDemangledTypename(typeid(*this));
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

    void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) {

      /*for (int i = 0; i < indent; ++i)
	stream << "\t";*/

      
      for (int i = 0; i < indent; ++i)
	  stream << "  ";
      
      if(printName)
        stream << "comp " << name << " = ";
      

      stream << "{" << std::endl;

      for (auto it : children) {

	it.second->saveToFile(stream, it.first, true, indent+1);
	stream << std::endl;

      }

      //if (printName) {
      for (int i = 0; i < indent; ++i)
	stream << "  ";
      //}
      stream << "}";

    }

    void saveAsFile(std::ostream & stream) {

      for (auto it : children) {

	it.second->saveToFile(stream, it.first, true, 0);
	stream << std::endl;

      }

    }

  private:

    std::unordered_map<std::string, std::shared_ptr<NodeBase>> children;

  };

  template <> class Node<char> : public NodeBase {

  public:

    Node(size_t elemCount, const char * data) {
      this->elementCount = elemCount;
      this->data = new char[elementCount+1];

      for (unsigned int i = 0; i < elementCount; ++i) {
	this->data[i] = data[i];
      }

      this->data[elemCount] = 0;

    }

    virtual ~Node() {

      delete[] data;

    }

    char & operator[] (size_t i) {
      return data[i];
    }

    char & getElement(unsigned int i) {
      return data[i];
    }

    const char * getRawData() {
      return data;
    }

    std::string getTypeName() {
      return getDemangledTypename(typeid(char));
    }

    size_t getElementCount() {
      return elementCount;
    }

    std::string getValueString() {
      return std::string(data);
    }

    void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) {

      for (int i = 0; i < indent; ++i)
	stream << "  ";
      
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
