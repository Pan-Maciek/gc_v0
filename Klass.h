#pragma once

#include <cstddef>

enum Type {
  Reference = 0,
  Int = 1,
};

struct Field {
  Type type;
  const char* name;
  size_t offset;

  Field(Type type, const char* name, size_t offset) : type(type), name(name), offset(offset) {}
};

struct Klass {
  const char* name;
  Field* fields;
  size_t fields_count;
  size_t size;
};


#define nameof(x) #x
#define offset(class, field) ((size_t)&((class*)nullptr)->field)
