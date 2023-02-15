project "Proton-Editor"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Proton/vendor/spdlog/include",
		"%{wks.location}/Proton/src",
		"%{wks.location}/Proton/vendor",
		"%{IncludeDir.Assimp}",
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