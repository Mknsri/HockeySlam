#ifndef GAME_COLLECTIONS_H
#define GAME_COLLECTIONS_H

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

struct array
{
  size_t Size;
  void* Items;
};

struct linked_list_node
{
  linked_list_node* Next;
  void* Data;
};

struct linked_list
{
  linked_list_node* Root;
  size_t Size;
};

struct binary_tree_node
{
  binary_tree_node* Left;
  binary_tree_node* Right;
  void* Key;
  void* Data;
};

struct binary_tree
{
  binary_tree_node* Root;
  bool (*Comparator)(const void*, const void*);
};

struct hash_table_entry
{
  hash_table_entry* Next;
  uintptr_t Key;
  void* Value;
};

struct hash_table
{
  size_t Size;
  hash_table_entry** Entries;
};

array create_array(void* data)
{
  array result = {};
  result.Items = data;
  return result;
}

uintptr_t hash_key(const hash_table& table, uintptr_t key)
{
  return (key * 31) % table.Size;
}

const void* get(linked_list& list, size_t index)
{
  linked_list_node** currNode = &list.Root;
  size_t currIndex = 0;
  while (*currNode) {
    if (currIndex++ == index) {
      return (*currNode)->Data;
    }
    currNode = &(*currNode)->Next;
  }

  return nullptr;
}

void* get(const hash_table& table, const size_t key)
{
  uintptr_t rawKey = (uintptr_t)key;
  uintptr_t hashKey = hash_key(table, rawKey);
  hash_table_entry* entry = table.Entries[hashKey];

  while (entry) {
    if (entry->Key == rawKey) {
      return entry->Value;
    } else {
      entry = entry->Next;
    }
  }

  return nullptr;
}

void* get(const hash_table& table, const void* key)
{
  if (key == nullptr) {
    return nullptr;
  }
  uintptr_t rawKey = (uintptr_t)key;
  uintptr_t hashKey = hash_key(table, rawKey);
  hash_table_entry* entry = table.Entries[hashKey];

  while (entry) {
    if (entry->Key == rawKey) {
      return entry->Value;
    } else {
      entry = entry->Next;
    }
  }

  return nullptr;
}
#endif // GAME_COLLECTIONS_H