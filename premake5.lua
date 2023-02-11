include "Dependencies.lua"

workspace "Proton"
	architecture "x64"
	startproject "Proton-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Proton/vendor/imgui"
include "Proton/vendor/yaml-cpp"
include "Proton/vendor/assimp"
include "Proton-ScriptCore"

project "Proton"
	location "Proton"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

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
		"%{IncludeDir.DirectXTK}",
		"%{IncludeDir.DirectXTex}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.yaml_cpp}"
	}

	links
	{
		"ImGui",
		"yaml-cpp",
		"assimp",

		"%{Library.mono}",
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
			("%{wks.location}/%{prj.name}/vendor/DirectXTK/" .. outputdir .. "/DirectXTK.lib"),
			("%{wks.location}/%{prj.name}/vendor/DirectXTex/" .. outputdir .. "/DirectXTex.lib"),
			"%{Library.WinSock}",
			"%{Library.Winmm}",
			"%{Library.Bcrypt}",
			"%{Library.WinVersion}",
			--("%{wks.location}/%{prj.name}/vendor/assimp/" .. outputdir .. "/assimp-vc142-mtd.lib")
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

	--Shader Output
	filter { "files:**.hlsl" }
   		shadermodel "5.0"

	filter { "files:**PS.hlsl" }
   		shadertype "Pixel"
		shaderobjectfileoutput("%{wks.location}/%{prj.name}/%{file.basename}.cso")

	filter { "files:**VS.hlsl" }
   		shadertype "Vertex"
		shaderobjectfileoutput("%{wks.location}/%{prj.name}/%{file.basename}.cso")

	filter { "files:**CS.hlsl" }
   		shadertype "Compute"
		shaderobjectfileoutput("%{wks.location}/%{prj.name}/%{file.basename}.cso")

	filter {"files:**Inc.hlsl"}
		flags {"ExcludeFromBuild"}
		shaderobjectfileoutput("%{wks.location}/%{prj.name}/%{file.basename}.cso")
	

project "Sandbox"
	location "Sandbox"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

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
		"%{wks.location}/Proton/vendor",
		"%{IncludeDir.entt}"
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



project "Proton-Editor"
	location "Proton-Editor"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

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
		"%{wks.location}/Proton/vendor",
		"%{IncludeDir.DirectXTex}",
		"%{IncludeDir.entt}"
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