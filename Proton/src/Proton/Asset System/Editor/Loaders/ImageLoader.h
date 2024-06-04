#pragma once
#include "Proton/Renderer/Bindables/Texture.h"

#include <filesystem>

namespace Proton
{
    class ImageLoader
    {
    public:
        static Ref<Texture2D> LoadImageEditor(const std::filesystem::path& imagePath);
    };
}