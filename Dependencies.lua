IncludeDir = {}
IncludeDir["ImGui"] = "%{wks.location}/Proton/vendor/imgui"
IncludeDir["ImGuizmo"] = "%{wks.location}/Proton/vendor/ImGuizmo"
IncludeDir["Assimp"] = "%{wks.location}/Proton/vendor/assimp/include"
IncludeDir["entt"] = "%{wks.location}/Proton/vendor/EnTT/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Proton/vendor/yaml-cpp/include"
IncludeDir["filewatch"] = "%{wks.location}/Proton/vendor/filewatch"
IncludeDir["mono"] = "%{wks.location}/Proton/vendor/mono/include"
IncludeDir["DirectXTK"] = "%{wks.location}/Proton/vendor/DirectXTK/include"
IncludeDir["DirectXTex"] = 	"%{wks.location}/Proton/vendor/DirectXTex/include"

LibraryDir = {}
LibraryDir["mono"] = "%{wks.location}/Proton/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["Winmm"] = "Winmm.lib"
Library["Bcrypt"] = "Bcrypt.lib"
Library["WinVersion"] = "Version.lib"