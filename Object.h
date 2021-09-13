#pragma once
#include "GC.h"
#include "Klass.h"

using ptr = char*;

struct Object;
union ObjPtr {
  ptr ptr, *ref;
  Object* object;
  void* voidptr;
};

struct FieldView {
  const Field field;
  ObjPtr const pointer;

  const char* name() const { return field.name; }
  bool is_primitive() const { return field.klass->primitive; }
  bool is_ref() const { return !is_primitive(); }
  ostream &show(ostream & os) const {
    if (is_ref() && field.klass->show == nullptr) {
      return os << "0x" << (void*) *pointer.ref;
    } else {
      return field.klass->show(os, pointer.voidptr);
    }
  }
};

/**
 * This class should not be constructed directly!
 * Class represents a runtime object.
 * Object consists of const size header and data segment dependant on underlying type.
 * | Header { mark, klass } | Data { depends on klass } |
 */
struct Object final {
  ObjPtr mark;
  const Klass* klass;

  /**
   * Primary way of constructing Object from existing reference.
   * @param ref Pointer to Object data
   */
  static Object* fromRef(void* ref) {
    return (Object*)(((ptr)ref) - sizeof(Object));
  }
  Object() = delete;

  /**
   * Pointer to Object data segment
   */
  ObjPtr dataPointer() const {
    return {((ptr)this) + sizeof(Object)};
  }

  /**
   * Direct pointer to nth object field.
   */
  ObjPtr field(size_t n) const {
    return {dataPointer().ptr + klass->fields[n].offset};
  }

  FieldView fieldView(size_t i) const {
    return { klass->fields[i], this->field(i) };
  }

  /**
   * Size of Object (header + data)
   */
  size_t size() const {
    return sizeof(Object) + klass->size;
  }

  struct Iterator {
    const Object* const header;
    size_t fieldCounter;

    Iterator operator++() {
      fieldCounter++;
      return *this;
    }

    FieldView operator*() const {
      return header->fieldView(fieldCounter);
    }

    bool operator!=(Iterator other) const {
      return fieldCounter != other.fieldCounter;
    }
  };

  Iterator begin() const { return {this, 0}; }
  Iterator end() const { return {this, this->klass->fields_count}; }


  static Object* fromFieldView(FieldView& fieldView) {
    return (Object*) (fieldView.pointer.ptr - sizeof(Object));
  }

  static struct PrintingConfigType {
    bool showHeader;
    struct {
      bool showTypes, showNames, showValues;
      bool showPrimitives, showReferences;
      const char* separator;
      const char* lastSeparator;
    } fields;
  } PrintingConfig;
};

Object::PrintingConfigType Object::PrintingConfig {
  .showHeader = false,
  .fields = {
    .showTypes = false,
    .showNames = true,
    .showValues = true,
    .showPrimitives = true,
    .showReferences = true,
    .separator = ", ",
    .lastSeparator = " "
  }
};
