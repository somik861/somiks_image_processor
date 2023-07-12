# SSIMP - Somik's image processor
This is free-time project developed with motivation to create simple and free image conversion (and light processing) application.

## Current status
At this stage, there is(are) a release(s) of CLI and Library available for both Linux and Windows. These contains the latest functionality in the time of its release.

Note that the functionality is **not** heavily tested. There are some tests done, but that are far from bulletproof testsuit.

# Contributions
## How to
You can contribute to the code by implementing new format/algorithm support. Implementing support for both of them are pretty simillary and contains following steps:

1. Copy header file of other format/algo
2. Change the header file to match your needs (class name, algo/format name, supported types)
3. Create .cpp file and implement all the functions that are required by the header file
4. Do not forget to explicitly instantiate templates for supported types (you can use macro in *common_macro.hpp*)
5. Create config file *name*.json
6. To register your class to the whole application, include your header file and add the class to _registered_{formats, algorithms} in {format, algorithm}_manager.hpp

## Notes:
* Do not include any new 3rd party library in header file (to prevent poluting namespace with macros etc...)
* If you are unsure of something, look to other formats/algorithms (the simplest: testing_sample, split_channels) for needed info
* You may also need to modify CMakeLists.txt and conanfile.txt, which is fine as long as you stick to the simplest changes possible and follow its conventions

## Code rules
### File formating
* all C++ files must be formatted via clang-format using provided configuration
* all python files must be formatted with autopep8
* if you want to ensure this automatically before each commit, you can use pre-commit utility (tutorial below)

### Cross-platform compatibility
* All of the code has to be (at any time) compilable on Windows (using MSVC) and Linux (using clang 16 and gcc 13.0).
* **Compatiblity with clang-16 is suspended due to the bug that does not allow clang to compile libstc++-13 ranges**.
* Do not ever use compiler/os-specific code. (Not even when guarded with macros).

### C++ standard and project rules
* At this point of the time, the project is build with C++23. (Use only features that are compatible with all 3 main compilers)
* Header files start with **#pragma once**.
* For package management, [conan](https://conan.io)  is used
* Do not add any unecessary libraries. You can ofcourse add library implementing new functionality, but it is really not necessary to add for example **PCRE** when regexes are already in **BOOST**. The same stands for **fmt**, we have **std::format**, and therefore **fmt** is here unecessary.
* Never use *new* and *delete*, use smart pointers.
* Never use *typedef*, use *using*.
* Do not try the write super-optimized code, write readable code.
* Never do *using namespace ...* inside header file.
* Never do *using namespace std* anywhere.

### Recomended code style
* namespaces are written in *snake_case*
* classes are written in *CamelCase*
* attributes, methods and functions are written in *snake_case*
* names of private and protected methods and attributes begin *_with_underscore*
* types defined using *using* are in *snake_case* and *ends_with_t*
* template types are either one capital letter, or the same as *using*

example:
``` cpp
namespace working_people {
template <typename T, typename backpack_t>
class PersonWithBag{
    public:
        using item_t = T;
        void run();
        int getAge() const;
    private:
        void _precompute_hash();
        std::string _name;
        int _age;
        backpack_t<T> _stuff;
};
} // working_people
```


## How to setup enviromnent
### Precommit hooks
If you want to setup precommit hooks to automatically control code formating for C++ and python, run this inside the root of the project
``` bash
pip3 install --upgrade pre-commit
pre-commit --version
pre-commit install
```

### Cmake and conan
#### conan
To install dependecies, run:\
(Do not forget to switch cpp version to 23 and select correct compiler inside conan profile).
``` bash
pip3 install --upgrade conan
conan --version
conan profile detect --name sip
vim $( conan profile path sip )
```
Conan config file for windows should look something like this:
```
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=23
compiler.runtime=dynamic
compiler.version=193
os=Windows
```

Conan config file for linux (clang) should look like this:
```
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=23
compiler.libcxx=libstdc++11
compiler.version=16
os=Linux
[buildenv]
CC=clang-16
CXX=clang++-16
```

Conan config file for linux (gcc) should look like this:
```
[settings]
arch=x86_64
build_type=Release
compiler=gcc
compiler.cppstd=23
compiler.libcxx=libstdc++11
compiler.version=13
os=Linux
[buildenv]
CC=gcc-13
CXX=g++-13
```

#### build
**Windows:**

``` bash
conan install . --output-folder=build_conan --build=missing -pr:h=sip -pr:b=sip
conan install . --output-folder=build_conan --build=missing  -pr:h=sip -pr:b=sip -s build_type=Debug

cmake --preset conan-default
```

* Debug: `cmake --build --preset conan-debug`
* Release: `cmake --build --preset conan-release`

**Linux:**

Substitute \<ccomp\> and \<cppconp\> with your chosen compiler, e.g. clang-16 and clang-16++.

* Debug:
``` bash
conan install . --output-folder=build_conan --build=missing  -pr:h=sip -pr:b=sip -s build_type=Debug
CC=<ccomp> CXX=<cppcomp> cmake --preset conan-debug
cmake --build --preset conan-debug
```

* Release:
``` bash
conan install . --output-folder=build_conan --build=missing  -pr:h=sip -pr:b=sip
CC=<ccomp> CXX=<cppcomp> cmake --preset conan-release
cmake --build --preset conan-release
```
