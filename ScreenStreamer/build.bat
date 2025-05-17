@echo off
setlocal

:: Set FFmpeg path
set FFMPEG_ROOT=C:\Users\Administrator\Desktop\ffmpeg-master-latest-win64-gpl-shared

:: Clean and create build directory
if exist build rmdir /s /q build
mkdir build
cd build

:: Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64 -DFFMPEG_ROOT="%FFMPEG_ROOT%"

:: Build the project
cmake --build . --config Release

:: Copy FFmpeg DLLs
echo Copying FFmpeg DLLs...
copy "%FFMPEG_ROOT%\bin\avcodec-62.dll" Release\
copy "%FFMPEG_ROOT%\bin\avformat-62.dll" Release\
copy "%FFMPEG_ROOT%\bin\avutil-60.dll" Release\
copy "%FFMPEG_ROOT%\bin\swscale-9.dll" Release\

echo.
echo Build complete! Running ScreenStreamer...
echo.

:: Run the application
cd Release
ScreenStreamer.exe -i 255.255.255.255

endlocal 