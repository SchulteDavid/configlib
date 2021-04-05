#ifndef _NODE_H
#define _NODE_H

#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <ostream>
#include <sstream>
#include <vector>
#include <iostream>
#include <any>

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

    virtual const std::type_info & getTypeId() = 0;

    virtual size_t getElementCount() {
      return 0;
    }

    template <typename T> const T & getElement(size_t i){};

    virtual void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) = 0;

  protected:

  private:



  };

  template <typename T> class Node : public NodeBase {

  public:

    Node(size_t elemCount, const T * data) {

      this->data = std::vector<std::any>(elemCount);

      for (unsigned int i = 0; i < elemCount; ++i) {
	this->data[i] = data[i];
      }

    }

    Node(const std::vector<T> data) : Node(data.size(), data.data()) {

    }

    virtual ~Node() {

    }

    T operator[] (size_t i) {
      return std::any_cast<T>(data[i]);
    }

    const T getElement(size_t i) {
      return std::any_cast<T>(data[i]);
    }

    std::vector<std::any> & getRawData() {
      return data;
    }

    std::string getTypeName() {
      return getDemangledTypename(typeid(T));
    }

    size_t getElementCount() {
      return data.size();
    }

    const std::type_info & getTypeId() {
      return typeid(T);
    }

    std::string getValueString() {
      std::stringstream stream;
      stream << "[";
      for (unsigned int i = 0; i < data.size()-1; ++i) {
	stream << std::any_cast<T>(data[i]) << ", ";
      }
      stream << std::any_cast<T>(data[data.size()-1]) << "]";
      return stream.str();
    }

    void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) {

      for (int i = 0; i < indent; ++i)
	stream << "  ";

      stream << getFormatedTypename(typeid(T)) << " " << name << " = ";

      if (data.size() == 1) {

	stream << std::any_cast<T>(data[0]);

      } else {

	stream << "[";

	for (unsigned int i = 0; i < data.size()-1; ++i) {

	  stream << std::any_cast<T>(data[i]) << ", ";

	}

	stream << std::any_cast<T>(data[data.size()-1]);

	stream << "]";

      }

    }

    std::vector<T> getAsVector() {
      std::vector<T> res(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
	res[i] = std::any_cast<T>(data[i]);
      }
      return res;
    }

  protected:

    std::vector<std::any> data;

  };

  class NodeCompound;

  template <> class Node<std::shared_ptr<NodeCompound>> : public NodeBase {

  public:

    Node(size_t elemCount, const std::shared_ptr<NodeCompound> * data) {

      this->data = std::vector<std::shared_ptr<NodeCompound>>(elemCount);

      for (unsigned int i = 0; i < elemCount; ++i) {
	this->data[i] = data[i];
      }

    }

    virtual ~Node() {

    }

    std::shared_ptr<NodeCompound> & operator[] (size_t i) {
      return data[i];
    }

    const std::shared_ptr<NodeCompound> & getElement(size_t i) {
      return data[i];
    }

    std::vector<std::shared_ptr<NodeCompound>> & getRawData() {
      return data;
    }

    std::string getTypeName() {
      return getDemangledTypename(typeid(std::shared_ptr<NodeCompound>));
    }

    size_t getElementCount() {
      return data.size();
    }

    std::string getValueString() {
      std::stringstream stream;
      stream << "[";
      for (unsigned int i = 0; i < data.size()-1; ++i) {
	stream << data[i] << ", ";
      }
      stream << data[data.size()-1] << "]";
      return stream.str();
    }

    void saveToFile(std::ostream & stream, std::string name, bool printName = true, int indent = 0) {

      for (int i = 0; i < indent; ++i)
	stream << "  ";

      stream << "comp " << name << " = ";

      stream << "[" << std::endl;

      for (unsigned int i = 0; i < data.size()-1; ++i) {

	std::dynamic_pointer_cast<NodeBase>(data[i])->saveToFile(stream, "", false, indent+1);
	stream << ", " << std::endl;

      }

      std::dynamic_pointer_cast<NodeBase>(data[data.size()-1])->saveToFile(stream, "", false, indent+1);

      stream << std::endl;
      for (int i = 0; i < indent; ++i)
	stream << "  ";
      stream << "]";


    }

    const std::type_info & getTypeId() {
      return typeid(void);
    }

  protected:

    std::vector<std::shared_ptr<NodeCompound>> data;

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

    void addChild(std::string name, const std::shared_ptr<config::NodeBase> & node)  {
      this->children[name] = node;
    }

    template <typename T> void add(std::string name, const T & data) {
      std::cout << "Using template to attach " << name << ": " << data << std::endl;
      children[name] = std::make_shared<config::Node<T>>(1, &data);
    }

    template <typename T> void add(std::string name, const std::vector<T> & data) {
      children[name] = std::make_shared<config::Node<T>>(data);
    }

    std::shared_ptr<NodeBase> getChild(std::string name) {
      if (this->children.find(name) == children.end()) {
	throw std::runtime_error(std::string("No such child Node '").append(name).append("'"));
      }
      return children[name];
    }

    template <typename T> const T & get(std::string name) {
      if (this->children.find(name) == children.end())
	throw std::runtime_error(std::string("No such child Node '").append(name).append("'"));
      std::shared_ptr<Node<T>> node = std::dynamic_pointer_cast<Node<T>>(children[name]);
      if (!node) {
	std::string msg("Child Node '");
	msg.append(name).append("' has wrong type: ");
	msg.append(children[name]->getTypeName());
	msg.append(" but requested was: ");
	msg.append(getDemangledTypename(typeid(T)));

	throw std::runtime_error(msg);
      }

      return node->getElement(0);	
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

    const std::type_info & getTypeId() {
      return typeid(void);
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

    const std::type_info & getTypeId() {
      return typeid(char);
    }

  protected:

    size_t elementCount;
    char * data;

  };
  
}

std::ostream & operator<< (std::ostream & stream, std::shared_ptr<config::NodeBase> node);
std::ostream & operator<< (std::ostream & stream, std::shared_ptr<config::NodeCompound> node);

#endif
