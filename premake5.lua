solution "project-zombye"
    configurations { "debug", "release"}
    language "C++"

    includedirs {
        "deps/bullet3/src",
        "deps/jsoncpp/include",
        "deps/gli/include",
        "deps/glm/include",
        "src/include",
        "mesh_converter/include",
        "animation_converter/include"
    }

    libdirs {
        "deps/jsoncpp",
        "deps/bullet3"
    }

    configuration "debug"
        flags {"Symbols"}
        optimize "Off"

    configuration "release"
        optimize "Full"

    project "zombye"
        kind "WindowedApp"

        buildoptions "-std=c++1y"

        files "src/source/zombye/**.cpp"

        defines "GLM_FORCE_RADIANS"

        configuration {"gmake", "windows"}
            buildoptions "-std=gnu++1y"

            includedirs {
                "deps/mingw/SDL2-2.0.3/x86_64-w64-mingw32/include",
                "deps/mingw/SDL2-2.0.3/x86_64-w64-mingw32/include/SDL2",
                "deps/mingw/SDL2_mixer-2.0.0/x86_64-w64-mingw32/include",
                "deps/mingw/glew/include"
            }

            libdirs {
                "deps/mingw/SDL2-2.0.3/x86_64-w64-mingw32/lib",
                "deps/mingw/SDL2_mixer-2.0.0/x86_64-w64-mingw32/lib",
                "deps/mingw/glew/lib/x64"
            }

            defines {"GLEW_STATIC"}

            linkoptions {"-lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -lglew32s -lopengl32"}

            links {"jsoncpp", "bullet3"}

            postbuildcommands {
                "cp deps/mingw/SDL2-2.0.3/x86_64-w64-mingw32/bin/SDL2.dll SDL2.dll",
                "cp deps/mingw/SDL2_mixer-2.0.0/x86_64-w64-mingw32/bin/*.dll ."
            }

        configuration {"gmake", "linux"}
            if _OPTIONS["cc"] == "clang" then
                toolset "clang"
                buildoptions "-stdlib=libc++"
                links "c++"
            end
            links { "GL", "GLEW", "SDL2", "SDL2_mixer", "jsoncpp", "bullet3" }

        configuration {"gmake", "macosx"}
            links { "OpenGL.framework", "GLEW", "SDL2", "SDL2_mixer", "jsoncpp", "bullet3" }

        configuration "debug"
            flags {"FatalWarnings"}
            defines "ZOMBYE_DEBUG"
