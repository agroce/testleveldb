1.  install [DeepState](https://github.com/trailofbits/deepstate)

2.  install leveldb, after editing its CMakeLists.txt to add-fsanitize=integer,undefined,address,fuzzer-no-link

editing the CMakeLists.txt thus:

```
@@ -40,6 +40,7 @@ include(CheckCXXSourceCompiles)
 # -Werror is necessary because unknown attributes only generate warnings.
 set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
 list(APPEND CMAKE_REQUIRED_FLAGS -Werror -Wthread-safety)
+list(APPEND CMAKE_REQUIRED_FLAGS -fsanitize=fuzzer-no-link,integer,address,undefined)
 check_cxx_source_compiles("
 struct __attribute__((lockable)) Lock {
 void Acquire() __attribute__((exclusive_lock_function()));
 ```
 may work

3.  edit Makefile to point to good clang and to leveldb

3.  fuzz
