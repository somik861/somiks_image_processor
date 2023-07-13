if gcc --version | head -n 1 | grep -q '12\.'; then
	export CC=gcc
	export CXX=g++
else
	export CC=gcc-13
	export CXX=g++-13
fi

pip3 install --upgrade conan
conan --version
conan profile detect --name ssimp_release

# Create directories
rm -rf ssimp_release
mkdir ssimp_release
cd ssimp_release
mkdir library binary
cd library
mkdir -p Debug Release include/ssimp
cd ../../

# Release builds
conan install . --output-folder=build_conan --build=missing -pr:h=ssimp_release -pr:b=ssimp_release -s compiler.cppstd=23 -s compiler.libcxx=libstdc++11 -s compiler.version=13
cmake --fresh --preset conan-release -DSSIMP_INLINE_CONFIGS=ON
cmake --build --preset conan-release

# copy release builds
cp build_conan/ssimp ssimp_release/binary
cp build_conan/libssimp.so ssimp_release/library/Release

# Debug builds
conan install . --output-folder=build_conan --build=missing -pr:h=ssimp_release -pr:b=ssimp_release -s compiler.cppstd=23 -s compiler.libcxx=libstdc++11 -s compiler.version=13 -s build_type=Debug
cmake --fresh --preset conan-debug -DSSIMP_INLINE_CONFIGS=ON
cmake --build --preset conan-debug

# copy debug builds
cp build_conan/libssimp.so ssimp_release/library/Debug/

# copy include files
cp src/application/*.hpp ssimp_release/library/include/ssimp

# create library archive
cd ssimp_release/library
tar -czvf lib_linux.tar.gz Debug/ Release/ include/
cd ../../
