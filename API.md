# C++ VLog's API

This branch includes a simple API to use VLog from c++.
The installation process is equivalent to Vlog's, namely:

```
mkdir build
cd build
cmake ..
make -j
```

Please note that we have modified current VLog's project and add an API on top of it.

## Example

### code
We include one simple example that shows how to use this API. Please check the file `src/apiexamples/01.cpp`.

### compilation
Let `src/apiexamples/exp02.cpp` be the relative path of a code that uses VLog's API. It is necessary to add the following two lines at the end of the `CMakeLists.txt` file:
```
add_executable(exp02 src/apiexamples/exp02.cpp)
target_link_libraries(exp02 api)
```
Then, compile the project again using
```
cd build
cmake ..
make
```
To execute the experiment, run
```
./exp02
```

Note that:
* When we compile the project second time, only the new experiment is built
