#include "game_memory.h"

static game_memory_allocator* permanentMemoryPool = nullptr;
static game_memory_allocator* transientMemoryPool = nullptr;

static bool memory_pools_loaded()
{
  return permanentMemoryPool != nullptr && transientMemoryPool != nullptr;
}

static void set_memory_pools(game_memory& memory)
{
  permanentMemoryPool = &memory.PermanentStorageAllocator;
  transientMemoryPool = &memory.TransientStorageAllocator;
}

static void m_init_allocator(void* begin,
                             const size_t size,
                             game_memory_allocator& memoryPool)
{
  memoryPool = {};

  memoryPool.MaxSize = size;
  memoryPool.Begin = (uint8_t*)begin;

  // Create region and set first and tail to it
  game_memory_region* initialRegion = (game_memory_region*)memoryPool.Begin;
  initialRegion->Start = memoryPool.Begin + sizeof(game_memory_region);
  initialRegion->Size = sizeof(game_memory_region);
  initialRegion->Next = nullptr;
  initialRegion->Free = false;

  memoryPool.FirstRegion = initialRegion;
  memoryPool.TailRegion = initialRegion;

  memoryPool.CursorOffset = (uint8_t*)initialRegion->Size;

  memoryPool.Initialized = true;
}

void init_allocator(game_memory& memory)
{
  m_init_allocator(memory.PermanentStorage,
                   memory.PermanentStorageSize,
                   memory.PermanentStorageAllocator);
  m_init_allocator(memory.TransientStorage,
                   memory.TransientStorageSize,
                   memory.TransientStorageAllocator);
  set_memory_pools(memory);
}

static void* m_allocate(const size_t size, game_memory_allocator& memoryPool)
{
  if (size == 0) {
    return NULL;
  }

  size_t cursor = (size_t)memoryPool.CursorOffset;
  if ((memoryPool.Begin + cursor + size) >
      (memoryPool.Begin + memoryPool.MaxSize)) {
    HOKI_ASSERT(false);
  }

  game_memory_region* tailRegion = memoryPool.TailRegion;
  game_memory_region* allocatedRegion =
    (game_memory_region*)(memoryPool.Begin + cursor);
  allocatedRegion->Start =
    (memoryPool.Begin + cursor + sizeof(game_memory_region));
  allocatedRegion->Size = size;
  allocatedRegion->Free = false;
  allocatedRegion->Next = NULL;

  memoryPool.AllocationCount++;

  tailRegion->Next = allocatedRegion;
  allocatedRegion->Previous = tailRegion;
  memoryPool.CursorOffset =
    (uint8_t*)(allocatedRegion->Start + size - memoryPool.Begin);
  memoryPool.TailRegion = allocatedRegion;

  HOKI_ASSERT(allocatedRegion->Start);
  HOKI_ASSERT((memoryPool.TailRegion->Start + memoryPool.TailRegion->Size) ==
              (memoryPool.Begin + (size_t)memoryPool.CursorOffset));

  return (void*)allocatedRegion->Start;
}

static void m_unallocate(void* cursor, game_memory_allocator& memoryPool)
{
  if (cursor == NULL) {
    return;
  }

  game_memory_region* region =
    (game_memory_region*)((uint8_t*)cursor - sizeof(game_memory_region));
  if (region->Start == NULL || region->Free) {
    return;
  }

  HOKI_ASSERT(region->Start == cursor);
  region->Free = true;
}

static void* m_reallocate(void* start,
                          const size_t size,
                          game_memory_allocator& memoryPool)
{
  if (start == NULL) {
    return m_allocate(size, memoryPool);
  }

  game_memory_region* regionToAllocate =
    (game_memory_region*)((uint8_t*)start - sizeof(game_memory_region));

  // cant fit
  if (regionToAllocate->Size < size) {
    uint8_t* newAllocation = (uint8_t*)m_allocate(size, memoryPool);
    uint8_t* targetCursor = newAllocation;
    uint8_t* copyCursor = (uint8_t*)start;
    while (copyCursor < ((uint8_t*)start + regionToAllocate->Size)) {
      *targetCursor++ = *copyCursor++;
    }
    m_unallocate((uint8_t*)regionToAllocate->Start, memoryPool);
    HOKI_ASSERT(targetCursor == (newAllocation + regionToAllocate->Size));
    return newAllocation;
  } else {
    HOKI_ASSERT(start);

    return start;
  }
}

#if HOKI_DEV
void dump_memory_list(game_memory_allocator& memoryPool)
{
  game_memory_region* region = memoryPool.FirstRegion;
  int regIdx = 0;
  DEBUG_LOG("Starting memory dump\n");
  while (region) {
    const uint8_t* addr = region->Start;
    DEBUG_LOG(
      "%i# Start: %x Size: #i\n", regIdx++, ((const void*)addr), region->Size);
    region = region->Next;
  }
  DEBUG_LOG("Ended memory dump\n");
}
#endif

void* read(void* start, const size_t size)
{
  return (uint8_t*)start + size;
}

void* allocate(const size_t size)
{
  return m_allocate(size, *permanentMemoryPool);
}

void* reallocate(void* start, const size_t size)
{
  return m_reallocate(start, size, *permanentMemoryPool);
}

void unallocate(void* cursor)
{
  return m_unallocate(cursor, *permanentMemoryPool);
}

void* allocate_t(const size_t size)
{
  return m_allocate(size, *transientMemoryPool);
}

void* reallocate_t(void* start, const size_t size)
{
  return m_reallocate(start, size, *transientMemoryPool);
}

void unallocate_t(void* cursor)
{
  return m_unallocate(cursor, *transientMemoryPool);
}
