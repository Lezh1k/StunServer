#ifndef FUNCTORWITHOUTRESULT_H
#define FUNCTORWITHOUTRESULT_H

#include "IFunctor.h"

//RT - RETURN TYPE
//P1-P16 - PARAMETER TYPES
template<class P1 = char, class P2 = char, class P3 = char, \
class P4 = char, class P5 = char, class P6 = char, class P7 = char, class P8 = char, \
class P9 = char, class P10 = char, class P11 = char, class P12 = char, class P13 = char, \
class P14 = char, class P15 = char, class P16 = char> class FunctorWithoutResult;
//////////////////////////////////////////////////////////////////////////

template<> class FunctorWithoutResult<> : public IFunctor
{
private:
  void(*m_pf)(void);

public:
  FunctorWithoutResult(void(*pf)(void), const char* methodName) : IFunctor(methodName),\
    m_pf(pf)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf();
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1> class FunctorWithoutResult<P1> : public IFunctor
{
private:

  P1 m_param1;
  void(*m_pf)(P1);

public:
  FunctorWithoutResult(void(*pf)(P1), P1 param1, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2> class FunctorWithoutResult<P1, P2> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  void(*m_pf)(P1, P2);

public:
  FunctorWithoutResult(void(*pf)(P1, P2), P1 param1, P2 param2, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void InvokeMethod(void) {
    m_pf(m_param1, m_param2);
  }

  virtual void operator()(void) {
    m_pf(m_param1, m_param2);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3> class FunctorWithoutResult< P1, P2, P3> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  void(*m_pf)(P1, P2, P3);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3), P1 param1, P2 param2, P3 param3, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4> class FunctorWithoutResult< P1, P2, P3, P4> : public IFunctor
{
private:

  void(*m_pf)(P1, P2, P3, P4);
  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;  

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4), P1 param1, P2 param2, P3 param3, P4 param4, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5> class FunctorWithoutResult< P1, P2, P3, P4, P5> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  void(*m_pf)(P1, P2, P3, P4, P5);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5), P1 param1, P2 param2, P3 param3, P4 param4,
    P5 param5, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5)
  {  }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  void(*m_pf)(P1, P2, P3, P4, P5, P6);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6), P1 param1, P2 param2, P3 param3, P4 param4, P5 param5,
    P6 param6, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7), P1 param1, P2 param2, P3 param3, P4 param4, P5 param5,
    P6 param6, P7 param7, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8), P1 param1, P2 param2, P3 param3, P4 param4, P5 param5,
    P6 param6, P7 param7, P8 param8, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8, P9> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  P9 m_param9;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9), P1 param1, P2 param2, P3 param3, P4 param4, P5 param5, P6 param6,
    P7 param7, P8 param8, P9 param9, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8), m_param9(param9)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8, m_param9);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8, P9, P10> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  P9 m_param9;
  P10 m_param10;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10), P1 param1, P2 param2, P3 param3, P4 param4, P5 param5, P6 param6,
    P7 param7, P8 param8, P9 param9, P10 param10, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8), m_param9(param9), m_param10(param10)
  {  }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8, m_param9, m_param10);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  P9 m_param9;
  P10 m_param10;
  P11 m_param11;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11), P1 param1, P2 param2, P3 param3, P4 param4,
    P5 param5, P6 param6, P7 param7, P8 param8, P9 param9, P10 param10, P11 param11, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8), m_param9(param9), m_param10(param10), m_param11(param11)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8, m_param9, m_param10, m_param11);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  P9 m_param9;
  P10 m_param10;
  P11 m_param11;
  P12 m_param12;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12), P1 param1, P2 param2, P3 param3, P4 param4, P5 param5,
    P6 param6, P7 param7, P8 param8, P9 param9, P10 param10, P11 param11,
    P12 param12, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8), m_param9(param9), m_param10(param10), m_param11(param11),\
    m_param12(param12)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8, m_param9, m_param10, m_param11, m_param12);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  P9 m_param9;
  P10 m_param10;
  P11 m_param11;
  P12 m_param12;
  P13 m_param13;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13), P1 param1, P2 param2, P3 param3, P4 param4, P5 param5,
    P6 param6, P7 param7, P8 param8, P9 param9, P10 param10, P11 param11, P12 param12,
    P13 param13, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8), m_param9(param9), m_param10(param10), m_param11(param11),\
    m_param12(param12), m_param13(param13)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8, m_param9, m_param10, m_param11, m_param12, m_param13);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  P9 m_param9;
  P10 m_param10;
  P11 m_param11;
  P12 m_param12;
  P13 m_param13;
  P14 m_param14;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14), P1 param1, P2 param2, P3 param3, P4 param4,
    P5 param5, P6 param6, P7 param7, P8 param8, P9 param9, P10 param10, P11 param11, P12 param12,
    P13 param13, P14 param14, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8), m_param9(param9), m_param10(param10), m_param11(param11),\
    m_param12(param12), m_param13(param13), m_param14(param14)
  { }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8, m_param9, m_param10, m_param11, m_param12, m_param13, m_param14);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////

template< class P1, class P2, class P3, class P4, class P5, class P6, class P7, class P8, class P9, class P10, class P11, class P12, class P13, class P14, class P15> class FunctorWithoutResult< P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15> : public IFunctor
{
private:

  P1 m_param1;
  P2 m_param2;
  P3 m_param3;
  P4 m_param4;
  P5 m_param5;
  P6 m_param6;
  P7 m_param7;
  P8 m_param8;
  P9 m_param9;
  P10 m_param10;
  P11 m_param11;
  P12 m_param12;
  P13 m_param13;
  P14 m_param14;
  P15 m_param15;
  void(*m_pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15);

public:
  FunctorWithoutResult(void(*pf)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15), P1 param1, P2 param2, P3 param3, P4 param4,
    P5 param5, P6 param6, P7 param7, P8 param8, P9 param9, P10 param10, P11 param11, P12 param12, P13 param13,
    P14 param14, P15 param15, const char* methodName) : IFunctor(methodName),\
    m_pf(pf), m_param1(param1), m_param2(param2), m_param3(param3), m_param4(param4), m_param5(param5),\
    m_param6(param6), m_param7(param7), m_param8(param8), m_param9(param9), m_param10(param10), m_param11(param11),\
    m_param12(param12), m_param13(param13), m_param14(param14), m_param15(param15)
  {  }

  virtual ~FunctorWithoutResult(void){}

  virtual void operator()(void) {
    m_pf(m_param1, m_param2, m_param3, m_param4, m_param5, m_param6, m_param7, m_param8, m_param9, m_param10, m_param11, m_param12, m_param13, m_param14, m_param15);
  }

  virtual void* GetResult(void) {
    return NULL;
  }
};
//////////////////////////////////////////////////////////////////////////


#endif // FUNCTORWITHOUTRESULT_H
