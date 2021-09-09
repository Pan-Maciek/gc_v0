#include "GC.h"

struct Foo {
    Foo* other;
    static Klass& klass() {
        static Klass klass {
            .name = nameof(Foo),
            .refs = new int[]{offset(Foo, other)},
            .names = new const char*[]{"other"},
            .refcount = 1,
            .size = sizeof(Foo)
        };
        return klass;
    }
};

struct Bar {
    Foo* foo1, *foo2;
    static Klass& klass() {
        static Klass klass {
            .name = "Bar",
            .refs = new int[]{offset(Bar, foo1), offset(Bar, foo2)},
            .names = new const char*[]{"foo1","foo2"},
            .refcount = 2,
            .size = sizeof(Bar)
        };
        return klass;
    }
};

//#define assing(x, y, z) { (x)->y = z; post(x, z); }

int main() {
    GC gc(4096, 4096);

    Ref<Foo> foo1(gc);
    Ref<Foo> foo2(gc);

//    assing(*foo1, other, *foo2);
    (*foo1)->other = *foo2;
    (*foo2)->other = *foo1;

    {
        Ref<Bar> bar(gc);
        (*bar)->foo1 = (gc.young.allocate(Foo));
    }

    for (int i = 0; i < 100; i++) {
        gc.young.allocate(Foo);
    }

    gc.show();
    gc.phase1();
    gc.show();

    return 0;
}
