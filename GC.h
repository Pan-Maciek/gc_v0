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

  set<RefBase*> roots;
  void phase1();
  void show();

private:
  stack<ObjectHeader*> stack;
  set<ObjectHeader*> marked;

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
        FieldView field = obj ->field(i);
        if (field.type == Type::Reference) {
          auto header = objectHeader(field);
          printVerbose(obj, 0);
          stack.push(objectHeader(field));
        }
      }
    }
  }
};

void GC::phase1() {
//     1. Mark objects
  for (auto ref : roots) {
    stack.push(ref->header());
  }

  while(!stack.empty()) {
    ObjectHeader* obj = stack.top();
    stack.pop();
    mark_all(obj);
  }

  // 2. Calculate new positions for objects and store them in mark word
  ptr tmp = old.current;
  for(auto obj : marked) {
    if (young.contains(obj)) {
      obj->mark = tmp;
      tmp += obj->size();
    }
  }

    // 3. Iterate rsyoung and marked objects in young and fix refs
  for (auto obj : marked) {
    if (young.contains(obj)) {
      for(int i = 0; i < obj->klass->fields_count; i++) {
        FieldView fieldView = obj->field(i);
        if (young.contains(fieldView.cast<void*>())) {
          *obj->refPointer(i) = objectHeader(fieldView)->mark + sizeof(ObjectHeader) + obj->klass->fields[i].offset;
        }
      }
    }
  }
    // 4. Update roots
    for (RefBase* ref : roots) {
      ref->ref = ref->header()->mark + sizeof(ObjectHeader);
    }

    // 4. Copy marked to old
    for(auto obj : marked) {
      if (young.contains(obj)) {
        old.copy(obj);
      }
    }

    // 5. Clean young
    young.clean();
}

void GC::show() {
    cout << "Young(" << young.used() << '/' << young.size << "): " << endl;
    for(ObjectHeader* obj : young)
      printVerbose(obj, 1);

    cout << "Old(" << old.used() << '/' << old.size << "): " << endl;
    for(ObjectHeader* obj : old)
      printVerbose(obj, 1);

    cout << endl;
}

template<typename T>
struct Ref : public RefBase {
  GC* gc;

  explicit Ref(GC& gc) : RefBase(), gc(&gc) {
    this->ref = gc.young.template allocate<T>(T::klass());
    gc.roots.insert(this);
  }

  T* operator*() {
    return (T*) ref;
  }

  ~Ref() {
    gc->roots.erase(this);
  }
};

#define allocate(_class) allocate<_class>(_class::klass())