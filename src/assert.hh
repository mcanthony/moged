// -*- C++ -*-

#ifndef LAB_ASSERT_LABASSERT_H
#define LAB_ASSERT_LABASSERT_H

#ifdef DEBUG
#include <iostream>
#include <cstdlib>
#endif

#if defined(LINUX)
#define DEBUGBREAK() asm("int $3") ;
#elif defined(WIN32)
#define DEBUGBREAK() __asm { int 3 }
#endif

#ifdef DEBUG
//#define LABASSERT(condition,message) do { if(!(condition)) { std::cerr << message << std::endl; DEBUGBREAK(); } } while(0)
#define ASSERT(condition) do { if(!(condition)) { DEBUGBREAK() } } while(0)
#else
//#define LABASSERT(condition,message) do { (void)sizeof(condition); } while(0)
#define ASSERT(condition) do { (void)sizeof(condition); } while(0)
#endif

#endif


