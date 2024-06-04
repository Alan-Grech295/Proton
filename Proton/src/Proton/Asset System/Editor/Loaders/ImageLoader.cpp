#include "ptpch.h"
#include "ImageLoader.h"

namespace Proton
{
    Ref<Texture2D> ImageLoader::LoadImageEditor(const std::filesystem::path& imagePath)
    {
        return Texture2D::Create(imagePath.string());
    }
}

