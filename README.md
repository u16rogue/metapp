# metapp
A library for C++ Metaprogramming, template, and other magical stuff under one utility header
* *There are 2 libraries named `metapp` and `metapp-std`. The latter includes and uses the std/stl.*

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

### (5) mpp::impl_iterator<T>
Fully implements C++ iterators with automatic implementation with minimum effort
TBD

### (6) mpp::task_dispatcher<mpp::default | mpp::signal_complete>
Dispatch tasks on new/different threads
TBD
```c++
static mpp::task_dispatcher<mpp::default> reader; // Value is true while the thread is running and turns false when it ends.
if (!reader && renderer->button("Load file"))
  reader = [&]() { value = read_file(text_input->text()); };
else
  renderer->text("Loading...");
// ----
// Value is true while thread is running and will stay true until the thread ends and `completed()` is called. completed() will remain false while the thread is running until it ends, when the thread ends completed() will return true, the next and consequent calls will return false. completed() will only be true once when the thread is completed.
static mpp::task_dispatcher<mpp::signal_complete> reader; 
static const char * text = "Load file";
renderer->button(text);
if (!reader && renderer->event->clicked())
  reader = [&]() { value = read_file(text_input->text()); };
else
  renderer->text("Loading...");
if (reader.completed())
  value.file_close();
```

### (7) mpp_lock
Scope based mutex lock - Locks a mutex within a given scope.
```c++
std::mutex x;
int y = 0;
// ...
mpp_lock(x) {
  ++y;
};
```

### (8) cmphstr and cmphstr_partial
Compile time hashed string using FNV1A64 with partial hashing.
```c++
cmphstr("hello") == "hello"; // true
cmphstr("hello") == "hallo"; // false
cmphstr_partial("hello") == "hello"; // true
cmphstr_partial("hello") == "hello world"; // true
```
