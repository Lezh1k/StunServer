#ifndef STUNSETTINGS_H
#define STUNSETTINGS_H

#include <string>

namespace Stun {

  class CStunSettings {

  private:
    static const char* SK_ADDR0;
    static const char* SK_ADDR1;
    static const char* SK_PORT0;
    static const char* SK_PORT1;
    static const char* SK_LOG_FILES_DIR;

    std::string m_addr0, m_addr1;
    unsigned short m_port0, m_port1;
    std::string m_logFileDir;

    void ParseLine(std::string &line);

    CStunSettings();

  public:
    CStunSettings(const char* appDir);
    ~CStunSettings(void);

    int ReadFromFile(const char* fileName);

    const char* Addr0(void) const {return m_addr0.c_str();}
    const char* Addr1(void) const {return m_addr1.c_str();}
    const char* LogFileDir(void) const {return m_logFileDir.c_str();}

    unsigned short Port0(void) const {return m_port0;}
    unsigned short Port1(void) const {return m_port1;}
  };

}
#endif // STUNSETTINGS_H
