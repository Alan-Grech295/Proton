#pragma once
#include "Scene.h"
#include "Proton\Renderer\EditorCamera.h"
#include "Proton\Core\Core.h"

namespace Proton
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath, const EditorCamera& editorCam);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath, EditorCamera& editorCamera);
		bool DeserializeRuntime(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};
}