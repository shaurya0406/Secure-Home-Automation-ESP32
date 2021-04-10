$BuildPath = "build"

cmake.exe -B $BuildPath -DBUILD_TEST:BOOL=ON -A Win32
cmake.exe --build $BuildPath -v --config Release