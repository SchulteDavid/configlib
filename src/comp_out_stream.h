#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#include <iostream>

class comp_out_stream {

public:
  comp_out_stream() {};
  comp_out_stream(const comp_out_stream & stream);
  virtual ~comp_out_stream() {};

  
  void push(uint8_t e);
  std::vector<uint8_t> getCompressedData();

  void absorb(const comp_out_stream & stream);

  std::vector<uint8_t> getRawData();

private:
  std::vector<uint8_t> data;
  
};

template <typename T> comp_out_stream & operator << (comp_out_stream & stream, T t) {
  
  for (size_t i = 0; i < sizeof(T); ++i)
    stream.push(((uint8_t *) &t)[i]);
  
  return stream;
    
}

template <> comp_out_stream & operator << <const char *> (comp_out_stream & stream, const char * str);
template <> comp_out_stream & operator << <char *> (comp_out_stream & stream, char * str);
template <> comp_out_stream & operator << <const std::string &> (comp_out_stream & stream, const std::string & str);
template <> comp_out_stream & operator << <std::string &> (comp_out_stream & stream, std::string & str);
template <> comp_out_stream & operator << <std::string> (comp_out_stream & stream, std::string str);
template <> comp_out_stream & operator << <const comp_out_stream &> (comp_out_stream & stream, const comp_out_stream & str);
