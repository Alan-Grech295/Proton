project "Proton"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ptpch.h"
	pchsource "src/ptpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hlsl"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
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
			"vendor/DirectXTK/" .. outputdir .. "/DirectXTK.lib",
			"vendor/DirectXTex/" .. outputdir .. "/DirectXTex.lib",
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
		shaderobjectfileoutput("%{file.basename}.cso")

	filter { "files:**VS.hlsl" }
   		shadertype "Vertex"
		shaderobjectfileoutput("%{file.basename}.cso")

	filter { "files:**CS.hlsl" }
   		shadertype "Compute"
		shaderobjectfileoutput("%{file.basename}.cso")

	filter {"files:**Inc.hlsl"}
		flags {"ExcludeFromBuild"}
		shaderobjectfileoutput("%{file.basename}.cso")
	