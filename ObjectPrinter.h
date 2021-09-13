#pragma once

ostream& operator<<(ostream& os, Object* obj)
{
  os << obj->klass->name << "(0x" << obj->dataPointer().voidptr << ")";
  if (Object::PrintingConfig.showHeader) {
    os << "[mark: 0x" << obj->mark.voidptr << "]";
  }
  const auto& fields = Object::PrintingConfig.fields;
  if (fields.showPrimitives || fields.showReferences) {
    os << "{ ";
    unsigned int field = 0, last = obj->klass->fields_count-1;
    for (; last > 0; last--) {
      if (fields.showReferences and !obj->klass->fields[last].klass->primitive)
        break;
      if (fields.showPrimitives and obj->klass->fields[last].klass->primitive)
        break;
    }
    for (auto fieldView : *obj) {
      if (fieldView.is_primitive() and !fields.showPrimitives)
        continue;
      if (fieldView.is_ref() and !fields.showReferences)
        continue;

      if (fields.showNames) os << fieldView.name() << " = ";
      else os << "[" << fieldView.field.offset << "] = ";

      if (fields.showTypes) os << fieldView.field.klass->name;
      if (fields.showValues) {
        if (fields.showTypes) os << "(";
        fieldView.show(os);
        if (fields.showTypes) os << ")";
      }
      os << (field++ == last ? fields.lastSeparator : fields.separator);
    }
    os << "}";
  }
  return os;
}

