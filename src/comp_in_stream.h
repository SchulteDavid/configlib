#pragma once

#include <vector>
#include <string>
#include <stdint.h>

class comp_in_stream {

 public:
  comp_in_stream(std::vector<uint8_t> compData);
  virtual ~comp_in_stream();

  template <typename T> T getNextAs() {

    T * t = (T *) (data.data() + index);
    index += sizeof(T);
    return *t;
    
  }

private:
  std::vector<uint8_t> data;
  size_t index;
  
};

template <typename T> inline comp_in_stream & operator >> (comp_in_stream & stream, T & elem) {
  elem = stream.getNextAs<T>();
  return stream;
}

template <> inline comp_in_stream & operator >> <std::string> (comp_in_stream & stream, std::string & str) {

  str = "";
  while (char c = stream.getNextAs<char>()) {
    str += c;
  }

  return stream;
  
}
