#if defined(NEW_MEMORY)

#include "infra/Assert.hpp"
#include "env/OMRTestSegmentAllocator.hpp"
#include "env/MemorySegment.hpp"

TestAlloc::OMRSystemSegmentProvider::OMRSystemSegmentProvider(size_t segmentSize, TestAlloc::RawAllocator &rawAllocator) :
   _rawAllocator(rawAllocator),
   _currentBytesAllocated(0),
   _highWaterMark(0),
   _defaultSegmentSize(segmentSize),
   _segments(std::less< TR::MemorySegment >(), SegmentSetAllocator(rawAllocator))
   {
   }

TestAlloc::OMRSystemSegmentProvider::~OMRSystemSegmentProvider() throw()
   {
   }

TR::MemorySegment &
TestAlloc::OMRSystemSegmentProvider::allocate(size_t requiredSize)
   {
   size_t adjustedSize = ( ( requiredSize + (_defaultSegmentSize - 1) ) / _defaultSegmentSize ) * _defaultSegmentSize;
   void *newSegmentArea = _rawAllocator.allocate(adjustedSize);
   try
      {
      auto result = _segments.insert( TR::MemorySegment(newSegmentArea, adjustedSize) );
      TR_ASSERT(result.first != _segments.end(), "Bad iterator");
      TR_ASSERT(result.second, "Insertion failed");
      _currentBytesAllocated += adjustedSize;
      _highWaterMark = _currentBytesAllocated > _highWaterMark ? _currentBytesAllocated : _highWaterMark;
      return const_cast<TR::MemorySegment &>(*(result.first));
      }
   catch (...)
      {
      _rawAllocator.deallocate(newSegmentArea);
      throw;
      }
   }

void
TestAlloc::OMRSystemSegmentProvider::deallocate(TR::MemorySegment &segment) throw()
   {
   auto it = _segments.find(segment);
   _rawAllocator.deallocate(segment.base());
   _currentBytesAllocated -= segment.size();
   TR_ASSERT(it != _segments.end(), "Segment lookup should never fail");
   _segments.erase(it);
   }

size_t
TestAlloc::OMRSystemSegmentProvider::bytesAllocated() const throw()
   {
   return _highWaterMark;
   }

#if (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX)
#include <sys/mman.h>
#if defined(__APPLE__) || !defined(MAP_ANONYMOUS)
#define MAP_ANONYMOUS MAP_ANON
#endif
#elif defined(OMR_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <string.h>
#endif /* defined(OMR_OS_WINDOWS) */


TestAlloc::OMRDebugSegmentProvider::OMRDebugSegmentProvider(size_t segmentSize, TestAlloc::RawAllocator &rawAllocator) :
   _rawAllocator(rawAllocator),
   _currentBytesAllocated(0),
   _defaultSegmentSize(segmentSize),
   _segments(std::less< TR::MemorySegment >(), SegmentSetAllocator(rawAllocator))
   {
   }

TestAlloc::OMRDebugSegmentProvider::~OMRDebugSegmentProvider() throw()
   {
   for ( auto it = _segments.begin(); it != _segments.end(); it = _segments.begin() )
      {
#if (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX)
      munmap(it->base(), it->size());
#elif defined(OMR_OS_WINDOWS)
      VirtualFree(it->base(), 0, MEM_RELEASE);
#else
      _rawAllocator.deallocate(it->base(), it->size());
#endif /* (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX) */
      _segments.erase(it);
      }
   }

TR::MemorySegment &
TestAlloc::OMRDebugSegmentProvider::allocate(size_t requiredSize)
   {
   size_t adjustedSize = ( ( requiredSize + (_defaultSegmentSize - 1) ) / _defaultSegmentSize ) * _defaultSegmentSize;
#if (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX)
   void *newSegmentArea = mmap(NULL, adjustedSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
   if (newSegmentArea == MAP_FAILED) throw std::bad_alloc();
#elif defined(OMR_OS_WINDOWS)
   void *newSegmentArea = VirtualAlloc(NULL, adjustedSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
   if (!newSegmentArea) throw std::bad_alloc();
#else
   void *newSegmentArea = _rawAllocator.allocate(requiredSize);
#endif /* (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX) */
   try
      {
      auto result = _segments.insert( TR::MemorySegment(newSegmentArea, adjustedSize) );
      TR_ASSERT(result.first != _segments.end(), "Bad iterator");
      TR_ASSERT(result.second, "Insertion failed");
      _currentBytesAllocated += adjustedSize;
      return const_cast<TR::MemorySegment &>(*(result.first));
      }
   catch (...)
      {
#if (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX)
      munmap(newSegmentArea, adjustedSize);
#elif defined(OMR_OS_WINDOWS)
      VirtualFree(newSegmentArea, 0, MEM_RELEASE);
#else
     _rawAllocator.deallocate(newSegmentArea, adjustedSize);
#endif /* (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX) */
      throw;
      }
   }

void
TestAlloc::OMRDebugSegmentProvider::deallocate(TR::MemorySegment &segment) throw()
   {
#if (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX)
   void * remap = mmap(segment.base(), segment.size(), PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
   TR_ASSERT(remap == segment.base(), "Remapping of memory failed!");
#elif defined(OMR_OS_WINDOWS)
   VirtualFree(segment.base(), segment.size(), MEM_DECOMMIT);
   VirtualAlloc(segment.base(), segment.size(), MEM_COMMIT, PAGE_NOACCESS);
#else
   memset(segment.base(), 0xEF, segment.size());
#endif /* (defined(LINUX) && !defined(OMRZTPF)) || defined(__APPLE__) || defined(_AIX) */
   }

size_t
TestAlloc::OMRDebugSegmentProvider::bytesAllocated() const throw()
   {
   return _currentBytesAllocated;
   }

#endif //defined(NEW_MEMORY)
