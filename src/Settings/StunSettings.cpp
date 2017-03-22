#include <algorithm>
#include <math.h>
#include "Commons/Commons.h"
#include "Commons/IfstreamWrapper.h"
#include "Settings/StunSettings.h"

const char* Stun::CStunSettings::SK_ADDR0 = "addr0";
const char* Stun::CStunSettings::SK_ADDR1 = "addr1";
const char* Stun::CStunSettings::SK_PORT0 = "port0";
const char* Stun::CStunSettings::SK_PORT1 = "port1";
const char* Stun::CStunSettings::SK_LOG_FILES_DIR = "log_files_dir";

Stun::CStunSettings::CStunSettings(const char *appDir) :
  m_addr0(""),
  m_addr1(""),
  m_port0(CCommons::STUN_PORT),
  m_port1(CCommons::STUN_PORT2),
  m_logFileDir(appDir){
}

Stun::CStunSettings::~CStunSettings() {
}
//////////////////////////////////////////////////////////////////////////

void Stun::CStunSettings::ParseLine(std::string& line) {
  CCommons::trim(line);
  if (line[0] == '[' || line[0] == '#')
    return; //it's commetn or section beginning.

  int indexOf = line.find_first_of("=");

  if (indexOf == -1)
    return;

  std::string key = line.substr(0, indexOf);
  std::string value = line.substr(indexOf+1, line.size());

  if (key.compare(SK_ADDR0) == 0)
    m_addr0 = value;
  else if (key.compare(SK_ADDR1) == 0)
    m_addr1 = value;
  else if (key.compare(SK_PORT0) == 0)
    m_port0 = CCommons::str_to_us(value, m_port0) ? m_port0 : 3478;
  else if (key.compare(SK_PORT1) == 0)
    m_port1 = CCommons::str_to_us(value, m_port1) ? m_port1 : 5349;
  else if (key.compare(SK_LOG_FILES_DIR) == 0)
    m_logFileDir = value;
  else
    return;
}
//////////////////////////////////////////////////////////////////////////

int Stun::CStunSettings::ReadFromFile(const char *fileName) {
  CIfstreamWrapper wrapper(fileName);
  if (!wrapper.IsOpen())
    return -1;

  std::string tmp;
  while (true) {
    tmp = wrapper.GetLine();
    ParseLine(tmp);
    if (wrapper.IsEof())
      break;
  }
  return 0;
}
//////////////////////////////////////////////////////////////////////////
