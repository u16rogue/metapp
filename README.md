# metapp
An all purpose C++ utility library for metaprogramming, templating, and other questionable things.
* *There are 2 libraries named `metapp` and `metapp-std`. The latter includes and uses the std.*

## Project Usage
* CMakeLists.txt
  ```cmake
  add_subdirectory(<path...>/metapp)
  target_link_libraries(<target> PRIVATE metapp)
  ```
* CMakeLists.txt - [CPM](https://github.com/cpm-cmake/CPM.cmake)
  ```cmake
  CPMAddPackage("gh:u16rogue/metapp@<tag>") # metapp@0.10.0
  if (NOT metapp_ADDED)
    message(FATAL_ERROR "...")
  endif()
  target_link_libraries(<target> PRIVATE metapp)
  ```
<hr>

* Source
  ```c++
  #include <metapp/metapp.hh>
  #include <metapp/metapp-std.hh>
  ```

## Features

### Fixed data types
* mpp::i8 
* mpp::i16
* mpp::i32
* mpp::i64
* mpp::u8 
* mpp::u16
* mpp::u32
* mpp::u64

### mpp_defer
Defer statement - Executes code on scope exit

Example Usage:
```c++
print(" 1")
int * x = new int;
mpp_defer {
  print(" 2");
  delete x;
};
*x = 1;
print(" 3");
// Prints: 1 3 2
```

### mpp::array_length
Array length - Determine the length of an array's capacity in n elements.

Example Usage:
```c++
char x[256];
auto x = mpp::array_length(x); // x = 256
```

### mpp::normalize_lambda_now
Normalize lambda immediately - Allows a lambda with captures to have an ABI function pointer friendly callable function.

**NOTE:** Requires the lambda to be mutable

Example Usage:
```c++
auto invoke(int(*)(int));
invoke(mpp::normalize_lambda_now([&](int x) mutable { return 0; })); // This will not work if you directly pass the lambda
```

### mpp_lock
Scope based mutex lock - Locks a mutex within a given scope.
```c++
std::mutex x;
int y = 0;
// ...
mpp_lock(x) {
  ++y;
};
```

### CmpHStr and CmpHStrPartial
Compile time hashed string using FNV1A64 with partial hashing.
```c++
CmpHStr("hello") == "hello"; // true
CmpHStr("hello") == "hallo"; // false
CmpHStrPartial("hello") == "hello"; // true
CmpHStrPartial("hello") == "hello world"; // true
```

### Result
Result container that can either contain a value of `T` or a reason enum of `R`.
```c++
enum class Reason { AllocFail, TooLow };
struct Value { int value; };
auto get(int x) -> Result<std::unique_ptr<Value>, Reason> {
    if (x < 10) return Reason::TooLow;
    auto r = std::make_unique<Value>(x * 2);
    if (!r) return Reason::AllocFail;
    return r;
}

auto result = get(12);
if (!result) {
    log(result.valErrTryTransact(
        [&](Reason r){ switch (r) { case...: return "Error: ..."; } },
        []{ return "There is no error"; },
    ));
}

return result.valOkTryTransact(
    [&]{ return result->value; },
    []{ return -1; },
);
```

### mpp::normalize_lambda_from
TBD

### mpp::eq_either
TBD

### mpp::eq_all
TBD

### mpp::impl_iterator<T>
Fully implements C++ iterators with automatic implementation with minimum effort
TBD
