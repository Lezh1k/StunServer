#ifndef COMMONS_H
#define COMMONS_H

#define UNUSED_ARG(x) ((void)(x))

#include <stdint.h>
#include <string>
#include <algorithm>

namespace Stun {

  class CCommons {
  public:
    //consts
    static const int STUN_PORT = 3478;
    static const int STUN_PORT2 = 5349;
    static const int DEFAULT_BUF_SIZE = 1024;
    static const int MAGIC_COOKIE = 0x2112A442;
    //methods

    static void SwapBytes(int8_t* b1, int8_t* b2);

    static int16_t SwapWord(int16_t val);
    static int32_t SwapDword(int32_t val);

    static int16_t HToNs(int16_t val) {return SwapWord(val);}
    static int16_t NToHs(int16_t val) {return SwapWord(val);}

    static int32_t HToNl(int32_t val) { return SwapDword(val);}
    static int32_t NToHl(int32_t val) { return SwapDword(val);}

    static int16_t GetWordFromNetStream(const int8_t* stream);
    static void   SetWordToNetStream(int8_t* stream, int16_t val);
    static int32_t GetDWordFromNetStream(const int8_t* stream);
    static void  SetDWordToNetStream(int8_t* stream, int32_t val);

    static char* CurrentDateFileNameString(void);
    static char* CurrentDateTimeString(void);

    static std::string &ltrim(std::string &s);
    static std::string &rtrim(std::string &s);
    static std::string &trim(std::string &s);

    static bool str_to_us(std::string& s, unsigned short& val);
    //////////////////////////////////////////////////////////////////////////

  };
}

#endif
