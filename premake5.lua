workspace "Proton"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Include Directories relative to root folder
IncludeDir = {}
IncludeDir["ImGui"] = "%{wks.location}/Proton/vendor/imgui"
IncludeDir["Assimp"] = "%{wks.location}/Proton/vendor/Assimp/include"

include "Proton/vendor/imgui"
include "Proton/vendor/Assimp"

project "Proton"
	location "Proton"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ptpch.h"
	pchsource "Proton/src/ptpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hlsl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/vendor/DirectXTK/include",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Assimp}"
	}

	links
	{
		"ImGui",
		"assimp"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PT_PLATFORM_WINDOWS",
			"PT_BUILD_DLL"
		}

		links
		{
			("%{wks.location}/%{prj.name}/vendor/DirectXTK/" .. outputdir .. "/DirectXTK.lib")
		}

	filter "configurations:Debug"
		defines "PT_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PT_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PT_DIST"
		runtime "Release"
		optimize "on"
	
project "Sandbox"
	location "Sandbox"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Proton/vendor/spdlog/include",
		"%{wks.location}/Proton/src",
		"%{wks.location}/Proton/vendor"
	}

	links
	{
		"Proton"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"PT_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "PT_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PT_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PT_DIST"
		runtime "Release"
		optimize "on"