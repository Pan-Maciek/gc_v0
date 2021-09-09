#pragma once
#include "Memory.h"
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

    stack<Object*> stack;
    set<Object*> marked;

    bool is_marked(Object* obj) {
        return marked.contains(obj);
    }

    void mark(Object* obj) {
        marked.insert(obj);
    }

    void mark_all(Object* obj) {
        if (not is_marked(obj)) {
            mark(obj);
            for (auto child : *obj) {
                if (child != nullptr) stack.push(child);
            }
        }
    }

};

void GC::phase1() {
    // 1. Mark objects
    for (auto ref : roots) {
        stack.push(&ref->object());
    }

    while(!stack.empty()) {
        Object* obj = stack.top();
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
    for(auto obj : marked) {
        if (young.contains(obj)) {
            for(int i = 0; i < obj->klass->refcount; i++) {
                if (young.contains(obj->ref(i))) {
                    obj->ref(i) = obj->object(i)->mark + sizeof(Object) + obj->klass->refs[i];
                }
            }
        }
    }
    // 4. Update roots
    for (RefBase* ref : roots) {
        ref->ref = ref->object().mark + sizeof(Object);
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
    for(auto& obj : young) {
        cout << '\t' << obj << endl;
    }
    cout << "Old(" << old.used() << '/' << old.size << "): " << endl;
    for(auto& obj : old)
        cout << '\t' << obj << endl;
    cout << endl;
}

template<typename T>
struct Ref : public RefBase {
    GC* gc;

    explicit Ref(GC& gc) : gc(&gc) {
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