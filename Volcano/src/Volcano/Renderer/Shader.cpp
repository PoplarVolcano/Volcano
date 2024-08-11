#include "volpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Volcano/Platform/OpenGL/OpenGLShader.h"

namespace Volcano {

	Ref<Shader> Shader::Create(const std::string& filepath)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   VOL_CORE_ASSERT(false, "Shader：API为None不支持"); return nullptr;
		case RendererAPIType::OpenGL: return std::make_shared <OpenGLShader>(filepath);
		}

		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}

	Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		switch (RendererAPI::Current())
		{
		case RendererAPIType::None:   VOL_CORE_ASSERT(false, "Shader：API为None不支持"); return nullptr;
		case RendererAPIType::OpenGL: return std::make_shared <OpenGLShader>(name, vertexSrc, fragmentSrc);
		}

		VOL_CORE_ASSERT(false, "Buffer：未知API");
		return nullptr;
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
	{

		VOL_CORE_ASSERT(!Exists(name), "ShaderLibrary:shader已经存在了");
		m_Shaders[name] = shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string filepath)
	{
		auto shader = Shader::Create(filepath);
		Add(shader);
		return shader;
	}


	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		VOL_CORE_ASSERT(Exists(name), "ShaderLibrary:未找到shader");
		return m_Shaders[name];
	}

	bool ShaderLibrary::Exists(const std::string& name)
	{
		return m_Shaders.find(name) != m_Shaders.end();
	}

}