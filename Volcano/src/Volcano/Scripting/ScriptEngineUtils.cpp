#include "volpch.h"
#include "ScriptEngine.h"

#include "Volcano/Core/Buffer.h"
#include "Volcano/Core/FileSystem.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>

namespace Volcano
{
    namespace Utils
    {
        // C#的float空间名 + 类名是->System.Single，需转换我们自定义的类名称“Float”。
        // 自定义C#类类型名称
        static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
        {
            { "System.Single",      ScriptFieldType::Float      },
            { "System.Double",      ScriptFieldType::Double     },
            { "System.Boolean",     ScriptFieldType::Bool       },
            { "System.Char",        ScriptFieldType::Char       },
            { "System.Int16",       ScriptFieldType::Short      },
            { "System.Int32",       ScriptFieldType::Int        },
            { "System.Int64",       ScriptFieldType::Long       },
            { "System.Byte",        ScriptFieldType::Byte       },
            { "System.UInt16",      ScriptFieldType::UShort     },
            { "System.UInt32",      ScriptFieldType::UInt       },
            { "System.UInt64",      ScriptFieldType::ULong      },
            { "System.String",      ScriptFieldType::String     },

            { "Volcano.Vector2",    ScriptFieldType::Vector2    },
            { "Volcano.Vector3",    ScriptFieldType::Vector3    },
            { "Volcano.Vector4",    ScriptFieldType::Vector4    },
            { "Volcano.Quaternion", ScriptFieldType::Quaternion },
            { "Volcano.Matrix4x4",  ScriptFieldType::Matrix4x4  },

            { "Volcano.Object",           ScriptFieldType::Object           },
            { "Volcano.GameObject",       ScriptFieldType::GameObject       },
            { "Volcano.ScriptableObject", ScriptFieldType::ScriptableObject },
            { "Volcano.Component",        ScriptFieldType::Component        },
            { "Volcano.Transform",        ScriptFieldType::Transform        },
            { "Volcano.Behaviour",        ScriptFieldType::Behaviour        },
            { "Volcano.MonoBehaviour",    ScriptFieldType::MonoBehaviour    }
        };

        /*
        通常在游戏引擎中需要加载两个 DLL：一个由引擎提供，一个包含游戏代码。
        这样引擎开发者可以为用户提供一个安全的 API 来交互。

        将程序集文件以二进制形式读入内存
        mono_image_open_from_data_full：通过内存数据创建一个 镜像（Image）——它代表程序集的元数据结构
        第三个参数告诉 Mono 是否要复制数据。这里传 1，表示 Mono 会将数据复制到其内部缓冲区中
        第四个参数是一个指向 MonoImageOpenStatus 枚举的指针，通过这个值来判断 Mono 是否成功读取数据
        第五个参数是一个布尔值。
        如果设为 true（或 1），表示 Mono 将以“反射模式”加载镜像，这意味着我们可以检查类型，
        但不能运行任何代码。但我们是要运行代码的，所以设为 false（或 0）。

        加载调试符号：
        当 loadPDB 为 true 时，构造对应的 .pdb 文件路径（替换扩展名）。
        如果文件存在，则同样读取其二进制数据，调用 mono_debug_open_image_from_memory 将调试符号信息关联到刚创建的 image 上。
        这样，调试器就能获取源代码行号、局部变量名等信息。
        若找不到 .pdb，则不加载，也不会报错（只是缺少调试信息）。

        mono_assembly_load_from_full：将 MonoImage 转换为 MonoAssembly，并使其成为运行时可执行单元。
        第一个参数是从 Mono 获取的镜像。
        第二个参数只是一个名称，Mono 在打印错误时会用到它。
        第三个参数是 status 变量。如果出错，该函数会写入 status，但此时不应该再有错误了，所以不检查它。
        第四个参数与 mono_image_open_from_data_full 的最后一个参数相同，如果在那里传了 1，这里也应该传 1

        关闭镜像并返回程序集
        */
        MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
        {
            ScopedBuffer fileData = FileSystem::ReadFileBinary(assemblyPath);

            MonoImageOpenStatus status;
            MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), (uint32_t)fileData.Size(), 1, &status, 0);
            VOL_CORE_ASSERT(status == MONO_IMAGE_OK, mono_image_strerror(status));

            // 设置debug
            if (loadPDB)
            {
                std::filesystem::path pdbPath = assemblyPath;
                pdbPath.replace_extension(".pdb");
                if (std::filesystem::exists(pdbPath))
                {
                    ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);
                    mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), (uint32_t)pdbFileData.Size());
                    VOL_CORE_INFO("Loaded PDB：{0} ", pdbPath.generic_string());
                }
            }

            MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
            mono_image_close(image);

            return assembly;
        }

        void PrintAssemblyTypes(MonoAssembly* assembly)
        {
            MonoImage* image = mono_assembly_get_image(assembly);
            // 类型定义表（type definitions table）
            const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
            int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

            for (int32_t i = 0; i < numTypes; i++)
            {
                uint32_t cols[MONO_TYPEDEF_SIZE];
                mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

                const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
                const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

                printf("%s.%s\n", nameSpace, name);
            }
        }

        /*
        获取给定字段的可访问性级别
        首先检索传入字段上设置的所有标志（这些标志不仅存储可访问性数据），
        然后通过位与运算（&）结合 MONO_FIELD_ATTR_FIELD_ACCESS_MASK 掩码，从中提取可访问性数据。
        */
        uint8_t GetFieldAccessibility(MonoClassField* field)
        {
            if (field == nullptr)
                return 0;

            uint8_t accessibility = (uint8_t)Accessibility::None;
            uint32_t accessFlag = mono_field_get_flags(field) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

            switch (accessFlag)
            {
            case MONO_FIELD_ATTR_PRIVATE:
            {
                accessibility = (uint8_t)Accessibility::Private;
                break;
            }
            case MONO_FIELD_ATTR_FAM_AND_ASSEM:
            {
                accessibility |= (uint8_t)Accessibility::Protected;
                accessibility |= (uint8_t)Accessibility::Internal;
                break;
            }
            case MONO_FIELD_ATTR_ASSEMBLY:
            {
                accessibility = (uint8_t)Accessibility::Internal;
                break;
            }
            case MONO_FIELD_ATTR_FAMILY:
            {
                accessibility = (uint8_t)Accessibility::Protected;
                break;
            }
            case MONO_FIELD_ATTR_FAM_OR_ASSEM:
            {
                accessibility |= (uint8_t)Accessibility::Private;
                accessibility |= (uint8_t)Accessibility::Protected;
                break;
            }
            case MONO_FIELD_ATTR_PUBLIC:
            {
                accessibility = (uint8_t)Accessibility::Public;
                break;
            }
            }

            return accessibility;
        }

        /*
        获取给定属性的可访问性级别
        获取属性的可访问性不如从字段获取那么容易。原因在于属性本质上代表两个方法：getter 和 setter。
        调用 mono_property_get_get_method 并传入属性 MonoProperty 来获取 getter 方法的引用
        调用 mono_method_get_flags 并传入 getter 方法以及 nullptr，从该 getter（如果存在）中获取可访问性标志。
        对于 setter，我们只检查它是否是公共的——如果不是，就把 accessibility 变量设为 Accessibility::Private，
        因为如果 setter 是私有的，我们就不能写入该属性。如果没有 setter，同样把可访问性设为 private。
        */
        uint8_t GetPropertyAccessibility(MonoProperty* property)
        {
            if (property == nullptr)
                return 0;

            uint8_t accessibility = (uint8_t)Accessibility::None;

            MonoMethod* propertyGetter = mono_property_get_get_method(property);
            if (propertyGetter != nullptr)
            {
                uint32_t accessFlag = mono_method_get_flags(propertyGetter, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;

                switch (accessFlag)
                {
                case MONO_FIELD_ATTR_PRIVATE:
                {
                    accessibility = (uint8_t)Accessibility::Private;
                    break;
                }
                case MONO_FIELD_ATTR_FAM_AND_ASSEM:
                {
                    accessibility |= (uint8_t)Accessibility::Protected;
                    accessibility |= (uint8_t)Accessibility::Internal;
                    break;
                }
                case MONO_FIELD_ATTR_ASSEMBLY:
                {
                    accessibility = (uint8_t)Accessibility::Internal;
                    break;
                }
                case MONO_FIELD_ATTR_FAMILY:
                {
                    accessibility = (uint8_t)Accessibility::Protected;
                    break;
                }
                case MONO_FIELD_ATTR_FAM_OR_ASSEM:
                {
                    accessibility |= (uint8_t)Accessibility::Private;
                    accessibility |= (uint8_t)Accessibility::Protected;
                    break;
                }
                case MONO_FIELD_ATTR_PUBLIC:
                {
                    accessibility = (uint8_t)Accessibility::Public;
                    break;
                }
                }
            }

            MonoMethod* propertySetter = mono_property_get_set_method(property);
            if (propertySetter != nullptr)
            {
                uint32_t accessFlag = mono_method_get_flags(propertySetter, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;
                if (accessFlag != MONO_FIELD_ATTR_PUBLIC)
                    accessibility = (uint8_t)Accessibility::Private;
            }
            else
            {
                accessibility = (uint8_t)Accessibility::Private;
            }

            return accessibility;
        }

        // CheckMonoError 从给定的 MonoError 结构中提取错误代码和消息，然后记录到控制台
        bool CheckMonoError(MonoError& error)
        {
            bool hasError = !mono_error_ok(&error);
            if (hasError)
            {
                unsigned short errorCode = mono_error_get_error_code(&error);
                const char* errorMessage = mono_error_get_message(&error);
                printf("Mono Error!\n");
                printf("\tError Code: %hu\n", errorCode);
                printf("\tError Message: %s\n", errorMessage);
                mono_error_cleanup(&error);
            }
            return hasError;
        }

        /*
        接受一个 MonoString 指针（指向 C# 托管字符串的指针），并返回一个 std::string
        mono_string_to_utf8_checked 将monoString复制到非托管内存中
        （注意，如果 Mono 函数有“checked”版本，你应该始终使用那个版本，而不要使用未检查的版本。）
        这个函数还返回一个指向缓冲区的指针，我们负责在完成后释放它。请记住这一点。
        */
        std::string MonoStringToUTF8(MonoString* monoString)
        {
            if (monoString == nullptr || mono_string_length(monoString) == 0)
                return "";

            MonoError error;
            char* utf8 = mono_string_to_utf8_checked(monoString, &error);
            if (CheckMonoError(error))
                return "";
            std::string result(utf8);
            mono_free(utf8);
            return result;
        }

        // C#类类型转C++类类型标记
        ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
        {
            std::string typeName = mono_type_get_name(monoType);

            auto it = s_ScriptFieldTypeMap.find(typeName);
            if (it == s_ScriptFieldTypeMap.end())
            {
                VOL_CORE_ERROR("Unknown type: {}", typeName);
                return ScriptFieldType::None;
            }

            return it->second;
        }

    }
}