#pragma once
#include "Proton/Model/Model.h"

#include <filesystem>

namespace Proton
{
    class MaterialLoader
    {
    public:
        static Ref<Material> LoadMaterialEditor(const std::filesystem::path& materialPath);
    };
}