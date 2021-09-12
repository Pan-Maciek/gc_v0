#pragma once

#define Indent(os, indent) for(int i = 0; i < indent; i++) { os << "\t"; }

void printFieldView(const FieldView& field, signed int indent) {
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

void printObjectVague(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << obj->klass->name << "(" << obj->objectVoidPointer() << ") { " << std::endl;

  for (FieldView fieldView : *obj) {
    printFieldView(fieldView, indent + 1);
  }

  Indent(std::cout, indent);
  std::cout << "}" << std::endl;
}

void printObject(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << "Object" << "(" << obj->objectVoidPointer() << ") { " << std::endl;

  for (FieldView fieldView : *obj) {
    printFieldView(fieldView, indent + 1);
  }

  Indent(std::cout, indent);
  std::cout << "}" << std::endl;
}

void printObjectHeader(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << "Header (" << obj->headerVoidPointer() << ") { " << std::endl;

  Indent(std::cout, indent + 1);
  std::cout << "Size: " << obj ->size() << std::endl;
  Indent(std::cout, indent + 1);
  std::cout << "Mark Word: " << (void*) obj->mark << std::endl;

  Indent(std::cout, indent);
  std::cout << "}" << std::endl;
}

void printVerbose(const ObjectHeader* obj, signed int indent) {
  Indent(std::cout, indent)
  std::cout << obj->klass->name << " { " << std::endl;

  printObjectHeader(obj, indent + 1);
  printObject(obj, indent + 1);

  Indent(std::cout, indent)
  std::cout << "}" << std::endl;
}

void printVague(const ObjectHeader* obj, signed int indent) {
  printObjectVague(obj, indent);
}
