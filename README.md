Differential fuzzing for leveldb and rocksdb.

1.  install [DeepState](https://github.com/trailofbits/deepstate)

2.  install leveldb, after editing its CMakeLists.txt to add -fsanitize=integer,undefined,address,fuzzer-no-link

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

Make sure to use a clang >= 6.0 to compile!

3. install rocksdb, after editing its Makefile to add
-fsanitize=integer,undefined,address,fuzzer-no-link:

```
@@ -232,6 +232,11 @@ ifneq ($(filter -DROCKSDB_LITE,$(OPT)),)
        LUA_PATH =
 endif
 
+DISABLE_JEMALLOC=1
+EXEC_LDFLAGS += -fsanitize=address,integer,undefined,fuzzer-no-link
+PLATFORM_CCFLAGS += -fsanitize=address,integer,undefined,fuzzer-no-link
+PLATFORM_CXXFLAGS += -fsanitize=address,integer,undefined,fuzzer-no-link
+
 # ASAN doesn't work well with jemalloc. If we're compiling with ASAN, we should use regular malloc.
 ifdef COMPILE_WITH_ASAN
        DISABLE_JEMALLOC=1
```

Make sure to use a clang >= 6.0 to compile!

4.  edit this repo's Makefile to point to a >= 6.0 clang, to leveldb,
and to rocksdb.  edit `TestLevelDB.cpp` with locations for the respective databases.

Using a ramdisk for the databases is *STRONGLY* recommended, unless
you want to both fuzz slowly and burn out your SSD.

5.  `make`

`TestBoth` and `TestBoth_LF` are the primary fuzzers.  `TestLevelDB`
just runs leveldb, without differential testing with RocksDB.
