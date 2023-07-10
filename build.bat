@ECHO OFF

SET PDB_SUFFIX=%RANDOM%
SET CommonCompilerFlags=/EHsc /EHa- /MTd /nologo /GR- /Od /Oi /WX /W4 /we4388 /wd4201 /wd4100 /wd4189 /wd4505 ^
        /DHOKI_DEV=1 /DHOKI_SLOW=1 /DHOKI_SOUND=1 ^
        /FC /D_MBCS /Z7 /utf-8 /MP
SET ExternalIncludes=/I "../3rdparty/"
SET PlatformCompilerFlags=
SET CommonLinkerFlags=/INCREMENTAL:NO /OPT:REF /debug:fastlink
SET GameLinkerFlags=
SET RendererLinkerFlags=opengl32.lib 
SET PlatformLinkerFlags=user32.lib gdi32.lib winmm.lib xaudio2.lib opengl32.lib
SET GameExportFlags=/EXPORT:GameMain /EXPORT:GameGetSoundSamples
SET RendererExportFlags=/EXPORT:RendererMain /EXPORT:RendererLoadExtensions

IF NOT EXIST build mkdir build && xcopy /E res build\res\
cd build

DEL /Q "game_*.pdb" "renderer_*.pdb" 1> NUL 2>NUL

PUSHD "..\glsl\"
FOR %%F IN ("*") DO CALL :SHADER_PROCESS %%F
POPD

SET FAILED = 0
cl %ExternalIncludes% %CommonCompilerFlags% %GameLinkerFlags%  "..\game\game_main.cpp" /LD /link %CommonLinkerFlags% /PDB:game_%PDB_SUFFIX%.pdb %GameExportFlags%
if %ERRORLEVEL% neq 0 (SET FAILED=%ERRORLEVEL%)
cl %ExternalIncludes% %CommonCompilerFlags% %RendererLinkerFlags% "..\ogl\ogl_main.cpp" /LD /link %CommonLinkerFlags% /PDB:renderer_%PDB_SUFFIX%.pdb %RendererExportFlags%
if %ERRORLEVEL% neq 0 (SET FAILED=%ERRORLEVEL%)
cl %ExternalIncludes% %CommonCompilerFlags% %PlatformCompilerFlags% "..\windows\win_main.cpp" /link %CommonLinkerFlags% %PlatformLinkerFlags%
if %ERRORLEVEL% neq 0 (SET FAILED=%ERRORLEVEL%)
GOTO EOF

:SHADER_PROCESS
SET FILENAME=%~1
SET SOURCE_PATH="%~1"
SET INTERMEDIARY_FILE=%FILENAME:~0,-5%.i
SET DEST_PATH_GL="..\res\shaders\preprocessed\gl\%FILENAME%"
SET DEST_PATH_GLES="..\res\shaders\preprocessed\gles\%FILENAME%"
cl /I "incl" /EP /P %SOURCE_PATH%
MOVE /Y %INTERMEDIARY_FILE% %DEST_PATH_GL%
cl /I "incl" /EP /DGLES=1 /P %SOURCE_PATH%
MOVE /Y %INTERMEDIARY_FILE% %DEST_PATH_GLES%

:EOF
EXIT /B %FAILED%