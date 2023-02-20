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

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"



include "Proton/vendor/imgui"
include "Proton/vendor/yaml-cpp"
include "Proton/vendor/assimp"
include "Proton"
include "Proton-ScriptCore"
include "Proton-Editor"
include "Sandbox"