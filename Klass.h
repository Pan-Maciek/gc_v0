#pragma once

#include <iostream>
using namespace std;

#define nameof(x) #x
#define offset(class, field) ((size_t)&((class*)nullptr)->field)

struct Klass;
struct Field {
  Klass* const klass;
  const char* const name;
  size_t const offset;
};

struct Klass {
  const char* const name;
  Field* const fields;
  size_t const fields_count;
  size_t const size;
  bool const primitive;
  ostream& (*const show)(ostream& os, void* obj);
};
#define DefKlass() static Field fields[]; static Klass klass;
#define MakeKlass(T) Klass T::klass { .name = #T, .fields = T::fields, .fields_count = (sizeof(T::fields)/(sizeof(*T::fields))), .size = sizeof(T), .primitive = false, .show = nullptr }

template<typename T>
struct Primitive {
  static Klass klass;
  static ostream& show(ostream& os, void* obj) {
    return os << *(T*) obj;
  }
};
#define MakePrimitiveKlass(T, Name) template<> Klass Primitive<T>::klass { .name = #Name, .fields = nullptr, .fields_count = 0, .size = sizeof(T), .primitive = true, .show = Primitive<T>::show }

MakePrimitiveKlass(int, Int);
MakePrimitiveKlass(float, Float);
MakePrimitiveKlass(double, Double);
MakePrimitiveKlass(char*, String);
