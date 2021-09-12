#pragma once

#define Indent(os, indent) for(int i = 0; i < indent; i++) { os << "  "; }

void print_field_view(const FieldView& field, signed int indent) {
  Indent(std::cout, indent)
  switch (field.type) {
    case Type::Reference:
      std::cout << field.name << ": " << "Reference = " << field.cast<void*>();
      break;
    case Type::Int:
      std::cout << field.name << ": " << "Int = " << field.cast<int>();
      break;
  }
  std::cout << std::endl;
}

void print_object_vague(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << obj->klass->name << "(" << obj->objectVoidPointer() << ") { " << std::endl;

  for (FieldView fieldView : *obj) {
    print_field_view(fieldView, indent + 1);
  }

  Indent(std::cout, indent);
  std::cout << "}" << std::endl;
}

void print_object(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << "Object" << "(" << obj->objectVoidPointer() << ") { " << std::endl;

  for (FieldView fieldView : *obj) {
    print_field_view(fieldView, indent + 1);
  }

  Indent(std::cout, indent);
  std::cout << "}" << std::endl;
}

void print_object_header(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << "Header (" << obj->headerVoidPointer() << ") { ";

//  Indent(std::cout, indent + 1);
  std::cout << "Size: " << obj ->size();
//  Indent(std::cout, indent + 1);
  std::cout << ", Mark Word: " << (void*) obj->mark;

//  Indent(std::cout, indent);
  std::cout << " }" << std::endl;
}

void print_verbose(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << obj->klass->name << " { " << std::endl;

  print_object_header(obj, indent + 1);
  print_object(obj, indent + 1);

  Indent(std::cout, indent)
  std::cout << "}" << std::endl;
}

void print_vague(const ObjectHeader* obj, signed int indent) {
  print_object_vague(obj, indent);
}
