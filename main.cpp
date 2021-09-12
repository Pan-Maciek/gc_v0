#include "GC.h"
#define New(klass) gc.young.allocate(klass)

struct Foo {
  Foo* other;
  int primitive;
  static Klass& klass() {
    static Klass klass {
      .name = nameof(Foo),
      .fields = new Field[2] {
        { Type::Reference, nameof(other), offset(Foo, other) },
        { Type::Int, nameof(primitive), offset(Foo, primitive) },
      },
      .fields_count = 2,
      .size = sizeof(Foo)
    };

    return klass;
  }
};

struct Bar {
  Foo* foo1, *foo2, *foo3;
  static Klass& klass() {
    static Klass klass {
      .name = nameof(Bar),
      .fields = new Field[3] {
        { Type::Reference, nameof(foo1), offset(Bar, foo1) },
        { Type::Reference, nameof(foo2), offset(Bar, foo2) },
        { Type::Reference, nameof(foo3), offset(Bar, foo3) },
      },
      .fields_count = 3,
      .size = sizeof(Bar)
    };
    return klass;
  }
};

int main() {
  const size_t youngMemBytes = 256;
  const size_t oldMemBytes = 128;
  GC gc(youngMemBytes, oldMemBytes);

  Ref<Foo> foo1(gc);
  Ref<Foo> foo2(gc);

  (*foo1)->other = *foo2;
  (*foo1)->primitive = 1;

  (*foo2)->primitive = 2;
  (*foo2)->other = *foo1;

  {
    Ref<Bar> bar(gc);
    (*bar)->foo1 = *foo1;
    (*bar)->foo2 = *foo2;

    Foo* foo3 = New(Foo);
    foo3->primitive = 3;
    (*bar) -> foo3 = foo3;
  }

  gc.show();
  gc.phase1();
  gc.show();

  return 0;
}
