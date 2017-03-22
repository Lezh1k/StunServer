#ifndef IFSTREAM_WRAPPER_H
#define IFSTREAM_WRAPPER_H

#include <iostream>
#include <fstream>
#include <string>

typedef unsigned char uint8_t;

class CIfstreamWrapper {
private:
  std::ifstream m_ifStream;
  static const uint8_t m_buffSize = 0xff;
  char m_buffer[m_buffSize];

  CIfstreamWrapper(void);
  CIfstreamWrapper(const CIfstreamWrapper&);
  void operator=(const CIfstreamWrapper&);

public:

  CIfstreamWrapper(const char* filePath,
                   std::ios_base::openmode mode = std::ios_base::in){ m_ifStream.open(filePath, mode);}

  ~CIfstreamWrapper(void){m_ifStream.close();}

  bool IsOpen(void){return m_ifStream.is_open();}
  bool IsEof(){return m_ifStream.eof(); }

  std::string GetLine(void){
    m_ifStream.getline(m_buffer, m_buffSize);
    return std::string(m_buffer);
  }

};

#endif // IFSTREAM_WRAPPER_H
