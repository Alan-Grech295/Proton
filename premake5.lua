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

--Include Directories relative to root folder
IncludeDir = {}
IncludeDir["ImGui"] = "%{wks.location}/Proton/vendor/imgui"
IncludeDir["Assimp"] = "%{wks.location}/Proton/vendor/Assimp/include"
IncludeDir["entt"] = "%{wks.location}/Proton/vendor/EnTT/include"
IncludeDir["DirectXTK"] = "%{wks.location}/Proton/vendor/DirectXTK/include"
IncludeDir["DirectXTex"] = 	"%{wks.location}/Proton/vendor/DirectXTex/include"
IncludeDir["yaml_cpp"] = 	"%{wks.location}/Proton/vendor/yaml-cpp/include"

include "Proton/vendor/imgui"
include "Proton/vendor/yaml-cpp"

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
		"%{IncludeDir.DirectXTK}",
		"%{IncludeDir.DirectXTex}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}"
	}

	links
	{
		"ImGui",
		"yaml-cpp"
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
			("%{wks.location}/%{prj.name}/vendor/Assimp/" .. outputdir .. "/assimp-vc142-mtd.lib")
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

	filter { "files:**VS.hlsl" }
   		shadertype "Vertex"

	filter { "files:**CS.hlsl" }
   		shadertype "Compute"

	filter {"files:**Inc.hlsl"}
		flags {"ExcludeFromBuild"}
	

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