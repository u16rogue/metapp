# metapp
A library for C++ Metaprogramming, template, and other magical stuff under one utility header

## Project Usage
* CMakeLists.txt
```cmake
add_subdirectory(.../metapp)
...
target_link_libraries(
  ${PROJECT_NAME}
  ...
  metapp
)
```
* Source
```c++
#include <metapp/metapp.hpp>
```

## Features

### (0) Fixed data types
* mpp::i8 
* mpp::i16
* mpp::i32
* mpp::i64
* mpp::u8 
* mpp::u16
* mpp::u32
* mpp::u64

### (1) mpp_defer
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

### (2) mpp::array_size
Array size - Determine size of array in n elements
                                                                                                                         
Example Usage:
```c++
char x[256];
auto x = mpp::array_size(x); // x = 256
```

### (3.1) mpp::normalize_lambda_now
Normalize lambda immediately - Allows a lambda with captures to have an ABI function pointer friendly callable function.

**NOTE:** Requires the lambda to be mutable
                                                                                                                         
Example Usage:
```c++
auto invoke(int(*)(int));
invoke(mpp::normalize_lambda_now([&](int x) mutable { return 0; })); // This will not work if you directly pass the lambda
```

### (3.2) mpp::normalize_lambda_from
TBD

### (4.1) mpp::eq_either
TBD

### (4.2) mpp::eq_all
TBD
