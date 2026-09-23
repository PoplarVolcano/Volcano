#include"volpch.h"
#include "OpenGLShader.h"

#include <GLAD/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Volcano/Core/FileSystem.h"
#include "Volcano/Core/Timer.h"
#include "Volcano/Utils/FileUtils.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace Volcano
{
	namespace Utils {

		// 通过字符串获取Shader类型（vertex还是fragment）
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "geometry")
				return GL_GEOMETRY_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;


			VOL_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			VOL_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "Resources/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
			case GL_GEOMETRY_SHADER:  return ".cached_opengl.geom";
			case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
			}
			VOL_CORE_ASSERT(false);
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
			case GL_GEOMETRY_SHADER:  return ".cached_vulkan.geom";
			case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
			}
			VOL_CORE_ASSERT(false);
			return "";
		}

		// 把 GL_VERTEX_SHADER 等转成 shaderc 的 shaderc_shader_kind
		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_GEOMETRY_SHADER: return shaderc_glsl_geometry_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			VOL_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

	}

	OpenGLShader::OpenGLShader(const std::string& filepath, ShaderBackend shaderBackend)
		: m_FilePath(filepath)
	{
		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = FileSystem::ReadFileBinary(filepath).As<char>();
		auto shaderSources = PreProcess(source);

		m_Name = FileUtils::GetFileNameFromPath(filepath);

		switch (shaderBackend)
		{
		case ShaderBackend::OpenGL:
			Compile(shaderSources);
			break;
		case ShaderBackend::Vulkan:
		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			// 显示读取shader的时间
			// 注：每次修改glsl需要删除二进制缓冲文件并重新编译二进制缓冲文件，编译后读取二进制缓冲文件速度特别快
			VOL_CORE_WARN("{0}: Shader creation took {1} ms", m_Name, timer.GetSecondMillis());
			break;
		}
		default:
			Compile(shaderSources);
			break;
		}

	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		//Compile(sources);

		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::UnBind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		UploadUniformFloat3(name, value);
	}
	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		UploadUniformFloat4(name, value);
	}
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));

	}


	// 预处理着色器文件，将文件分解成顶点、几何、片段着色器
	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		//分割标记
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		// 从位置 0 开始，查找字符串 source 中第一次出现 "#type" 的位置。
		// 如果找到了，pos 就是 "#type" 第一个字符的索引；如果没找到，pos 等于 std::string::npos。
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			VOL_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			// 跳过 "#type" 和后面的空格
			size_t begin = pos + typeTokenLength + 1;
			// 提取从 begin 到换行符之间的字符串
			std::string type = source.substr(begin, eol - begin);
			VOL_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			// 从 eol（当前行的换行符）开始，跳过所有连续的换行符，找到下一行第一个非换行字符的位置
			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			// 从 nextLinePos 开始，查找下一个 #type 的位置。
			// 如果找不到，pos 为 std::string::npos。
			pos = source.find(typeToken, nextLinePos);

			// 用类型（如 GL_VERTEX_SHADER）作为键，存储提取的源码。
			shaderSources[Utils::ShaderTypeFromString(type)] =
				(pos == std::string::npos) ? source.substr(nextLinePos, source.find_last_of('}') - nextLinePos + 1) : source.substr(nextLinePos, pos - nextLinePos);
		}
		return shaderSources;
	}

	// 编译着色器，直接把 GLSL 源码丢给 OpenGL 驱动编译
	// 问题：
	// 不同显卡驱动对 GLSL 的解析不一致，行为可能不同；
    // 没法做 SPIR-V 缓存，每次启动都要重新编译；
    // 未来要接 Vulkan 时，OpenGL 的 GLSL 不能复用。
	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		std::vector<GLenum> glShaderIDs;
		// resize，改变 size，改变 capacity，构造新元素
		// reserve，不改变 size，改变 capacity，不构造新元素
		glShaderIDs.reserve(shaderSources.size());

		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);
			
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				VOL_CORE_ERROR("Shader compile failed (type = {0}):", type);
				VOL_CORE_ERROR("{0}", infoLog.data());
				VOL_CORE_ERROR("Source:\n{0}", source);   // 把源码也打出来

				glDeleteShader(shader);

				VOL_CORE_ERROR("{0}", infoLog.data());
				VOL_CORE_ASSERT(false, "Shader compilation failure!");
				break;
			}
			glAttachShader(program, shader);

			// 已经有对象，要放进去	  push_back
			// 要从参数构造新对象	  emplace_back
			// 参数类型可能有歧义	  push_back（更明确）
			// 想要 explicit 构造	  emplace_back
			// 类型构造成本高	      emplace_back（少一次移动）
			// 不确定	              push_back（更安全）
			glShaderIDs.push_back(shader);
		}

		m_RendererID = program;
		
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			VOL_CORE_ERROR("{0}", infoLog.data());
			VOL_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		for (auto id : glShaderIDs)
			glDetachShader(program, id);
	}

	// 本函数会把每个 stage 的 GLSL 编译成 Vulkan SPIR-V 二进制文件，并写缓存（保存在本地）。
	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		//GLuint program = glCreateProgram();

		// 创建编译器Compiler: shaderc提供的glslang编译器
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		// 告诉编译器生成的是给 Vulkan 1.2 用的 SPIR-V
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		// 开启优化（optimize）：性能优先。
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		// 获取二进制缓存文件的缓存目录
		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		// 创建指向 Vulkan Shader 二进制数据的指针并清除
		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		// 遍历并编译每个shader（顶点、几何、像素着色器）， 为其生成一个单独的binary文件
		for (auto&& [stage, source] : shaderSources)
		{
			// 着色器文件路径
			std::filesystem::path shaderFilePath = m_FilePath;
			// 着色器高速缓存路径 = 原 shader 文件名 + 阶段后缀，例如 MyShader.glsl.vert.vulkan.spv。
			// 用 filename() 而不是全路径，避免不同目录的同名 shader 互相覆盖。
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			// 读取高速缓存
			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			// 缓存是否存在,如果有现成的缓存文件, 那么直接读取该文件, 存到data里
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				// SPIR-V 是 32 位字序列
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				// 把 GLSL 编译为SPIR-V, m_FilePath.c_str() 作为错误信息里的文件名
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					VOL_CORE_ERROR(module.GetErrorMessage());
					VOL_CORE_ASSERT(false);
				}

				// 把数据保存到m_VulkanSPIRV
				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				// 把vulkan的 SPIR-V 保存到高速缓存
				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		// 调用反映函数, 显示shader的uniform buffer
		// for (auto&& [stage, data] : shaderData)
		// 	Reflect(stage, data);
	}

	// 把 Vulkan SPIR-V 二进制文件 转换为 OpenGL glsl 源文件字符串
	// 再将 OpenGL glsl 源文件字符串 转换为 OpenGL SPIR-V 二进制文件，并写缓存（保存在本地）
	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

		// 关掉优化。
		// spirv-cross 反编译出来的 GLSL 有时在优化后会触发驱动 bug，或 uniform 名被优化掉导致反射匹配不上。
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_OpenGLSPIRV;
		shaderData.clear();
		m_OpenGLSourceCode.clear();
		// 读取m_VulkanSPIRV中数据
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				// spirv交叉编译器，专门把 SPIR-V 反编译成 GLSL
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				// 编译获得 OpenGL glsl 代码字符串，并保留下来方便调试（比如把生成的 GLSL 打到日志里）
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				// 编译 OpenGL glsl 为 SPIR-V
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					VOL_CORE_ERROR(module.GetErrorMessage());
					VOL_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				// 把opengl的 SPIR-V 写缓存（保存在本地）
				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	// 用 OpenGL API 加载 SPIR-V 并链接
	void OpenGLShader::CreateProgram()
	{
		// 创建着色器程序对象
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			// emplace_back 返回新元素的引用，所以 shaderID 就是刚塞进去的那个。
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			// 告诉 GL 这是 SPIR-V 二进制
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			// 将着色器对象附加给着色器程序对象
			glAttachShader(program, shaderID);
		}

		// 链接着色器程序对象
		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			VOL_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);

			return;
		}

		// 链接成功后不要忘了删除着色器对象
		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	// 诊断函数
	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		VOL_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		VOL_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		VOL_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		VOL_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			VOL_CORE_TRACE("    name = {0}", resource.name);
			VOL_CORE_TRACE("    Size = {0}", bufferSize);
			VOL_CORE_TRACE("    Binding = {0}", binding);
			VOL_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

}