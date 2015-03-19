#include "Commons.h"
#include <memory.h>
#include <time.h>
#include <arpa/inet.h>
#include <math.h>

void Stun::CCommons::SwapBytes( byte_t* b1, byte_t* b2 ) {
  byte_t tmp = *b1;
  *b1 = *b2;
  *b2 = tmp;
}

//////////////////////////////////////////////////////////////////////////

dword_t Stun::CCommons::SwapDword(dword_t val) {
  register dword_t res, arg;
  arg = val;
  asm ("bswap %0" : "=r" (res) : "0" (arg));
  return res;
}

word_t Stun::CCommons::SwapWord(word_t val){
  register word_t res, arg;
  arg = val;
  asm ("ror %0, 8" : "=r" (res) : "0" (arg) : "cc");
  return res;
}
//////////////////////////////////////////////////////////////////////////

word_t Stun::CCommons::GetWordFromNetStream(const byte_t* stream ) {
  return ((word_t)stream[0] << 8) | stream[1];
}
//////////////////////////////////////////////////////////////////////////

void Stun::CCommons::SetWordToNetStream( byte_t* stream, word_t val ) {  
  stream[0] = (byte_t)((val & 0xff00) >> 8);
  stream[1] = (byte_t)(val & 0x00ff);
}
//////////////////////////////////////////////////////////////////////////

dword_t Stun::CCommons::GetDWordFromNetStream(const byte_t* stream ) {
  dword_t result = ntohl(*((dword_t*)stream));
  return result;
}
//////////////////////////////////////////////////////////////////////////

void Stun::CCommons::SetDWordToNetStream( byte_t* stream, dword_t val ) {
  *((dword_t*)stream) = htonl(val);
}
//////////////////////////////////////////////////////////////////////////

static const char* date_time_format = "%d.%m.%Y %H:%M:%S";
static char date_time_str_buffer[20];
char* Stun::CCommons::CurrentDateTimeString( void ) {
  time_t ct = time(NULL); //system now
  tm now;
  localtime_r(&ct, &now);
  strftime(date_time_str_buffer, 20, date_time_format, &now);
  return date_time_str_buffer;
}

//////////////////////////////////////////////////////////////////////////

static const char* date_format = "%d.%m.%Y.txt";
static char date_str_buffer[15];
char* Stun::CCommons::CurrentDateFileNameString( void ) {
  time_t ct = time(NULL); //system now
  tm now;
  localtime_r(&ct, &now);
  strftime(date_str_buffer, 15, date_format, &now);
  return date_str_buffer;
}
//////////////////////////////////////////////////////////////////////////

std::string &Stun::CCommons::ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
std::string &Stun::CCommons::rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
std::string &Stun::CCommons::trim(std::string &s) {
  return ltrim(rtrim(s));
}
//////////////////////////////////////////////////////////////////////////

bool Stun::CCommons::str_to_us(std::string& s, unsigned short& val) {
  std::string::iterator iter =
      std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isdigit)));
  if (iter != s.end() || s.size() > 5)
    return false;

  val = 0;
  const int len = s.size() - 1;

  for (int i = len; i >= 0; --i) {
    int tmp = (s[i] - '0') *  pow(10, (len - i));
    val += tmp;
  }

  return true;
}
//////////////////////////////////////////////////////////////////////////
