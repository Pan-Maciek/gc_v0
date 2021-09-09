#pragma once
#include <iostream>
#include "Klass.h"
#include "GC.h"
#include <cassert>

using byte = uint8_t;
using ptr = byte*;

struct Object {
    ptr mark;
    const Klass* klass;
    Object() = delete;

    ptr& ref(int i) const {
        return *(ptr*) (((ptr) this) + sizeof (Object) + klass->refs[i]);
    }

    Object* object(int i) const {
        ptr _ref = ref(i);
        return _ref == nullptr ? nullptr : (Object*)(_ref - sizeof(Object));
    }

    int size() const {
        return sizeof(Object) + klass->size;
    }

    struct Iterator {
        Object* const object;
        int ref;
        Iterator(Object* object, int ref) : object(object), ref(ref) {}

        Iterator& operator++() {
            ref++;
            return *this;
        }

        Object* operator*() const {
            return object->object(ref);
        }

        bool operator!=(Iterator other) const {
            return ref != other.ref;
        }
    };

    Iterator begin() { return {this, 0}; }
    Iterator end() { return {this, this->klass->refcount}; }
};

std::ostream& operator<<(std::ostream& os, const Object& obj)
{
    os << obj.klass->name << "(0x" << &obj << ") { ";
    for(int i = 0; i < obj.klass->refcount-1; i++)
        os << obj.klass->names[i] << ": 0x" << (void*) obj.object(i) << ", ";
    if (obj.klass->refcount > 0)
        os << obj.klass->names[obj.klass->refcount - 1] << ": 0x" << (void*) obj.object(obj.klass->refcount - 1) << " }";
    return os;
}
struct RefBase {
    void* ref;
    Object& object() {
        return *(Object*) (((ptr) ref) - sizeof(Object));
    }
};
