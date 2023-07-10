#include "game_collections.h"

void append(linked_list& list, const void* const value)
{
  linked_list_node** currNode = (linked_list_node**)&list.Root;

  while (*currNode) {
    currNode = &(*currNode)->Next;
  }

  *currNode = (linked_list_node*)allocate_t(sizeof(linked_list_node));
  (*currNode)->Data = (void*)value;
  (*currNode)->Next = nullptr;
  list.Size++;
}

void insert(linked_list& list, size_t index, const void* const value)
{
  linked_list_node** currNode = &list.Root;
  size_t currIndex = 0;
  while (*currNode) {
    if (currIndex++ == index) {
      break;
    }
    currNode = &(*currNode)->Next;
  }

  linked_list_node* shifted = *currNode;
  linked_list_node* newNode =
    (linked_list_node*)allocate_t(sizeof(linked_list_node));
  newNode->Data = (void*)value;
  newNode->Next = shifted;
  *currNode = newNode;
  list.Size++;
}

void remove_at(linked_list& list, size_t index)
{
  const linked_list_node** currNode = (const linked_list_node**)&list.Root;
  size_t currIndex = 0;
  while (*currNode) {
    if (currIndex++ == index) {
      *currNode = (*currNode)->Next;
      list.Size--;
      break;
    }

    *currNode = (*currNode)->Next;
  }
}

void remove_value(linked_list& list, const void* const value)
{
  linked_list_node* currNode = list.Root;
  linked_list_node** nodeAddress = (linked_list_node**)&list.Root;
  while (currNode) {
    if (currNode->Data == value) {
      *nodeAddress = currNode->Next;
      list.Size--;
      break;
    }

    nodeAddress = &currNode->Next;
    currNode = *nodeAddress;
  }
}

void clear(linked_list& list)
{
  linked_list_node* currNode = list.Root;
  linked_list_node** nodeAddress = (linked_list_node**)&list.Root;
  while (currNode) {
    unallocate_t(currNode->Data);
    unallocate_t(currNode);

    nodeAddress = &currNode->Next;
    currNode = *nodeAddress;
  }
  list.Root = NULL;
  list.Size = 0;
}

bool binary_tree_comparator_default(const void* const parent,
                                    const void* const node)
{
  int* l = (int*)parent;
  int* r = (int*)node;

  return l > r;
}

bool binary_tree_comparator_string(const void* const parent,
                                   const void* const node)
{
  char* l = (char*)parent;
  char* r = (char*)node;

  while (*l && *l == *r) {
    l++;
    r++;
  }

  return *l > *r;
}

void insert(binary_tree& tree, const void* const key, const void* const value)
{
  if (tree.Comparator == NULL) {
    tree.Comparator = &binary_tree_comparator_default;
  }

  binary_tree_node** currNode = &tree.Root;
  while (*currNode) {
    bool moveLeft = tree.Comparator((*currNode)->Data, value);
    currNode = moveLeft ? &(*currNode)->Left : &(*currNode)->Right;
  }

  *currNode = (binary_tree_node*)allocate_t(sizeof(binary_tree_node));
  (*currNode)->Key = (void*)key;
  (*currNode)->Data = (void*)value;
}

binary_tree_node* find_node(binary_tree_node* start,
                            const void* const key,
                            bool (*comparator)(const void*, const void*))
{
  binary_tree_node* currNode = start;
  while (currNode) {
    if (currNode->Key == key) {
      return currNode;
    }

    bool moveLeft = comparator(currNode->Key, key);
    currNode = moveLeft ? currNode->Left : currNode->Right;
    currNode = (binary_tree_node*)find_node(currNode, key, comparator);
  }

  return NULL;
}

void* find_by_key(binary_tree& tree, const void* const key)
{
  binary_tree_node* currNode = find_node(tree.Root, key, tree.Comparator);
  return currNode ? currNode->Data : NULL;
}

void remove_key(binary_tree& tree, const void* const key)
{
  if (tree.Comparator == NULL) {
    tree.Comparator = &binary_tree_comparator_default;
  }

  binary_tree_node* currNode = find_node(tree.Root, key, tree.Comparator);
  binary_tree_node** nodeAddress = &currNode;

  bool bothChildren = currNode->Left && currNode->Right;
  if (bothChildren) {
    binary_tree_node* toDelete = currNode;
    binary_tree_node** toReplace = &currNode;

    // Successor
    toReplace = &currNode->Right;
    while ((*toReplace)->Left) {
      toReplace = &(*toReplace)->Left;
    }
    toDelete->Data = (*toReplace)->Data;
    toDelete->Key = (*toReplace)->Key;
    *toReplace = (*toReplace)->Left ? (*toReplace)->Left : (*toReplace)->Right;
#if 0
        // Predecessor
        toReplace = &currNode->Left;
        while ((*toReplace)->Right) {
            toReplace = &(*toReplace)->Right;
        }
        toDelete->Data = (*toReplace)->Data;
        toReplace = (*toReplace)->Right ? &(*toReplace)->Right : &(*toReplace)->Left;
#endif
  } else {
    *nodeAddress = currNode->Left ? currNode->Left : currNode->Right;
  }
}

void remove(hash_table& table, uintptr_t key)
{
  uintptr_t hashKey = hash_key(table, key);
  hash_table_entry* entry = (hash_table_entry*)table.Entries + hashKey;

  while (entry->Next) {
    hash_table_entry* next = entry->Next;
    unallocate_t(entry->Next);
    *entry = {};
    entry = next;
  }
  *entry = {};
}

void clear(hash_table& table)
{
  for (uintptr_t key = 0; key < table.Size; key++) {
    remove(table, key);
    table.Entries[key] = {};
  }
}

hash_table create_hash_table(size_t size)
{
  HOKI_ASSERT((size & (size - 1)) == 0); // Check ^2
  hash_table result = {};
  result.Size = size;
  result.Entries =
    (hash_table_entry**)allocate_t(size * sizeof(hash_table_entry*));
#if HOKI_DEV
  for (size_t i = 0; i < size; i++) {
    result.Entries[i] = {};
  }
#endif

  return result;
}

void insert(const hash_table& table, uintptr_t key, const void* const value)
{
  uintptr_t hashKey = hash_key(table, key);
  hash_table_entry** entry = &table.Entries[hashKey];

  while (*entry) {
    entry = &(*entry)->Next;
  }

  (*entry) = (hash_table_entry*)allocate_t(sizeof(hash_table_entry));
  (*entry)->Key = key;
  (*entry)->Next = nullptr;
  (*entry)->Value = (void*)value;
}
