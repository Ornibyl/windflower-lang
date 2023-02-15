if externalincludedirs == nil then
    externalincludedirs = sysincludedirs
end

function default_config_info()
    filter { "configurations:Debug" }
        runtime "Debug"
        optimize "Off"
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }

        runtime "Release"
        optimize "On"
        symbols "On"

    filter { "configurations:Dist" }
        defines { "NDEBUG" }

        runtime "Release"
        optimize "On"
        symbols "Off"

    filter {}
end

function default_build_options()
    filter { "toolset:gcc or clang" }
        buildoptions {
            "-Wall", "-Wextra", "-Wconversion"
        }
    filter {}
end

workspace "windflower-lang"
    configurations { "Debug", "Release", "Dist" }

project "windflower"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}/%{prj.name}"

    files {
        "%{prj.name}/include/**.hpp",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
    }

    includedirs {
        "%{prj.name}/include",
        "%{prj.name}/src"
    }

    default_build_options()
    default_config_info()

project "wftool"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}/%{prj.name}"

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
    }

    externalincludedirs {
        "windflower/include"
    }

    links {
        "windflower"
    }

    default_build_options()
    default_config_info()
