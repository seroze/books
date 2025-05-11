
Pointer, Reference definitions:

- object has size, address
- pointer is something that points to a type (object/function)
- pointers can change it's pointee
- references cannot change it's pointee

Object lifetime:

- destructors are called only for object types not pointer types

Eg:

```cpp
#include <string>
#include <iostream>
#include <format>
struct X {
   std::string s;
   X(std::string_view s) : s{ s } {
      std::cout << std::format("X::X({})\n", s);
   }
   ~X(){
      std::cout << std::format("~X::X() for {}\n", s);
   }
};
X glob { "glob" };
void g() {
   X xg{ «g()» };
}
int main() {
   X *p0 = new X{ "p0" };
   [[maybe_unused]] X *p1 = new X{ "p1" }; // will leak
   X xmain{ "main()" };
   g();
   delete p0;
   // oops, forgot delete p1
}

```

output:

```
“X::X(glob)
X::X(p0)
X::X(p1)
X::X(main())
X::X(g())
~X::X() for g()
~X::X() for p0
~X::X() for main()
~X::X() for glob”
```

Note: desturctor for p1 is not called because it's a pointer type. of course once program is terminated os cleans up that memory.

Doubts:
- I was thinking why can't cpp call the constructor for pointer types when they go out of scope ?

   turns out this is a dumb thing to do because consider the below example.

 ```cpp

 X* p1 = new X("A");
 X* p2 = p1;  // Now 2 pointers point to the same object.
 ```

 In this case p2 goes out of scope first (LIFO) and this can use double-free which is problematic.

 Why can't cpp check if memory region is already freed ? This will lead to runtime overhead and cpp follows zero overhead principle. It has to additionally keep track of which memory has been freed.

 In best case a double free can result in a segmentation fault
 In worst case it can silently cause undefined behaviour. Hackers can utilize this.

- the order in which destructors are called for global/static objects is very important.

Object size, alignmet and padding:

- each object occupies storage

```cpp

class B; // forward declaration: there will be a class B
         // at some point in the future
void f(B*); // fine, we know what B is, even if we don't
            // know the details yet, and all object
            // addresses are of the same size
// class D : B {}; // oops! To know what a D is, we have
                   // to know how big a B is and what a
                   // B object contains since a D is a B”

```

static_assert is a compile time check. It's used to check if sizeof(class) is something or if a template is of certain type.

assert is a runtime check.

cpp objects always occupy atleast 1 byte even if there's nothing inside them.

Copy and movement:

Cpp considers six member functions as special. These will be automatically generated for four types unless you take steps to prevent it.

- default constructor
- destructor (called at the end of objects lifetime)
- copy constructor
- copy assignment
- move constructor
- move assignment
