#pragma once
#include "Object.h"
#include <cstdint>
#include <cstdlib>

struct Memory {
  ptr const base;
  ptr const top;
  ptr current;
  const size_t size;
  explicit Memory(size_t size) :
    size(size), base((ptr) malloc(size)),
    top(base + size), current(base) { }

  template<typename T>
  T* allocate(const Klass& klass) {
    // this assumes that only a single thread is performing allocations at a time.
    // either lock or CAS instructions (or both) if we want to support multithreading in the prototype.
    if (current + sizeof(ObjectHeader) + klass.size > top) {
      return nullptr;
    }

    auto* header = (ObjectHeader*) current;
    header->klass = &klass;
    header->mark = nullptr;
    current += sizeof(ObjectHeader);

    for (int i = 0; i < klass.size; i++)
      current[i] = 0;
    T* t = new(current)(T);
    current += klass.size;

    return t;
  }

  struct Iterator {
    ptr current;
    explicit Iterator(ptr ptr) : current(ptr) {}

    Iterator& operator++() {
      ObjectHeader * header = (ObjectHeader *) current;
      current += header->klass->size + sizeof(ObjectHeader);
      return *this;
    }

    ObjectHeader* operator*() const {
      return (ObjectHeader*) current;
    }

    bool operator!=(Iterator other) const {
      return current != other.current;
    }
  };

  Iterator begin() { return Iterator(base); }
  Iterator end() { return Iterator(current); }

  void copy(ObjectHeader* obj) {
    ptr src = (ptr) obj;
    for (int i = 0; i < obj->size(); i++)
      current[i] = src[i];
    current += obj->size();
  }

  bool contains(ptr ref) const { return base <= ref and ref <= top; }
  void clean() { current = base; }
  size_t used() const { return current - base; }
};
