#pragma once
#include <string>
#include <any>
#include "Proton.h"
#include "Proton/Renderer/Bindables/Binds.h"

namespace Proton
{
	//Add bindables as needed
#define BINDABLES \
		X(VertexBuffer)\
		X(IndexBuffer)\
		X(VertexConstantBuffer)\
		X(PixelConstantBuffer)\
		X(Sampler)\
		X(PixelShader)\
		X(VertexShader)\
		X(Texture2D)\
		X(Topology)\
		X(Blender)\
		X(Rasterizer)

	class ConstructableBindable
	{
	public:
		enum class ResourceType {
			NONE,
#define X(type) type,
			BINDABLES
#undef X
		};
	public:
		ConstructableBindable() {}
		ConstructableBindable(ResourceType resourceType, const std::string& tag)
			:
			m_Tag(tag),
			m_ResourceType(resourceType)
		{
		}

		virtual Ref<Bindable> GetRef() = 0;
		
	public:
		Ref<Bindable> m_Bindable = nullptr;

	public:
		std::string m_Tag = "";
		ResourceType m_ResourceType = ResourceType::NONE;
	};

	class SharedBindable : public ConstructableBindable
	{
	public:
		SharedBindable() {}

		SharedBindable(ResourceType resourceType, const std::string& tag) : ConstructableBindable(resourceType, tag) {}
		
		virtual Ref<Bindable> GetRef() override
		{
			PT_CORE_ASSERT(m_Bindable, "Bindable not initialized");
			return m_Bindable;
		}

		template<class T, typename... Args>
		void Initialize(Args&&... args)
		{
			switch (m_ResourceType)
			{
#define X(type) case ResourceType::type: assert("Unspecified type" && (std::is_same<T, type>::value)); break;
				BINDABLES
#undef X
			default:
				assert("Unspecified type" && false);
			}

			m_Bindable = T::Create(m_Tag, std::forward<Args>(args)...);
		}

		template<typename T>
		operator T*()
		{
			return dynamic_cast<T*>(m_Bindable.get());
		}
	};

	class UniqueBindable : public ConstructableBindable
	{
	public:
		UniqueBindable() {}

		UniqueBindable(ResourceType resourceType) : ConstructableBindable(resourceType, "") {}

		template<class T, typename... Args>
		void Initialize(Args&&... args)
		{
			switch (m_ResourceType)
			{
#define X(type) case ResourceType::type: assert("Unspecified type" && (std::is_same<T, type>::value)); break;
				BINDABLES
#undef X
			default:
				assert("Unspecified type" && false);
			}

			m_Bindable = T::CreateUnique(std::forward<Args>(args)...);
		}
		
		virtual Ref<Bindable> GetRef() override
		{
			PT_CORE_ASSERT(m_Bindable, "Bindable not initialized");

			switch (m_ResourceType)
			{
#define X(type) case ResourceType::type: return type::CreateUnique(m_Bindable);
				BINDABLES
#undef X
			}

			assert("Type given is not listed!" && false);
		}
	};
}