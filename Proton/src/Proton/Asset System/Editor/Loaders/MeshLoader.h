#pragma once
#include "Proton/Model/Model.h"

#include <filesystem>

namespace Proton
{
    class MeshLoader
    {
    public:
        static Ref<Mesh> LoadMeshEditor(const std::filesystem::path& imagePath);
    };
}