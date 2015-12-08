#include <iostream>
#include "shim/shim_registration.h"

using namespace std;

struct Foo
{
    int func(bool, int) { return 0; }
};

int some_func(bool, int, int) { return 0; }

int main()
{
    auto L = luaL_newstate();

    // std::function<int(bool, int, int)> fn = some_func;
    std::function<int(Foo&, bool, int)> fn = std::mem_fn(&Foo::func);

    Shim::open_class<Foo>("Foo", L)
        .add_function("some_func", fn);
        // .add_function("some_func", some_func);

    return 0;
}
