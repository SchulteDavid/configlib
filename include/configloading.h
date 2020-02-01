#ifndef CONFIGLOADING_H_INCLUDED
#define CONFIGLOADING_H_INCLUDED

#include "node.h"

namespace config {

std::shared_ptr<NodeCompound> parseFile(std::string filename);

};

#endif // CONFIGLOADING_H_INCLUDED
