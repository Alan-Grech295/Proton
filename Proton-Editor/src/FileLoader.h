#pragma once
#include "Proton/Scene/Scene.h"

#include <string>
#include <DirectXMath.h>

namespace Proton
{
    // Loads files based on their type
    // Used when dragging and dropping file
    class FileLoader
    {
    public:
        static void LoadFile(const std::string& path, DirectX::XMFLOAT3 position, Scene& activeScene);
    };
}