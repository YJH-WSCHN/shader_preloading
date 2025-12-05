set xxd=.\tools\xxd.exe
set codes=.\shaders\codes
set output=.\include\utils\shaders

call :compile_file shader.vert vertex.shader vertex_shader.h
call :compile_file shader.frag fragment.shader fragment_shader.h
exit /b 0

:compile_file
    REM compile the glsl code
    glslc "%codes%\%1" -o %2
    "%xxd%" -i %2 > "%output%\%3.tmp"
    del %2

    REM add header file protection
    set header_protector=%~n3

    for %%i in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
        call set header_protector=%%header_protector:%%i=%%i%%
    )

    set header_protector=%header_protector%_H_
    set tab="    "
    set tab=%tab:"=%

    @echo off
    setlocal enabledelayedexpansion
    (
        echo //Made by Han_feng
        echo.
        echo #ifndef %header_protector%
        echo #define %header_protector%
        echo.
        echo namespace vulkan {

        for /f "usebackq delims=" %%i in ("%output%\%3.tmp") do (
            set line=%%i
            if "!line:~0,8!"=="unsigned" (
                echo %tab%inline %%i
            ) else (
                echo %tab%%%i
            )
        )

        echo }
        echo.
        echo #endif // %header_protector%
    ) > "%output%\%3"
    del "%output%\%3.tmp"

    exit /b