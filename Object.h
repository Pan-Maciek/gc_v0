#pragma once

#include "GC.h"
#include "Klass.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

using byte = uint8_t;
using ptr = byte*;

struct FieldView {
  const char* name;
  Type type;
  ptr pointer;

  FieldView(const char* name, Type type, ptr pointer) : name(name), type(type), pointer(pointer) {}

  // reinterpret value
  template<class X>
  X cast() const {
    return *(X*) pointer;
  }


};

struct ObjectHeader {
  ptr mark;
  const Klass* klass;
  ObjectHeader() = delete;

  ptr headerPointer() const {
    return (ptr) this;
  }

  void* headerVoidPointer() const {
    return (void*) this;
  }

  ptr objectPointer() const {
    return headerPointer() + sizeof(ObjectHeader);
  };

  void* objectVoidPointer() const {
    return (void*) objectPointer();
  }

  ptr fieldPointer(int i) const {
    return objectPointer() + klass->fields[i].offset;
  }

  ptr* refPointer(int i) {
    return (ptr*) fieldPointer(i);
  }

  FieldView field(int i) const {
    const Field field = klass->fields[i];
    return { field.name, field.type, fieldPointer(i) };
  }

  size_t size() const {
    return sizeof(ObjectHeader) + klass->size;
  }

  struct Iterator {
    const ObjectHeader* const header;
    size_t fieldCounter;
    Iterator(const ObjectHeader* object, size_t fieldCounter) : header(object), fieldCounter(fieldCounter) {}

    Iterator operator++() {
      fieldCounter++;
      return *this;
    }

    FieldView operator*() const {
      return header->field(fieldCounter);
    }

    bool operator!=(Iterator other) const {
      return fieldCounter != other.fieldCounter;
    }
  };

  Iterator begin() const { return {this, 0}; }
  Iterator end() const { return {this, this->klass->fields_count}; }
};

struct RefBase {
  void* ref;

  ObjectHeader* header() const {
    return (ObjectHeader*) (((ptr) ref) - sizeof(ObjectHeader));
  }
};

ObjectHeader* objectHeader(FieldView& fieldView) {
  return (ObjectHeader*) (fieldView.pointer - sizeof(ObjectHeader));
}
