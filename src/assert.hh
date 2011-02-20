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

namespace Assert
{
    template <bool> struct COMPILE_ASSERT_FAILURE; // undefined if not true
    template <> struct COMPILE_ASSERT_FAILURE<true> { enum { value = 1 } ; };
    template <int x> struct compile_assert_test{};


}

#define COMPILE_ASSERT( condition ) \
    typedef ::Assert::compile_assert_test< sizeof( ::Assert::COMPILE_ASSERT_FAILURE< (bool) condition > ) > \
        __compile_assert_typedef_ ## __LINE __

#endif


