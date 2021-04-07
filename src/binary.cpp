#include <node.h>
#include <configloading.h>

#include <fstream>

#include "comp_in_stream.h"
#include "comp_out_stream.h"

namespace config {

  enum FileTypeId : uint8_t {
		   NONE,
		   INT8,
		   INT16,
		   INT32,
		   INT64,
		   FLOAT32,
		   FLOAT64,
		   STRING,
		   COMP,
		   
  };

  struct FileTypeField {

    FileTypeField() {

    }

    FileTypeField(FileTypeId type, size_t size) {

      isArray = size > 1 || !size;
      dataType = type & 0x7f;
      
    }

    
    uint8_t dataType: 7;
    uint8_t isArray: 1;
    
  };
  
  template <typename T> void saveNodeToStream(std::shared_ptr<Node<T>> node, comp_out_stream & stream);
  template <typename T> std::shared_ptr<Node<T>> loadNodeFromStream(comp_in_stream & stream, const FileTypeField & typeField);
  void saveCompoundToStream(std::shared_ptr<NodeCompound> comp, comp_out_stream & stream);
  void saveCompoundArrayToStream(std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> array, comp_out_stream & stream);
  void saveCompoundContentsToStream(std::shared_ptr<NodeCompound> comp, comp_out_stream & stream);
  void loadCompoundContentsFromStream(comp_in_stream & stream, std::shared_ptr<NodeCompound> comp);

  std::shared_ptr<NodeCompound> loadCompoundFromStream(comp_in_stream & stream);
  std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> loadCompoundArrayFromStream(comp_in_stream & stream);
  
};

using namespace config;

static FileTypeId getFileIdType(const std::type_info & typeinfo) {

  if (typeinfo == typeid(int8_t))
    return INT8;
  else if (typeinfo == typeid(int16_t))
    return INT16;
  else if (typeinfo == typeid(int32_t))
    return INT32;
  else if (typeinfo == typeid(int64_t))
    return INT64;
  else if (typeinfo == typeid(float))
    return FLOAT32;
  else if (typeinfo == typeid(double))
    return FLOAT64;
  else if (typeinfo == typeid(char))
    return STRING;

  return NONE;
  
}

static FileTypeField getTypeField(const std::type_info & typeinfo, size_t nodeSize) {

  FileTypeId id = getFileIdType(typeinfo);
  if (id == FileTypeId::NONE)
    throw std::runtime_error("Unknown type for saving");
  return FileTypeField(id, nodeSize);
  
}

template <typename T> void config::saveNodeToStream(std::shared_ptr<Node<T>> node, comp_out_stream & stream) {

  FileTypeField typeId = getTypeField(node->getTypeId(), node->getElementCount());
  stream << typeId;

  if (typeId.isArray)
    stream << (uint64_t) node->getElementCount();

  for (size_t i = 0; i < node->getElementCount(); ++i) {
    stream << node->getElement(i);
  }
  
}

void config::saveCompoundArrayToStream(std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> array, comp_out_stream & stream) {

  FileTypeField type(config::FileTypeId::COMP, array->getElementCount());
  stream << type;

  if (type.isArray)
    stream << array->getElementCount();

  for (size_t i = 0; i < array->getElementCount(); ++i) {

    saveCompoundContentsToStream(array->getElement(i), stream);
    
  }
  
}

void config::saveCompoundContentsToStream(std::shared_ptr<NodeCompound> comp, comp_out_stream & stream) {

  stream << comp->getChildMap().size();
  for (auto it : comp->getChildMap()) {

    stream << it.first;

    if (std::shared_ptr<NodeCompound> childCompound = std::dynamic_pointer_cast<NodeCompound>(it.second)) {
      
      saveCompoundToStream(childCompound, stream);
      
    } else if (std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> childCompArray = std::dynamic_pointer_cast<Node<std::shared_ptr<NodeCompound>>>(it.second)) {

      saveCompoundArrayToStream(childCompArray, stream);
      
    } else {

      FileTypeId typeId = getFileIdType(it.second->getTypeId());

      switch (typeId) {

      case INT8:
	saveNodeToStream<int8_t>(std::dynamic_pointer_cast<Node<int8_t>>(it.second), stream);
	break;

      case INT16:
	saveNodeToStream<int8_t>(std::dynamic_pointer_cast<Node<int8_t>>(it.second), stream);
	break;
	
      case INT32:
	saveNodeToStream<int32_t>(std::dynamic_pointer_cast<Node<int32_t>>(it.second), stream);
	break;

      case INT64:
	saveNodeToStream<int64_t>(std::dynamic_pointer_cast<Node<int64_t>>(it.second), stream);
	break;

      case FLOAT32:
	saveNodeToStream<float>(std::dynamic_pointer_cast<Node<float>>(it.second), stream);
	break;

      case FLOAT64:
	saveNodeToStream<double>(std::dynamic_pointer_cast<Node<double>>(it.second), stream);
	break;

      case STRING:
	saveNodeToStream<char>(std::dynamic_pointer_cast<Node<char>>(it.second), stream);
	break;

      default:
	throw std::runtime_error(std::string("Invalid type encountered when saving: ").append(std::to_string((int) typeId)));
	
      }
    }
  }
  
}

void config::saveCompoundToStream(std::shared_ptr<NodeCompound> comp, comp_out_stream &stream) {

  FileTypeField typeField(config::FileTypeId::COMP, 1);
  
  stream << typeField;

  saveCompoundContentsToStream(comp, stream);
    
}

std::vector<uint8_t> config::saveCompoundBinary(std::shared_ptr<NodeCompound> comp) {

  comp_out_stream stream;
  saveCompoundContentsToStream(comp, stream);

  size_t rawSize = stream.getRawData().size();

  std::vector<uint8_t> compData = stream.getCompressedData();

  std::cout << "Raw size: " << rawSize << " Compressed size: " << compData.size() << "( " << ((double) compData.size() / (double) rawSize) << ")" << std::endl;
  return compData;
  
}

template <typename T> std::shared_ptr<Node<T>> config::loadNodeFromStream(comp_in_stream & stream, const FileTypeField & typeField) {

  size_t elementCount = 1;
  
  if (typeField.isArray)
    stream >> elementCount;

  std::vector<T> data(elementCount);
  std::cout << "Array has " << elementCount << " elements" << std::endl;
  for (size_t i = 0; i < elementCount; ++i) {
    stream >> data[i];
  }

  return std::make_shared<Node<T>>(elementCount, data.data());
  
}

void config::loadCompoundContentsFromStream(comp_in_stream & stream, std::shared_ptr<NodeCompound> comp) {

  size_t mapSize;
  stream >> mapSize;
  
  for (size_t i = 0; i < mapSize; ++i) {

    std::string name;
    stream >> name;

    FileTypeField typeField;
    stream >> typeField;

    std::shared_ptr<NodeBase> child = nullptr;
    
    switch(typeField.dataType) {

    case INT8:
      child = loadNodeFromStream<int8_t>(stream, typeField);
      break;

    case INT16:
      child = loadNodeFromStream<int16_t>(stream, typeField);
      break;
      
    case INT32:
      child = loadNodeFromStream<int32_t>(stream, typeField);
      break;

    case INT64:
      child = loadNodeFromStream<int64_t>(stream, typeField);
      break;

    case FLOAT32:
      child = loadNodeFromStream<float>(stream, typeField);
      break;

    case FLOAT64:
      child = loadNodeFromStream<double>(stream, typeField);
      break;

    case STRING:
      child = loadNodeFromStream<char>(stream, typeField);
      break;

    case COMP:
      if (typeField.isArray)
	child = loadCompoundArrayFromStream(stream);
      else
	child = loadCompoundFromStream(stream);
      break;

    default:
      throw std::runtime_error(std::string("Unknown data type: ").append(std::to_string(typeField.dataType)));
      
    }

    comp->addChild(name, child);
    
  }
  
}

std::shared_ptr<Node<std::shared_ptr<NodeCompound>>> config::loadCompoundArrayFromStream(comp_in_stream &stream) {

  size_t length;
  stream >> length;

  std::vector<std::shared_ptr<NodeCompound>> elements(length);

  for (size_t i = 0; i < length; ++i) {

    elements[i] = std::make_shared<NodeCompound>();
    loadCompoundContentsFromStream(stream, elements[i]);
    
  }

  return std::make_shared<Node<std::shared_ptr<NodeCompound>>>(length, elements.data());
  
}

std::shared_ptr<NodeCompound> config::loadCompoundFromStream(comp_in_stream &stream) {

  std::shared_ptr<NodeCompound> node = std::make_shared<NodeCompound>();

  loadCompoundContentsFromStream(stream, node);
  
  return node;
  
}

std::shared_ptr<NodeCompound> config::loadCompoundBinary(std::vector<uint8_t> & data) {

  comp_in_stream stream(data);

  return loadCompoundFromStream(stream);

}

std::shared_ptr<NodeCompound> config::loadFileBinary(std::string fname) {

  std::ifstream inStream(fname, std::ios::binary | std::ios::ate);

  size_t size = inStream.tellg();
  inStream.seekg(4, std::ios::beg);
  
  std::vector<uint8_t> data(size);
  inStream.read((char*) data.data(), size);
  
  return loadCompoundBinary(data);
  
}
