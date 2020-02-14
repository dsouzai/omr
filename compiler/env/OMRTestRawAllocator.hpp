#ifndef OMR_TEST_RAW_ALLOCATOR
#define OMR_TEST_RAW_ALLOCATOR

#pragma once

#include <stddef.h>
#include <cstdlib>
#include <new>
#include "env/TypedAllocator.hpp"

namespace TestAlloc {

/* Raw Allocators */
class RawAllocator
   {
public:
   RawAllocator()
      {
      }

   RawAllocator(const RawAllocator &other)
      {
      }

   virtual void *allocate(size_t size, const std::nothrow_t tag, void * hint = 0)=0;
   virtual void *allocate(size_t size, void * hint = 0)=0;
   virtual void deallocate(void * p)=0;
   virtual void deallocate(void * p, const size_t size)=0;

   template <typename T>
   operator TR::typed_allocator< T, RawAllocator >() throw()
      {
      return TR::typed_allocator< T, RawAllocator >(*this);
      }
   };

class MallocRA : public RawAllocator
   {
public:
   MallocRA()
      {
      }

   MallocRA(const RawAllocator &other)
      {
      }

   virtual void *allocate(size_t size, const std::nothrow_t tag, void * hint = 0) throw()
      {
      return malloc(size);
      }

   virtual void * allocate(size_t size, void * hint = 0)
      {
      void * const alloc = allocate(size, std::nothrow, hint);
      if (!alloc) throw std::bad_alloc();
      return alloc;
      }

   virtual void deallocate(void * p) throw()
      {
      free(p);
      }

   virtual void deallocate(void * p, const size_t size) throw()
      {
      free(p);
      }
   };
}

inline void * operator new(size_t size, TestAlloc::RawAllocator &allocator)
   {
   return allocator.allocate(size);
   }

inline void operator delete(void *ptr, TestAlloc::RawAllocator &allocator) throw()
   {
   allocator.deallocate(ptr);
   }

inline void * operator new[](size_t size, TestAlloc::RawAllocator &allocator)
   {
   return allocator.allocate(size);
   }

inline void operator delete[](void *ptr, TestAlloc::RawAllocator &allocator) throw()
   {
   allocator.deallocate(ptr);
   }

inline void * operator new(size_t size, TestAlloc::RawAllocator &allocator, const std::nothrow_t& tag) throw()
   {
   return allocator.allocate(size, tag);
   }

inline void * operator new[](size_t size, TestAlloc::RawAllocator &allocator, const std::nothrow_t& tag) throw()
   {
   return allocator.allocate(size, tag);
   }

#endif // OMR_TEST_RAW_ALLOCATOR
