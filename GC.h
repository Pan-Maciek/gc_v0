#pragma once
#include "Memory.h"
#include "ObjectPrinter.h"
#include <stack>
#include <iostream>
#include <set>
#include <vector>
using namespace std;

#define new_ref(_class) allocate_root(_class::klass())
struct GC {
  Memory young;
  Memory old;

  explicit GC(int young, int old) : young(young), old(old) {}

  set<ptr*> roots;
  void phase1();
  void show();
  template<typename T>
  void setValue(void* objectRef, T* field, T value) {
    *field = value;
    if (old.contains((ptr) objectRef) && young.contains((ptr) value)) {
      remembered_set.insert(ObjectHeader::fromRef(objectRef));
    }
  }

private:
  stack<ObjectHeader*> stack;
  set<ObjectHeader*> marked;
  set<ObjectHeader*> remembered_set;

  bool is_marked(ObjectHeader* obj) {
    return marked.contains(obj);
  }

  void mark(ObjectHeader* obj) {
    marked.insert(obj);
  }

  void mark_all(ObjectHeader * obj) {
    if (not is_marked(obj)) {
      mark(obj);
      for (int i = 0; i < obj->klass->fields_count; i++ ) {
        FieldView field = obj->field(i);
        if (field.type == Type::Reference and field.cast<ptr>() != nullptr) {
          stack.push(ObjectHeader::fromFieldReference(field));
        }
      }
    }
  }
};

void GC::phase1() {
  // 1. Mark objects
  for (auto ref : roots) {
    stack.push(ObjectHeader::fromRef(*ref));
  }

  while(!stack.empty()) {
    ObjectHeader* obj = stack.top();
    stack.pop();
    mark_all(obj);
  }

  // 2. Calculate new positions for objects and store them in mark word
  ptr tmp = old.current;
  for(auto obj : marked) {
    if (young.contains(obj->headerPointer())) {
      obj->mark = tmp;
      tmp += obj->size();
    }
  }

  // 3. Iterate rsyoung and marked objects in young and fix refs
  // We should probably add rsyoung to makred set.
  for (auto obj : marked) {
    for (int i = 0; i < obj->klass->fields_count; i++) {
      FieldView fieldView = obj->field(i);
      if (fieldView.type == Type::Reference && young.contains(fieldView.cast<ptr>())) {
        *obj->refPointer(i) = ObjectHeader::fromFieldReference(fieldView)->mark + sizeof(ObjectHeader) + obj->klass->fields[i].offset;
      }
    }
  }

  // 4. Copy marked to old
  for(auto obj : marked) {
    if (young.contains(obj->headerPointer())) {
      old.copy(obj);
    }
  }

  // 5. Update roots
  for (void* ref : roots) {
    *((ptr*)ref) = ObjectHeader::fromRef(ref)->mark + sizeof(ObjectHeader);
  }

  // 6. Clean young
  young.clean();
}

void GC::show() {
    cout << "Young(" << young.used() << '/' << young.size << "): " << endl;
    for(ObjectHeader* obj : young)
      print_verbose(obj, 1);

    cout << "Old(" << old.used() << '/' << old.size << "): " << endl;
    for(ObjectHeader* obj : old)
      print_verbose(obj, 1);

    cout << endl;
}

#define allocate(_class) allocate<_class>(_class::klass())
#define UpdateRef(obj, field, value) gc.setValue<typeof(obj->field)>(obj, &obj->field, value)