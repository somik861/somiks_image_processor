# Somik's image processor
This is free-time project developed with motivation to create simple and free image conversion (and light processing) application.

## Current status
At this stage, there is really nothing to look at. The app, directory, files and all of the stuff are in WIP state.\
At the later time, there will be some instruction on how to contribute to the project and what can or should be done. If you are interested, you may enable some notifications, but as I said above, this is free-time project and I really do not have any deadlines, plans or time schedules, so there are really no guarantess when (or if) this app will be usable.

# Contributions
There is nothing to contribute to yet :D all the info about rules is just for me to be consistent.

## Code rules
### File formating
* all C++ files must be formatted via clang-format using provided configuration
* all python files must be formatted with autopep8
* if you want to ensure this automatically before each commit, you can use pre-commit utility (tutorial below)

### Cross-platform compatibility
* All of the code has to be (at any time) compilable on Windows (using MSVC) and Linux (using clang).
* Other compilers will become compatible once they implement new C++ standard features.
* Do not ever use compiler/os-specific code. (Not even when guarded with macros).

### C++ standard and project rules
* At this point of the time, the project is build with C++20 (and once C++23 will be usable, it will migrate).
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
```
pip3 install --upgrade pre-commit
pre-commit --version
pre-commit install
```

### Cmake and conan
#### conan
To install dependecies, run:\
(Do not forget to switch cpp version to 20 and select correct compiler inside conan profile).
```
pip3 install --upgrade conan
conan --version
conan profile detect --name sip
vim $( conan profile path sip )

conan install . --output-folder=build_conan --build=missing -pr=sip
conan install . --output-folder=build_conan --build=missing  -pr=sip -s build_type=Debug
```
Conan config file for windows should look something like this:
```
[settings]
arch=x86_64
build_type=Release
compiler=msvc
compiler.cppstd=20
compiler.runtime=dynamic
compiler.version=193
os=Windows
```

Conan config file for linux should look like this:
```
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=20
compiler.libcxx=libstdc++11
compiler.version=15
os=Linux
```

#### build
For windows:
```
cmake --preset conan-default
cmake --build --preset conan-debug
cmake --build --preset conan-release
```

For linux:
```
cmake --preset conan-debug
cmake --build --preset conan-debug
```
