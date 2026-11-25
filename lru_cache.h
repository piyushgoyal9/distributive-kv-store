
#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <iostream>
#include <unordered_map>
#include <list>
#include <string>

using namespace std;

class LRUCache {

private:

    size_t capacity;

    // Most recently used element stays at front
    list<pair<string, string>> cache;

    // Key points to its position in the list
    unordered_map<
        string,
        list<pair<string, string>>::iterator
    > cacheMap;


public:

    LRUCache(size_t cap) {
        capacity = cap;
    }

    // GET
    bool get(const string& key, string& value) {

        auto it = cacheMap.find(key);

        if (it == cacheMap.end()) {
            return false;
        }

        // Move accessed element to front
        cache.splice(
            cache.begin(),
            cache,
            it->second
        );

        value = it->second->second;

        return true;
    }

    // SET
    void set(const string& key, const string& value) {
        if (capacity == 0) {
        return;
    }

        auto it = cacheMap.find(key);

        // Key already exists
        if (it != cacheMap.end()) {

            it->second->second = value;

            cache.splice(
                cache.begin(),
                cache,
                it->second
            );

            return;
        }

        // Evict least recently used element
        if (cache.size() >= capacity) {

            string lruKey = cache.back().first;

            cacheMap.erase(lruKey);

            cache.pop_back();
        }

        // Insert new element at front
        cache.emplace_front(key, value);

        cacheMap[key] = cache.begin();
    }

    // DELETE
    bool remove(const string& key) {

        auto it = cacheMap.find(key);

        if (it == cacheMap.end()) {
            return false;
        }

        cache.erase(it->second);

        cacheMap.erase(it);

        return true;
    }

    // EXISTS
    bool exists(const string& key) const {

        return cacheMap.find(key) != cacheMap.end();
    }

    // SIZE
    size_t size() const {
        return cache.size();
    }
};

#endif
