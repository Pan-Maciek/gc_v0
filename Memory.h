#pragma once
#include "Object.h"
#include <cstdint>
#include <cstdlib>

struct Memory {
  ptr const base;
  ptr const top;
  ObjPtr current;
  const size_t size;
  explicit Memory(size_t size) :
    size(size), base((ptr) malloc(size)),
    top(base + size), current({base}) { }

  template<typename T>
  T* allocate(const Klass& klass) {
    // this assumes that only a single thread is performing allocations at a time.
    // either lock or CAS instructions (or both) if we want to support multithreading in the prototype.
    if (current.ptr + sizeof(Object) + klass.size > top) {
      return nullptr;
    }

    auto header = current.object;
    header->klass = &klass;
    header->mark.ptr = nullptr;
    current.ptr += sizeof(Object);

    memset(current.ptr, 0, klass.size);
    T* t = new(current.ptr)(T);
    current.ptr += klass.size;

    return t;
  }

  struct Iterator {
    ObjPtr current;

    Iterator& operator++() {
      current.ptr += current.object->size();
      return *this;
    }

    Object* operator*() const {
      return current.object;
    }

    bool operator!=(Iterator other) const {
      return current.ptr != other.current.ptr;
    }
  };

  Iterator begin() const { return {base}; }
  Iterator end() const { return {current}; }

  void copy(Object* obj) {
    memcpy(current.ptr, obj, obj->size());
    current.ptr += obj->size();
  }

  bool contains(void* ref) const { return base <= ref and ref <= top; }
  bool contains(FieldView view) const { return contains(*view.pointer.ref); }
  void clean() { current.ptr = base; }
  size_t used() const { return current.ptr - base; }
};
