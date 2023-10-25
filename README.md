# metapp
An all purpose C++ utility library for metaprogramming, templating, and other questionable things.
* *There are 2 libraries named `metapp` and `metapp-std`. The latter includes and uses the std.*

## Project Usage
* CMakeLists.txt
  ```cmake
  add_subdirectory(.../metapp)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE metapp
  )
  ```
* CMakeLists.txt - [CPM](https://github.com/cpm-cmake/CPM.cmake)
  *Check the repo's latest release tag as I might forget to update this.*
  ```cmake
  CPMAddPackage("gh:u16rogue/metapp#v0.5.0-pre") 
  if (NOT metapp_ADDED)
    message(FATAL_ERROR "Your nice message here!")
  endif()
  target_link_libraries(${PROJECT_NAME}
    PRIVATE metapp
  )
  ```
<hr>

* Source
  ```c++
  #include <metapp/metapp.hh>
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

### (8) CmpHStr and CmpHStrPartial
Compile time hashed string using FNV1A64 with partial hashing.
```c++
CmpHStr("hello") == "hello"; // true
CmpHStr("hello") == "hallo"; // false
CmpHStrPartial("hello") == "hello"; // true
CmpHStrPartial("hello") == "hello world"; // true
```

### (9) Result
Result container that can either contain a value or a failure status.
> [!NOTE]
> If `T` has a custom `->` defined `Result` will automatically chain its own `->` to `T`'s. You can disable this by setting `custom_arrow_operator_chaining` to false on its template parameter. What this does is in the case of `Result<std::unique_ptr<foo>> bar;` doing `bar->` will result to `foo` instead of `std::unique_ptr<foo>`.

```c++
// This is optional if you want to add your own status.
// Having 'SUCCESS = 0' and 'FAILURE = 1' is a requirement.
struct SomeStruct { SomeStruct(int); ... };
enum class MyError { SUCCESS, FAILURE, TOO_LOW };
auto get(int x) -> Result<SomeStruct, MyError> {
  if (x < 10) return MyError::TOO_LOW;
  return x * 2;
}

auto v = get(12);
if (!v) {
  switch (v.why()) {
    ...
  }
  return -1;
}

return v->value + (*v).value;
```
