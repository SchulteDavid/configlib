#ifndef CONFIGLOADING_H_INCLUDED
#define CONFIGLOADING_H_INCLUDED

#include "node.h"

namespace config {

  std::shared_ptr<NodeCompound> load(std::string filename);
  void save(std::shared_ptr<NodeCompound> node, std::ostream & file, bool binary = true);

  std::vector<uint8_t> saveCompoundBinary(std::shared_ptr<NodeCompound> comp);
  std::shared_ptr<NodeCompound> loadCompoundBinary(std::vector<uint8_t> & data);

  std::shared_ptr<NodeCompound> loadFileBinary(std::string fname);
  
};

#endif // CONFIGLOADING_H_INCLUDED
