# C++ VLog's API

This branch includes a simple API to use VLog from c++.
The installation process is equivalent to Vlog's, namely:

```
mkdir build
cd build
cmake ..
make
```

Please note that we have modified current VLog's project and add an API on top of it.

## Example

We include one simple example that uses this API. Please check the file `src/apiexamples/01.cpp`.

## Adding and executing new experiments

* Create a new c++ file containing a main method. Let's `src/apiexamples/experiment01.cpp` be the relative path of such file.
* Add the following two lines at the end of `CMakeLists.txt` file
```
add_executable(experiment01 src/apiexamples/experiment01.cpp)
target_link_libraries(experiment01 api)
```
* build the code (Note that the following commands will build only the new experiment)
```
cd build
cmake ..
make
```
* execute the new experiment
```
./experiment01
```
