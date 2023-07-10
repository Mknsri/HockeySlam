#ifndef GAME_MEMORY_H
#define GAME_MEMORY_H

#define MAX_ALLOCATIONS 256

struct game_memory_region
{
  const uint8_t* Start;
  size_t Size;
  bool Free;

  game_memory_region* Next;
  game_memory_region* Previous;
};

struct game_memory_allocator
{
  size_t MaxSize;
  uint8_t* Begin;
  uint8_t* CursorOffset;

  game_memory_region* FirstRegion;
  game_memory_region* TailRegion;
  uint32_t AllocationCount;

  bool Initialized;
};

void* allocate(size_t size);
void unallocate(void* cursor);
void* read(void* start, size_t size);
void* allocate_t(size_t size);
void unallocate_t(void* cursor);

#endif /* GAME_MEMORY_H */