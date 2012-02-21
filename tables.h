#ifndef tables_h_
#define tables_h_

#include <stdint.h>
#include <cstdlib>


// === Keys and values (common definitions used by both hash table implementations)

// The keys to be stored in our hash tables are 64-bit values. However two keys
// are set aside: Key(0) indicates that a record is empty, and Key(-1) indicates
// that the record has been deleted.

typedef uint64_t Key;
typedef Key KeyArg;
typedef uint64_t Value;
typedef Value ValueArg;
typedef uint32_t hashcode_t;

inline hashcode_t hash(KeyArg k) { return k; }

// A key is either "live" (that is, an actual value), empty, or a tombstone.
// For a given key, exactly one of the three predicates
// isLive/isEmpty/isTombstone is true. The implementation of isLive below is
// equivalent to !isEmpty(k) && !isTombstone(k) but *much* faster; it is the
// only fancy thing in this program.

inline bool isEmpty(KeyArg k) { return k == 0; }
inline void makeEmpty(Key &k) { k = 0; }
inline bool isTombstone(KeyArg k) { return k == Key(-1); }
inline void makeTombstone(Key &k) { k = Key(-1); }
inline bool isLive(KeyArg k) { return ((k + 1) & ~1) != 0; }

enum ByteSizeOption { BytesAllocated, BytesWritten };

// === OpenTable
// A simple hash table with open addressing.
// See <https://en.wikipedia.org/wiki/Hash_table#Open_addressing>.
//
class OpenTable {
    struct Entry {
        Key key;
        Value value;

        Entry() { makeEmpty(key); }
    };

    // Each Entry is either empty, live, or a tombstone.
    Entry *table;
    size_t live_count;      // number of live entries
    size_t nonempty_count;  // number of live and tombstone entries
    size_t mask;

    static const double minFillRatio = 0.25;
    static const double maxFillRatio = 0.75;

    inline Entry * lookup(KeyArg key);
    inline const Entry * lookup(KeyArg key) const;

    void grow();
    void shrink();
    void rehash(Entry *old_table, size_t old_capacity, size_t new_capacity);

public:
    OpenTable();
    ~OpenTable();

    size_t byte_size(ByteSizeOption option) const;
    size_t size() const;
    bool has(KeyArg key) const;
    Value get(KeyArg key) const;
    void set(KeyArg key, ValueArg value);
    bool remove(KeyArg key);
};


// === CloseTable
// A vector combined with a very simple hash table for fast lookup.
// Tyler Close proposed this.
//
class CloseTable {
private:
    struct Entry {
        Key key;
        Value value;
        Entry *chain;
    };

    typedef Entry *EntryPtr;

    EntryPtr *table;
    size_t table_mask;
    Entry *entries;
    size_t entries_capacity;
    size_t entries_length;
    size_t live_count;  // entries_length less tombstones

    inline Entry * lookup(KeyArg key, hashcode_t h);
    inline const Entry * lookup(KeyArg key) const;
    void rehash();

public:
    CloseTable();
    ~CloseTable();

    size_t byte_size(ByteSizeOption option) const;
    size_t size() const;
    bool has(KeyArg key) const;
    Value get(KeyArg key) const;
    void set(KeyArg key, ValueArg value);
    bool remove(KeyArg key);
};


#endif  // tables_h_
