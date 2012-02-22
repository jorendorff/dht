#include "tables.h"
#include <cstring>

using namespace std;


// === OpenTable

OpenTable::OpenTable() {
    table = new Entry[8];
    mask = 7;
    live_count = 0;
    nonempty_count = 0;
}

OpenTable::~OpenTable() {
    delete[] table;
}

OpenTable::Entry *
OpenTable::lookup(KeyArg key)
{
    hashcode_t h = hash(key);
    size_t i = h & mask;
    h >>= 3;
    while (!isEmpty(table[i].key)) {
        if (table[i].key == key)
            return &table[i];
        i = (i + (h | 1)) & mask;
    }
    return NULL;
}

const OpenTable::Entry *
OpenTable::lookup(KeyArg key) const
{
    return const_cast<OpenTable *>(this)->lookup(key);
}

void
OpenTable::grow()
{
    Entry *old_table = table;
    size_t old_capacity = mask + 1;
    size_t new_capacity = 2 * old_capacity;
    rehash(old_table, old_capacity, new_capacity);
}

void
OpenTable::shrink()
{
    if (mask <= 7)
        return;  // don't bother

    Entry *old_table = table;
    size_t old_capacity = mask + 1;
    size_t new_capacity = old_capacity / 2;
    rehash(old_table, old_capacity, new_capacity);
}

void
OpenTable::rehash(Entry *old_table, size_t old_capacity, size_t new_capacity)
{
    table = new Entry[new_capacity];
    mask = new_capacity - 1;
    live_count = 0;
    nonempty_count = 0;
    for (Entry *p = old_table, *end = p + old_capacity; p != end; ++p) {
        if (isLive(p->key))
            set(p->key, p->value);
    }
    delete[] old_table;
}

size_t
OpenTable::byte_size(ByteSizeOption) const
{
    return sizeof(*this) + (mask + 1) * sizeof(Entry);
}

size_t
OpenTable::size() const
{
    return live_count;
}

bool
OpenTable::has(KeyArg key) const
{
    return lookup(key) != NULL;
}

Value
OpenTable::get(KeyArg key) const
{
    const Entry *e = lookup(key);
    return e ? e->value : Value();
}

void
OpenTable::set(KeyArg key, ValueArg value)
{
    hashcode_t h = hash(key);
    size_t i = h & mask;
    h >>= 3;
    while (isLive(table[i].key)) {
        if (table[i].key == key) {
            table[i].value = value;
            return;
        }
        i = (i + (h | 1)) & mask;
    }

    bool tomb = isTombstone(table[i].key);
    table[i].key = key;
    table[i].value = value;
    live_count++;
    if (!tomb)
        nonempty_count++;
    if (nonempty_count > (mask + 1) * maxFillRatio)
        grow();
}

bool
OpenTable::remove(KeyArg key)
{
    Entry *e = lookup(key);
    if (!e)
        return false;
    makeTombstone(e->key);
    live_count--;
    if (live_count < (mask + 1) * minFillRatio)
        shrink();
}


// === DenseTable

DenseTable::DenseTable()
{
    Key k;
    makeEmpty(k);
    map.set_empty_key(k);
    makeTombstone(k);
    map.set_deleted_key(k);
}

size_t
DenseTable::byte_size(ByteSizeOption) const
{
    return sizeof(*this) + sizeof(std::pair<const Key, Value>) * map.bucket_count();
}

size_t
DenseTable::size() const
{
    return map.size();
}

bool
DenseTable::has(KeyArg key) const
{
    return map.find(key) != map.end();
}

Value
DenseTable::get(KeyArg key) const
{
    Map::const_iterator it = map.find(key);
    return (it == map.end() ? Value() : it->second);
}

void
DenseTable::set(KeyArg key, ValueArg value)
{
    map[key] = value;
}

bool
DenseTable::remove(KeyArg key)
{
    // dense_hash_map does not resize the table on removing entries. TODO.
    Map::iterator it = map.find(key);
    if (it == map.end())
        return false;
    map.erase(it);
    return true;
}


// === CloseTable

CloseTable::CloseTable()
{
    table = new EntryPtr[InitialSize];
    memset(table, 0, InitialSize * sizeof(EntryPtr));
    table_mask = InitialSize - 1;
    entries_capacity = FillFactor * InitialSize;
    entries = new Entry[entries_capacity];
    entries_length = 0;
    live_count = 0;
}

CloseTable::~CloseTable()
{
    delete[] table;
    delete[] entries;
}

CloseTable::Entry *
CloseTable::lookup(KeyArg key, hashcode_t h)
{
    for (Entry *e = table[h & table_mask]; e; e = e->chain) {
        if (e->key == key)
            return e;
    }
    return NULL;
}

const CloseTable::Entry *
CloseTable::lookup(KeyArg key) const {
    return const_cast<CloseTable *>(this)->lookup(key, hash(key));
}

void
CloseTable::rehash()
{
    int grow = (live_count >= entries_length * 3 / 4) ? 1 : 0;
    size_t new_capacity = entries_capacity << grow;
    size_t new_table_mask = (table_mask << grow) | 1;
    EntryPtr *new_table = new EntryPtr[new_table_mask + 1];
    memset(new_table, 0, (new_table_mask + 1) * sizeof(EntryPtr));
    Entry *new_entries = new Entry[new_capacity];

    Entry *q = new_entries;
    for (Entry *p = entries, *end = entries + entries_length; p != end; p++) {
        if (!isEmpty(p->key)) {
            hashcode_t h = hash(p->key) & new_table_mask;
            q->key = p->key;
            q->value = p->value;
            q->chain = new_table[h];
            new_table[h] = q;
            q++;
        }
    }

    delete[] table;
    delete[] entries;
    table = new_table;
    table_mask = new_table_mask;
    entries = new_entries;
    entries_capacity = new_capacity;
    entries_length = live_count;
}

size_t
CloseTable::byte_size(ByteSizeOption option) const
{
    return sizeof(*this)
        + (table_mask + 1) * sizeof(EntryPtr)
        + (option == BytesAllocated ? entries_capacity : entries_length) * sizeof(Entry);
}

size_t
CloseTable::size() const
{
    return live_count;
}

bool
CloseTable::has(KeyArg key) const
{
    return lookup(key) != NULL;
}

Value
CloseTable::get(KeyArg key) const
{
    const Entry *e = lookup(key);
    return e ? e->value : Value();
}

void
CloseTable::set(KeyArg key, ValueArg value)
{
    hashcode_t h = hash(key);
    Entry *e = lookup(key, h);
    if (e) {
        e->value = value;
    } else {
        if (entries_length == entries_capacity)
            rehash();
        h &= table_mask;
        live_count++;
        e = &entries[entries_length++];
        e->key = key;
        e->value = value;
        e->chain = table[h];
        table[h] = e;
    }
}

bool
CloseTable::remove(KeyArg key)
{
    // If an entry exists for the given key, empty it.
    //
    // This never resizes the table. It should, but no tests even call this
    // method yet. TODO.

    hashcode_t h = hash(key);
    Entry *e = lookup(key, h);
    if (e == NULL)
        return false;
    live_count--;
    table[h & table_mask] = e->chain;
    makeEmpty(e->key);
    return true;
}
