@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake --build "C:\Users\shash\source\repos\SmartAttendance\out\build\x64-Debug" 2>&1
echo BUILD_EXIT_CODE=%ERRORLEVEL%
