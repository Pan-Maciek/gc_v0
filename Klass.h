#pragma once

struct Klass {
    const char * name;
    int* refs;
    const char** names;
    int refcount;
    int size;
};

#define nameof(x) #x
#define offset(class, field) ((int)&((class*)nullptr)->field)
