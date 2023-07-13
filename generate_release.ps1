pip3 install --upgrade conan
conan --version
conan profile detect --name ssimp_release

# create directories
Remove-Item .\ssimp_release -Recurse -Force
New-Item ssimp_release -type Directory
Set-Location ssimp_release
New-Item  library -type Directory
New-Item  binary -type Directory
Set-Location library
New-Item Debug -type Directory
New-Item Release -type Directory
New-Item include\ssimp -type Directory
Set-Location ..\..\

# build
conan install . --output-folder=build_conan --build=missing -pr:h=ssimp_release -pr:b=ssimp_release -s compiler.cppstd=23
conan install . --output-folder=build_conan --build=missing  -pr:h=ssimp_release -pr:b=ssimp_release -s build_type=Debug -s compiler.cppstd=23

cmake --fresh --preset conan-default -DSSIMP_INLINE_CONFIGS=ON

cmake --build --preset conan-debug
cmake --build --preset conan-release

# copy files
Copy-Item .\build_conan\Release\ssimp.exe -Destination .\ssimp_release\binary
Copy-Item .\build_conan\Release\libssimp.* -Destination .\ssimp_release\library\Release
Copy-Item .\build_conan\Debug\libssimp.* -Destination .\ssimp_release\library\Debug
Copy-Item .\src\application\*.hpp -Destination .\ssimp_release\library\include\ssimp

# create library archive
Set-Location .\ssimp_release\library
Compress-Archive -Path .\Release, .\Debug, .\include -DestinationPath lib_windows.zip
Set-Location ..\..\
