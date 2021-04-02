#include "comp_in_stream.h"

#include <iostream>
#include <zlib.h>

comp_in_stream::comp_in_stream(std::vector<uint8_t> compData) {

  size_t decompSize = *((size_t *) compData.data());
  
  data.resize(decompSize);
  
  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  stream.avail_in = compData.size() - sizeof(size_t);
  stream.next_in = compData.data() + sizeof(size_t);
  stream.avail_out = decompSize;
  stream.next_out = data.data();

  inflateInit(&stream);
  inflate(&stream, Z_NO_FLUSH);
  inflateEnd(&stream);

  this->index = 0;
  
}

comp_in_stream::~comp_in_stream() {

}
