#ifndef COMMONS_H
#define COMMONS_H

#define UNUSED_ARG(x) ((void)(x))

typedef char            byte_t;
typedef unsigned short  word_t;
typedef unsigned int    dword_t;

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

    static void SwapBytes(byte_t* b1, byte_t* b2);

    static word_t SwapWord(word_t val);
    static dword_t SwapDword(dword_t val);

    static word_t HToNs(word_t val) {return SwapWord(val);}
    static word_t NToHs(word_t val) {return SwapWord(val);}

    static dword_t HToNl(dword_t val) { return SwapDword(val);}
    static dword_t NToHl(dword_t val) { return SwapDword(val);}

    static word_t GetWordFromNetStream(const byte_t* stream);
    static void   SetWordToNetStream(byte_t* stream, word_t val);
    static dword_t GetDWordFromNetStream(const byte_t* stream);
    static void  SetDWordToNetStream(byte_t* stream, dword_t val);

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
