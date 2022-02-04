@echo off
echo.
echo This script will build VCPKG itself (if required) in the ./submodules/vcpkg directory 
echo  and then download and build all packages required for the EAR Production Suite.
echo.
echo Note: This uses the "x64-windows-static" target triplet which will use the most recent
echo  platform toolset you have installed. To use a specific toolset, append the following
echo  command to the ./submodules/vcpkg/triplets/x64-windows-static.cmake file:
echo    set(VCPKG_PLATFORM_TOOLSET "v14*")
echo  [use v140 for VS2015, v141 for VS2017, and v142 for VS2019] 
echo.
pause

IF NOT EXIST ".\submodules\vcpkg\vcpkg.exe" (
     call .\submodules\vcpkg\bootstrap-vcpkg.bat
)

.\submodules\vcpkg\vcpkg.exe install boost:x64-windows-static
for /f "tokens=*" %%x in (response_file.txt) do (
    .\submodules\vcpkg\vcpkg.exe install %%x:x64-windows-static
)

echo.
echo Script Complete.
echo.
echo Use CMAKE with:
echo CMAKE_TOOLCHAIN_FILE=./submodules/vcpkg/scripts/buildsystems/vcpkg.cmake
echo VCPKG_TARGET_TRIPLET=x64-windows-static
echo.
pause