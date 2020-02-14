#ifndef OMR_TEST_SEGMENT_ALLOCATOR
#define OMR_TEST_SEGMENT_ALLOCATOR

#pragma once

#include <stddef.h>
#include <limits.h>
#include <new>
#include <set>
#include "env/MemorySegment.hpp"
#include "env/OMRTestRawAllocator.hpp"

namespace TestAlloc {

/* Segment Allocators */
class SegmentAllocator
   {
public:
   virtual TR::MemorySegment &allocate(size_t size) = 0;
   virtual void  deallocate(TR::MemorySegment &segment) = 0;
   virtual size_t allocationLimit() const throw() { TR_ASSERT_FATAL(false, "This routine should not be called\n"); return UINT_MAX; }
   virtual void setAllocationLimit(size_t allocationLimit) {  TR_ASSERT_FATAL(false, "This routine should not be called\n"); }
   virtual size_t getPreferredSegmentSize() { TR_ASSERT_FATAL(false, "This routine should not be called\n"); return 0; }
   virtual size_t bytesAllocated() const throw() { TR_ASSERT_FATAL(false, "This routine should not be called\n"); return 0; }
   virtual size_t systemBytesAllocated() const throw() { TR_ASSERT_FATAL(false, "This routine should not be called\n"); return 0; }
   virtual size_t regionBytesAllocated() const throw() { TR_ASSERT_FATAL(false, "This routine should not be called\n"); return 0; }
   };

class OMRSystemSegmentProvider : public SegmentAllocator
   {
public:
   OMRSystemSegmentProvider(size_t segmentSize, RawAllocator &rawAllocator);
   ~OMRSystemSegmentProvider() throw();
   virtual TR::MemorySegment &allocate(size_t size);
   virtual void  deallocate(TR::MemorySegment &segment) throw();
   virtual size_t bytesAllocated() const throw();

   private:
      RawAllocator &_rawAllocator;
      size_t _currentBytesAllocated;
      size_t _highWaterMark;
      size_t _defaultSegmentSize;

      typedef TR::typed_allocator<
         TR::MemorySegment,
         RawAllocator
         > SegmentSetAllocator;

      std::set<
         TR::MemorySegment,
         std::less< TR::MemorySegment >,
         SegmentSetAllocator
         > _segments;
   };

class OMRDebugSegmentProvider : public SegmentAllocator
   {
public:
   OMRDebugSegmentProvider(size_t segmentSize, RawAllocator &rawAllocator);
   ~OMRDebugSegmentProvider      () throw();
   virtual TR::MemorySegment &allocate(size_t size);
   virtual void  deallocate(TR::MemorySegment &segment) throw();
   virtual size_t bytesAllocated() const throw();

   private:
      RawAllocator &_rawAllocator;
      size_t _currentBytesAllocated;
      size_t _defaultSegmentSize;

      typedef TR::typed_allocator<
         TR::MemorySegment,
         RawAllocator
         > SegmentSetAllocator;

      std::set<
         TR::MemorySegment,
         std::less< TR::MemorySegment >,
         SegmentSetAllocator
         > _segments;
   };

}

#endif // OMR_TEST_SEGMENT_ALLOCATOR
