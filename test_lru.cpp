
#include <iostream>
#include <cassert>
#include "lru_cache.h"

using namespace std;

int main()
{
    LRUCache cache(3);
    string value;

    // TEST 1: Insert elements
    cache.set("A", "Apple");
    cache.set("B", "Banana");
    cache.set("C", "Cherry");

    assert(cache.size() == 3);
    cout << "TEST 1 PASSED: Insertions\n";

    // TEST 2: Get existing key
    assert(cache.get("A", value));
    assert(value == "Apple");

    cout << "TEST 2 PASSED: Retrieval\n";

    // TEST 3: LRU eviction
    cache.set("D", "Dragonfruit");

    assert(cache.exists("A"));
    assert(!cache.exists("B"));
    assert(cache.exists("C"));
    assert(cache.exists("D"));

    cout << "TEST 3 PASSED: LRU eviction\n";

    // TEST 4: Update existing key
    cache.set("A", "Updated Apple");

    assert(cache.get("A", value));
    assert(value == "Updated Apple");

    cout << "TEST 4 PASSED: Update\n";

    // TEST 5: Delete
    assert(cache.remove("C"));
    assert(!cache.exists("C"));

    cout << "TEST 5 PASSED: Deletion\n";

    // TEST 6: Missing key
    assert(!cache.get("X", value));

    cout << "TEST 6 PASSED: Missing key\n";

    // TEST 7: Zero-capacity cache
    LRUCache emptyCache(0);
    emptyCache.set("X", "Test");

    assert(emptyCache.size() == 0);
    assert(!emptyCache.exists("X"));

    cout << "TEST 7 PASSED: Zero capacity\n";

    cout << "\nALL LRU TESTS PASSED!\n";

    return 0;
}
