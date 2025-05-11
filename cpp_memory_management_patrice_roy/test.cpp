
#include <string>
#include <iostream>
#include <format>

class X {};
class Y {
   X x;
};
int main() {
   static_assert(sizeof(X) > 0);
   static_assert(sizeof(Y) == sizeof(X)); // <-- here
}
