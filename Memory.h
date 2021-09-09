#pragma once
#include <cstdint>
#include <cstdlib>
#include "Object.h"

struct Memory {
    ptr const base;
    ptr const top;
    ptr current;
    const int size;
    explicit Memory(int size) :
        size(size), base((ptr) malloc(size)),
        top(base + size), current(base) { }

    template<typename T>
    T* allocate(const Klass& klass) {
        Object* header = (Object*) current;
        header->klass = &klass;
        header->mark = 0;
        current += sizeof(Object);

        for (int i = 0; i < klass.size; i++)
            current[i] = 0;
        T* t = new(current)(T);
        current += klass.size;

        return t;
    }

    struct Iterator
    {
        ptr current;
        explicit Iterator(ptr ptr) : current(ptr) {}

        Iterator& operator++() {
            Object* header = (Object*) current;
            current += header->klass->size + sizeof(Object);
            return *this;
        }

        Object& operator*() const {
            return *(Object*) current;
        }

        bool operator!=(Iterator other) const {
            return current != other.current;
        }
    };

    Iterator begin() { return Iterator(base); }
    Iterator end() { return Iterator(current); }

    void copy(Object* obj) {
        ptr src = (ptr) obj;
        for (int i = 0; i < obj->size(); i++)
            current[i] = src[i];
        current += obj->size();
    }

    bool contains(void* ref) const { return base <= ref and ref <= top; }
    void clean() { current = base; }
    int used() const { return current - base; }
};
