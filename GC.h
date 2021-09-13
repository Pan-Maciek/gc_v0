#pragma once
#include "Memory.h"
#include "ObjectPrinter.h"
#include <stack>
#include <set>

struct GC {
  Memory young, old;

  explicit GC(int young, int old) : young(young), old(old) {}

  set<ptr*> roots;
  void phase1();
  void show();
  template<class T>
  void setValue(void* objectRef, T* field, T value) {
    *field = value;
    if (old.contains((ptr) objectRef) && young.contains((ptr) value)) {
      remembered_set.insert(Object::fromRef(objectRef));
    }
  }

private:
  stack<Object*> stack;
  set<Object*> marked;
  set<Object*> remembered_set;

  bool is_marked(Object* obj) {
    return marked.contains(obj);
  }

  void mark(Object* obj) {
    marked.insert(obj);
  }

  void mark_all(Object* obj) {
    if (not is_marked(obj)) {
      mark(obj);
      for (int i = 0; i < obj->klass->fields_count; i++ ) {
        FieldView field = obj->fieldView(i);
        if (field.is_ref() and field.pointer.ptr != nullptr) {
          auto ref = Object::fromFieldView(field);
          if (!is_marked(ref)) {
            stack.push(ref);
          }
        }
      }
    }
  }
};

void GC::phase1() {
  // 1. Mark objects
  for (auto ref : roots) {
    stack.push(Object::fromRef(*ref));
  }

  while(!stack.empty()) {
    Object* obj = stack.top();
    stack.pop();
    mark_all(obj);
  }

  // 2. Calculate new positions for objects and store them in mark word
  ptr tmp = old.current.ptr;
  for(auto obj : marked) {
    if (young.contains(obj)) {
      obj->mark.ptr = tmp;
      tmp += obj->size();
    }
  }

  // 3. Iterate rsyoung and marked objects in young and fix refs
  // We should probably add rsyoung to makred set.
  for (auto obj : marked) {
    if (young.contains(obj)) {
      for (int i = 0; i < obj->klass->fields_count; i++) {
        FieldView fieldView = obj->fieldView(i);
        if (fieldView.is_ref() and young.contains(fieldView)) {
          *obj->field(i).ref = Object::fromRef(*fieldView.pointer.ref)->mark.ptr + sizeof(Object) + obj->klass->fields[i].offset;
        }
      }
    }
  }
  for (auto obj : remembered_set) {
    for (int i = 0; i < obj->klass->fields_count; i++) {
      FieldView fieldView = obj->fieldView(i);
      if (fieldView.is_ref() and young.contains(fieldView)) {
        *obj->field(i).ref = Object::fromRef(*fieldView.pointer.ref)->mark.ptr + sizeof(Object) + obj->klass->fields[i].offset;
      }
    }
  }

  // 4. Copy marked to old
  for(auto obj : marked) {
    if (young.contains(obj)) {
      old.copy(obj);
    }
  }

  // 5. Update roots
  for (void* ref : roots) {
    *((ptr*)ref) = Object::fromRef(*(ptr*)ref)->mark.ptr + sizeof(Object);
  }

  // 6. Clean young
  young.clean();
  marked.clear();
  remembered_set.clear();
}

void GC::show() {
    cout << "Young(" << young.used() << '/' << young.size << "): " << endl;
    for(Object* obj : young)
      cout << '\t' << obj << endl;

    cout << "Old(" << old.used() << '/' << old.size << "): " << endl;
    for(Object* obj : old)
      cout << '\t' << obj << endl;

    cout << endl;
}

#define allocate(_class) allocate<_class>(_class::klass)
#define UpdateRef(obj, field, value) gc.setValue<>(obj, &obj->field, value)