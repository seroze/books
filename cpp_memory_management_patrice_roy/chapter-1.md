
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

Talking about move I had to understand about lvalue & rvalue references

I watched this https://www.youtube.com/watch?v=i_Z_o9T2fNE (Amir Kirsh - CppCon 2024) and https://cbarrete.com/move-from-scratch.html
https://learnmoderncpp.com/2025/01/07/move-semantics-in-modern-c-1/

Basically there are cases where you will be returning copies in certain cases

Eg1:

```cpp

// Without move semantics
std::string s1(1000, 'A'); // A very long string (to avoid SSO)
auto s2 = s1;
```

At runtime the contents of s1 are copied into s2 with s1 still being able to be accessed and modified
independently.

```cpp

// This code moves contents of s1 into s2 and it's very fast compared to the previous version
// With move semantics
std::string s1(1000, 'A');
auto s2 = std::move(s1);
```

Eg2:

```cpp

#include <vector>
#include <string>

std::vector<std::string> createStrings() {
    std::vector<std::string> vec;
    vec.push_back("Hello");
    vec.push_back("World");
    vec.push_back("This");
    vec.push_back("would");
    vec.push_back("copy");
    return vec; // Before C++11, this would often trigger a copy
}

int main() {
    std::vector<std::string> myStrings = createStrings(); // Expensive copy!
    // ...
}
```


```cpp
#include <vector>
#include <string>

std::vector<std::string> createStrings() {
    std::vector<std::string> vec;
    vec.push_back("Hello");
    vec.push_back("World");
    vec.push_back("This");
    vec.push_back("gets");
    vec.push_back("moved");
    return vec; // Modern C++ will move this automatically
}

int main() {
    std::vector<std::string> myStrings = createStrings(); // Efficient move!
    // ...
}
```

Eg3:

```cpp
std::vector<std::string> vec;
std::string temp("Large string content here...");

// This would make a copy:
vec.push_back(temp); // Copy constructor called
```

```cpp
std::vector<std::string> vec;
std::string temp("Large string content here...");

// This moves the content (no copy):
vec.push_back(std::move(temp)); // Move constructor called
// temp is now empty (but in valid state)
```

The goal of move semantics is to have some way to tell the code when to copy and when to move for performance.

I had this question of why not just return the reference of a std::vector<std::string> to avoid all this mess, but there are few problems

```cpp
std::vector<std::string>& createStrings() {  // Notice the &
    std::vector<std::string> vec;  // Local variable
    // ... fill the vector ...
    return vec;  // DANGER: Returning reference to local!
}
```

Eg:
1. If we try to return a reference in your example it will return reference to the local variable. vec is local var and it gets destroyed after function exits.
2. The returned reference is what we call as a "dangling reference" (pointing to destroyed memory)
3. Any use of the returned reference would cause undefined behavour

One other way to deal with this is to refurn reference to a heap allocated object

```cpp
std::vector<std::string>& createStrings() {
    auto* vec = new std::vector<std::string>();  // Heap allocation
    // ... fill the vector ...
    return *vec;  // Returns reference
}
```

Problems:

1. Now you have to manually delete it (memory management headache)
2. Easy to forget to delete, causing memory leaks
3. Doesn't work well with value semantics that C++ favors

One other way is to use output parameters but this has different semantic problems

```cpp
void createStrings(std::vector<std::string>& outVec) {
    // ... fill outVec ...
}
```

Problems:

1. Ugly syntax that breaks composability.
2. Doesn't work well with operator overloading or chaining.
3. Still requires a copy if you want to store the result.

Why Move Semantics Are the Right Solution
Move semantics solve this perfectly by:

1. Keeping clean value semantics
2. Avoiding unnecessary copies
3. Being safe (no dangling references)

Working automatically in many cases

How It Works with Move Semantics:
```cpp
std::vector<std::string> createStrings() {
    std::vector<std::string> vec;
    // ... fill the vector ...
    return vec;  // Compiler will move this automatically
}
```

When you do:

```cpp
auto strings = createStrings();  // No copy, just move
```

The compiler recognizes that:
- vec is about to be destroyed (it's an rvalue).
- It can "steal" the internal resources instead of copying.
- The moved-from object (vec) is left in a valid but unspecified state.

Key Advantages Over Reference Approaches

Safety: No dangling references possible
Clean ownership: The receiver clearly owns the object
Automatic: Works even in complex expressions
Composable: Works with operator overloading, chaining, etc.
Efficient: Often as fast as passing references

Real-World Example Where References Fail:

Consider:

```cpp
auto result = transformStrings(filterStrings(createStrings()));
```

With references, this would be impossible to do safely and efficiently. With move semantics, it:

1. Creates strings in createStrings()
2. Moves them to filterStrings()
3. Moves again to transformStrings()

Finally moves to result

All without any copies or unsafe references.

Why Not Just Use Copy Elision (RVO/NRVO)?
Copy elision (Return Value Optimization) helps in some cases, but:

1. It's not guaranteed by the standard (compiler can choose to do it)
2. Doesn't help in all cases (like storing intermediate results)
3. Move semantics provides a guaranteed, standardized way to avoid copies

Conclusion
The invention of rvalue references and move semantics provided:

1. A safe way to transfer ownership
2. While maintaining clean value semantics
3. Without requiring manual memory management
4. And working in all contexts (not just returns)

References couldn't solve this because they either:

1. Were unsafe (dangling references), or
2. Required manual memory management, or
3. Broke normal value semantics

Move semantics gave us the best of both worlds - the safety and simplicity of value semantics with the efficiency previously only possible with unsafe reference hacking.

For every class there are these 6 basic member functions

1. Default constructor
2. Destructor
3. Copy constructor
4. Copy assignment
5. Move constructor
6. Move assignment

```cpp
#include <iostream>
#include <string>

class Person {
    std::string name;
    int age;
public:
    // 1. Default constructor
    Person() : name("Unknown"), age(0) {
        std::cout << "Default constructor\n";
    }

    // 2. Destructor
    ~Person() {
        std::cout << "Destructor for " << name << "\n";
    }

    // 3. Copy constructor
    Person(const Person& other) : name(other.name), age(other.age) {
        std::cout << "Copy constructor\n";
    }

    // 4. Copy assignment
    Person& operator=(const Person& other) {
        name = other.name;
        age = other.age;
        std::cout << "Copy assignment\n";
        return *this;
    }

    // 5. Move constructor
    Person(Person&& other) noexcept
        : name(std::move(other.name)), age(other.age) {
        other.age = 0;
        std::cout << "Move constructor\n";
    }

    // Constructor with parameters
    Person(std::string n, int a) : name(n), age(a) {}

    void print() const {
        std::cout << name << ", " << age << "\n";
    }
};

int main() {
    std::cout << "--- Default construction ---\n";
    Person p1;  // Default constructor

    std::cout << "\n--- Copy construction ---\n";
    Person p2 = p1;  // Copy constructor

    std::cout << "\n--- Copy assignment ---\n";
    Person p3("Alice", 30);
    p1 = p3;  // Copy assignment

    std::cout << "\n--- Move construction ---\n";
    Person p4 = Person("Bob", 25);  // Move constructor

    std::cout << "\n--- Destructors called ---\n";
    // All destructors called when objects go out of scope
}
```

Q: Why do we need copy assignment ?
Key Difference: Copy construction creates a new object, while copy assignment modifies an existing one.
For classes managing resources (memory, files, etc.), assignment needs special handling:

```cpp
class String {
    char* data;
public:
    // Copy assignment must:
    // 1. Free existing resources
    // 2. Allocate new resources
    // 3. Copy content
    String& operator=(const String& other) {
        if (this != &other) {  // Protect against self-assignment
            delete[] data;     // 1. Free existing
            data = new char[strlen(other.data) + 1]; // 2. Allocate new
            strcpy(data, other.data);  // 3. Copy content
        }
        return *this;
    }
};
```

```cpp
String a, b;
a = b;             // Copy assignment
a = String("tmp"); // Move assignment (different operation)
```

Problematic case
```cpp
class BadString {
    char* data;
    // No copy assignment declared
};

BadString a, b;
a = b;  // Compiler-generated assignment copies the POINTER,
         // now both objects point to same memory!
         // Double-free will occur when both are destroyed
```
Questions:
1. what is the rule of five ?
2. what is noexcept and when do we use those ?
3. why should we implement swap() for simplifying move assignment
4. why do we use T&& for rvalue syntax ?
5. why is corton warning on use of std::move() https://cor3ntin.github.io/posts/move/
