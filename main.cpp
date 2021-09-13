#include "GC.h"

struct Foo {
  Foo* other;
  const char* primitive;
  DefKlass()
};

Field Foo::fields[] = {
  {&Foo::klass, nameof(other), offset(Foo, other)},
  {&Primitive<char*>::klass, nameof(primitive), offset(Foo, primitive)}
};
MakeKlass(Foo);

struct Bar {
  Foo* foo1, *foo2, *foo3;
  DefKlass()
};

Field Bar::fields[] = {
  {&Foo::klass, nameof(foo1), offset(Bar, foo1)},
  {&Foo::klass, nameof(foo2), offset(Bar, foo2)},
  {&Foo::klass, nameof(foo3), offset(Bar, foo3)}
};
MakeKlass(Bar);

#define NewRoot(type, name) type* name = gc.young.allocate<type>(type::klass); gc.roots.insert((ptr*)&(name))
#define New(klass) gc.young.allocate(klass)
#define Delete(ref) gc.roots.erase((ptr*)&(ref))
#define assert(x) cout << #x ": " << ((x) ? "true" : "false") << endl

int main() {
  const size_t youngMemBytes = 256;
  const size_t oldMemBytes = 128;
  GC gc(youngMemBytes, oldMemBytes);

  NewRoot(Foo, foo1);
  foo1->primitive = "foo1";
  gc.phase1();
  gc.show();

  NewRoot(Foo, foo2);

  foo2->primitive = "foo2";
  UpdateRef(foo1, other, foo2);

  NewRoot(Bar, bar);
  UpdateRef(bar, foo1, foo1);
  UpdateRef(bar, foo2, foo2);

  Foo* foo3 = New(Foo);
  foo3->primitive = "foo3";
  UpdateRef(bar, foo3, foo3);
  Delete(bar);

  gc.show();
  gc.phase1();
  gc.show();

  assert(gc.old.contains(foo1));
  assert(gc.old.contains(foo2));
  assert(foo1->other == foo2);
  assert(foo2->other == nullptr);

  return 0;
}
