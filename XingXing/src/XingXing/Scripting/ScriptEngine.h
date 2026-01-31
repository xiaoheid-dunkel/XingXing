#pragma once

#include "XingXing/Scene/Scene.h"
#include "XingXing/Scene/Entity.h"

#include <filesystem>
#include <string>
#include <map>

// Mono C API 前置声明
// Mono 是一个开源的 .NET 实现，这些是 Mono 运行时的核心类型
extern "C" {
	typedef struct _MonoClass MonoClass;           // 表示一个 .NET 类的元数据
	typedef struct _MonoObject MonoObject;         // 表示一个托管对象实例
	typedef struct _MonoMethod MonoMethod;         // 表示一个类方法
	typedef struct _MonoAssembly MonoAssembly;     // 表示一个编译后的程序集（DLL）
	typedef struct _MonoImage MonoImage;           // 程序集的内存镜像，包含类型信息
	typedef struct _MonoClassField MonoClassField; // 表示类的一个字段
	typedef struct _MonoString MonoString;         // 表示一个托管字符串
}

namespace Hazel {

	// ==================== 脚本字段类型系统 ====================
	
	/**
	 * @brief 脚本字段类型枚举
	 * 
	 * 定义了脚本系统支持的所有字段类型。这些类型用于在 C++ 和 C# 之间进行数据交换。
	 * 每个类型都映射到 C# 的相应类型（如 System.Int32, Hazel.Vector3 等）。
	 * 
	 * 注意：字段值存储在固定大小的缓冲区中（16字节），因此不支持引用类型（除了Entity）。
	 */
	enum class ScriptFieldType
	{
		None = 0,
		Float, Double,                               // 浮点数类型
		Bool, Char, Byte, Short, Int, Long,         // 整数类型（有符号）
		UByte, UShort, UInt, ULong,                 // 整数类型（无符号）
		Vector2, Vector3, Vector4,                   // 向量类型
		Entity                                       // 实体引用（通过 UUID 存储）
	};

	/**
	 * @brief 脚本字段结构
	 * 
	 * 表示 C# 脚本类中的一个公共字段。存储字段的元数据，包括：
	 * - Type: 字段的数据类型
	 * - Name: 字段的名称
	 * - ClassField: Mono 运行时中该字段的句柄
	 * 
	 * 这些信息用于在编辑器中显示字段，并在运行时读写字段值。
	 */
	struct ScriptField
	{
		ScriptFieldType Type;       // 字段类型
		std::string Name;           // 字段名称
		
		MonoClassField* ClassField; // Mono 运行时字段句柄，用于 get/set 操作
	};

	/**
	 * @brief 脚本字段实例
	 * 
	 * ScriptField（字段元数据）+ 数据存储的组合。
	 * 
	 * 每个 ScriptFieldInstance 包含：
	 * 1. Field: 字段的类型信息（ScriptField）
	 * 2. m_Buffer: 实际存储字段值的内存缓冲区（16字节）
	 * 
	 * 使用场景：
	 * - 在场景文件中序列化脚本字段的值
	 * - 在编辑器中编辑字段值
	 * - 在运行时实例化脚本时恢复字段值
	 * 
	 * 为什么是 16 字节？
	 * 16 字节足以存储最大的内置类型 Vector4（4个float = 16字节）
	 */
	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer)); // 初始化缓冲区为零
		}

		/**
		 * @brief 获取字段值
		 * @tparam T 字段的 C++ 类型
		 * @return 字段值的副本
		 * 
		 * 使用模板确保类型安全。编译时检查类型大小，防止缓冲区溢出。
		 */
		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_Buffer;
		}

		/**
		 * @brief 设置字段值
		 * @tparam T 字段的 C++ 类型
		 * @param value 要设置的新值
		 * 
		 * 将值复制到内部缓冲区。同样使用 static_assert 保证类型安全。
		 */
		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(T));
		}
	private:
		uint8_t m_Buffer[16]; // 16字节缓冲区，存储字段的实际值

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

	// 类型别名：Entity UUID -> 字段名 -> 字段实例
	// 用于存储每个实体的脚本字段值（用于序列化和编辑器）
	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	// ==================== 脚本类包装器 ====================
	
	/**
	 * @brief 脚本类包装器
	 * 
	 * 封装 Mono 的 MonoClass，提供 C++ 友好的接口。
	 * 
	 * 职责：
	 * 1. 表示一个 C# 类（如 MyGame.Player）
	 * 2. 创建该类的实例
	 * 3. 获取和调用类的方法
	 * 4. 管理类的公共字段列表
	 * 
	 * 生命周期：
	 * - 在程序集加载时创建
	 * - 在程序集重载时重新创建
	 * - 由 ScriptEngine 管理
	 */
	class ScriptClass
	{
	public:
		ScriptClass() = default;
		
		/**
		 * @brief 构造函数
		 * @param classNamespace C# 命名空间（如 "MyGame"）
		 * @param className C# 类名（如 "Player"）
		 * @param isCore 是否是核心程序集中的类（Hazel-ScriptCore.dll）
		 * 
		 * 根据命名空间和类名在 Mono 中查找对应的 MonoClass。
		 */
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		/**
		 * @brief 实例化类
		 * @return 新创建的托管对象（MonoObject）
		 * 
		 * 调用 Mono API 创建该类的一个新实例。
		 * 注意：此时对象已分配内存，但构造函数尚未被调用。
		 */
		MonoObject* Instantiate();
		
		/**
		 * @brief 获取方法
		 * @param name 方法名（如 "OnUpdate"）
		 * @param parameterCount 参数个数
		 * @return 方法句柄（MonoMethod*）
		 * 
		 * 通过反射查找类的方法。方法重载通过参数个数区分。
		 */
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		
		/**
		 * @brief 调用方法
		 * @param instance 对象实例
		 * @param method 方法句柄
		 * @param params 参数数组（可选）
		 * @return 方法返回值（如果有）
		 * 
		 * 调用托管方法。如果方法抛出异常，exception 参数会被设置。
		 */
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		/**
		 * @brief 获取所有公共字段
		 * @return 字段名 -> 字段信息的映射
		 */
		const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }
		
	private:
		std::string m_ClassNamespace;  // 命名空间
		std::string m_ClassName;       // 类名

		std::map<std::string, ScriptField> m_Fields; // 所有公共字段

		MonoClass* m_MonoClass = nullptr; // Mono 运行时类句柄

		friend class ScriptEngine;
	};

	// ==================== 脚本实例 ====================
	
	/**
	 * @brief 脚本实例
	 * 
	 * 表示一个 C# 对象的实例，与引擎中的 Entity 一一对应。
	 * 
	 * 职责：
	 * 1. 管理单个脚本对象的生命周期
	 * 2. 调用脚本的生命周期方法（OnCreate、OnUpdate）
	 * 3. 提供字段值的读写接口
	 * 
	 * 生命周期：
	 * 创建 -> InvokeOnCreate() -> InvokeOnUpdate() (每帧) -> 销毁
	 */
	class ScriptInstance
	{
	public:
		/**
		 * @brief 构造函数
		 * @param scriptClass 脚本类信息
		 * @param entity 关联的引擎实体
		 * 
		 * 创建脚本实例并与引擎实体绑定。
		 * 会自动调用 C# Entity 的构造函数，传入实体的 UUID。
		 */
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		/**
		 * @brief 调用 OnCreate 方法
		 * 
		 * 在实体被创建时调用一次，类似 Unity 的 Start()。
		 * 用于初始化脚本状态、缓存组件引用等。
		 */
		void InvokeOnCreate();
		
		/**
		 * @brief 调用 OnUpdate 方法
		 * @param ts 时间步长（delta time），单位：秒
		 * 
		 * 每帧调用一次，类似 Unity 的 Update()。
		 * 用于实现游戏逻辑、处理输入、更新状态等。
		 */
		void InvokeOnUpdate(float ts);

		/**
		 * @brief 获取脚本类信息
		 */
		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		/**
		 * @brief 获取字段值（模板版本）
		 * @tparam T 字段的 C++ 类型
		 * @param name 字段名
		 * @return 字段值
		 * 
		 * 从 C# 对象中读取字段值。
		 * 如果字段不存在或类型不匹配，返回默认构造的值。
		 */
		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);	
			if (!success)
				return T();

			return *(T*)s_FieldValueBuffer;
		}

		/**
		 * @brief 设置字段值（模板版本）
		 * @tparam T 字段的 C++ 类型
		 * @param name 字段名
		 * @param value 新值
		 * 
		 * 修改 C# 对象的字段值。
		 */
		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			SetFieldValueInternal(name, &value);
		}

		/**
		 * @brief 获取托管对象
		 * @return Mono 运行时对象句柄
		 * 
		 * 用于需要直接操作 Mono 对象的高级场景。
		 */
		MonoObject* GetManagedObject() { return m_Instance; }
		
	private:
		// 内部字段读写函数，处理实际的 Mono API 调用
		bool GetFieldValueInternal(const std::string& name, void* buffer);
		bool SetFieldValueInternal(const std::string& name, const void* value);
		
	private:
		Ref<ScriptClass> m_ScriptClass;       // 所属的脚本类

		MonoObject* m_Instance = nullptr;      // C# 对象实例
		MonoMethod* m_Constructor = nullptr;   // Entity 构造函数缓存
		MonoMethod* m_OnCreateMethod = nullptr; // OnCreate 方法缓存
		MonoMethod* m_OnUpdateMethod = nullptr; // OnUpdate 方法缓存

		inline static char s_FieldValueBuffer[16]; // 静态缓冲区，用于临时存储字段值

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;
	};

	// ==================== 脚本引擎（核心管理器）====================
	
	/**
	 * @brief 脚本引擎（静态类）
	 * 
	 * 脚本系统的中央管理器，负责：
	 * 1. Mono 运行时的初始化和关闭
	 * 2. 程序集的加载、卸载和热重载
	 * 3. 脚本类和实例的管理
	 * 4. 实体生命周期事件的分发
	 * 
	 * 设计模式：单例（通过静态方法实现）
	 * 
	 * 使用流程：
	 * 1. Init() - 引擎启动时初始化
	 * 2. LoadAssembly() / LoadAppAssembly() - 加载脚本程序集
	 * 3. OnRuntimeStart() - 场景开始运行
	 * 4. OnCreateEntity() / OnUpdateEntity() - 实体生命周期
	 * 5. ReloadAssembly() - 热重载（可选）
	 * 6. OnRuntimeStop() - 场景停止运行
	 * 7. Shutdown() - 引擎关闭时清理
	 */
	class ScriptEngine
	{
	public:
		// ==================== 生命周期管理 ====================
		
		/**
		 * @brief 初始化脚本引擎
		 * 
		 * 必须在使用脚本系统前调用。执行以下操作：
		 * 1. 初始化 Mono 运行时
		 * 2. 创建应用程序域（AppDomain）
		 * 3. 注册 C++ → C# 函数绑定（ScriptGlue）
		 * 4. 加载核心和应用程序集
		 */
		static void Init();
		
		/**
		 * @brief 关闭脚本引擎
		 * 
		 * 清理所有资源，卸载程序集，关闭 Mono 运行时。
		 * 应在引擎关闭时调用。
		 */
		static void Shutdown();

		// ==================== 程序集管理 ====================
		
		/**
		 * @brief 加载核心脚本程序集
		 * @param filepath 程序集文件路径（通常是 Hazel-ScriptCore.dll）
		 * @return 是否加载成功
		 * 
		 * 核心程序集包含引擎提供的基础类（如 Entity, Vector3, Input 等）。
		 * 必须在加载应用程序集之前调用。
		 */
		static bool LoadAssembly(const std::filesystem::path& filepath);
		
		/**
		 * @brief 加载应用脚本程序集
		 * @param filepath 程序集文件路径（通常是游戏的 DLL）
		 * @return 是否加载成功
		 * 
		 * 应用程序集包含用户编写的游戏脚本。
		 * 加载后会自动扫描所有继承自 Entity 的类。
		 * 同时会启动文件监视器，以支持热重载。
		 */
		static bool LoadAppAssembly(const std::filesystem::path& filepath);
		
		/**
		 * @brief 重新加载程序集（热重载）
		 * 
		 * 卸载当前的应用程序域，然后重新加载所有程序集。
		 * 
		 * 注意事项：
		 * - 所有运行时状态都会丢失
		 * - 需要重新创建脚本实例
		 * - 在脚本文件被修改后自动触发
		 */
		static void ReloadAssembly();

		// ==================== 运行时管理 ====================
		
		/**
		 * @brief 场景开始运行
		 * @param scene 当前场景指针
		 * 
		 * 在场景进入播放模式时调用。
		 * 设置场景上下文，以便脚本能够访问场景中的实体。
		 */
		static void OnRuntimeStart(Scene* scene);
		
		/**
		 * @brief 场景停止运行
		 * 
		 * 在场景退出播放模式时调用。
		 * 清理所有脚本实例，释放场景上下文。
		 */
		static void OnRuntimeStop();

		// ==================== 实体生命周期 ====================
		
		/**
		 * @brief 检查实体类是否存在
		 * @param fullClassName 完整类名（包含命名空间，如 "MyGame.Player"）
		 * @return 类是否存在
		 * 
		 * 用于验证脚本组件引用的类是否有效。
		 */
		static bool EntityClassExists(const std::string& fullClassName);
		
		/**
		 * @brief 创建实体脚本实例
		 * @param entity 要创建脚本的实体
		 * 
		 * 在实体被添加到场景或场景开始运行时调用。
		 * 执行以下操作：
		 * 1. 根据 ScriptComponent 中的类名创建脚本实例
		 * 2. 恢复序列化的字段值
		 * 3. 调用脚本的 OnCreate() 方法
		 */
		static void OnCreateEntity(Entity entity);
		
		/**
		 * @brief 更新实体脚本
		 * @param entity 要更新的实体
		 * @param ts 时间步长（delta time）
		 * 
		 * 每帧调用，用于执行脚本的 OnUpdate() 方法。
		 * 在主游戏循环中对所有带脚本的实体调用。
		 */
		static void OnUpdateEntity(Entity entity, Timestep ts);

		// ==================== 访问器 ====================
		
		/**
		 * @brief 获取当前场景上下文
		 * @return 场景指针
		 * 
		 * 用于 ScriptGlue 中的函数访问场景数据。
		 */
		static Scene* GetSceneContext();
		
		/**
		 * @brief 根据实体 UUID 获取脚本实例
		 * @param entityID 实体的唯一标识符
		 * @return 脚本实例（如果不存在则返回 nullptr）
		 */
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);
		
		/**
		 * @brief 根据类名获取脚本类
		 * @param name 完整类名
		 * @return 脚本类（如果不存在则返回 nullptr）
		 */
		static Ref<ScriptClass> GetEntityClass(const std::string& name);
		
		/**
		 * @brief 获取所有脚本类
		 * @return 类名 -> 脚本类的映射
		 */
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
		
		/**
		 * @brief 获取实体的脚本字段映射
		 * @param entity 实体
		 * @return 字段名 -> 字段实例的映射
		 * 
		 * 用于在编辑器中编辑脚本字段和场景序列化。
		 */
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);
		
		/**
		 * @brief 获取核心程序集的镜像
		 * @return MonoImage 指针
		 * 
		 * 用于在 ScriptGlue 中注册组件类型。
		 */
		static MonoImage* GetCoreAssemblyImage();

		// ==================== 辅助函数 ====================
		
		/**
		 * @brief 根据 UUID 获取托管对象
		 * @param uuid 实体 UUID
		 * @return MonoObject 指针
		 * 
		 * 用于 C# 脚本之间的互相引用。
		 */
		static MonoObject* GetManagedInstance(UUID uuid);

		/**
		 * @brief 创建 Mono 字符串
		 * @param string C 风格字符串
		 * @return MonoString 指针
		 * 
		 * 用于在 C++ → C# 调用中传递字符串参数。
		 */
		static MonoString* CreateString(const char* string);
		
	private:
		// ==================== 内部实现 ====================
		
		/**
		 * @brief 初始化 Mono 运行时
		 * 
		 * 设置程序集搜索路径，初始化 JIT 编译器，创建根域。
		 * 如果启用调试，还会配置调试器代理。
		 */
		static void InitMono();
		
		/**
		 * @brief 关闭 Mono 运行时
		 * 
		 * 卸载应用程序域，清理 JIT 运行时。
		 */
		static void ShutdownMono();

		/**
		 * @brief 实例化 Mono 类
		 * @param monoClass Mono 类句柄
		 * @return 新创建的对象实例
		 * 
		 * 内部辅助函数，封装 Mono API 调用。
		 */
		static MonoObject* InstantiateClass(MonoClass* monoClass);
		
		/**
		 * @brief 加载程序集中的所有类
		 * 
		 * 扫描应用程序集，找出所有继承自 Entity 的类，
		 * 并为每个类创建 ScriptClass 包装器。
		 * 同时提取类的公共字段信息。
		 */
		static void LoadAssemblyClasses();

		friend class ScriptClass;
		friend class ScriptGlue;
	};

	// ==================== 工具函数 ====================
	
	namespace Utils {

		/**
		 * @brief 将脚本字段类型转换为字符串
		 * @param fieldType 字段类型枚举
		 * @return 类型名称字符串
		 * 
		 * 用于日志输出和调试。
		 */
		/**
		 * @brief 将脚本字段类型转换为字符串
		 * @param fieldType 字段类型枚举
		 * @return 类型名称字符串
		 * 
		 * 用于日志输出和调试。
		 */
		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
				case ScriptFieldType::None:    return "None";
				case ScriptFieldType::Float:   return "Float";
				case ScriptFieldType::Double:  return "Double";
				case ScriptFieldType::Bool:    return "Bool";
				case ScriptFieldType::Char:    return "Char";
				case ScriptFieldType::Byte:    return "Byte";
				case ScriptFieldType::Short:   return "Short";
				case ScriptFieldType::Int:     return "Int";
				case ScriptFieldType::Long:    return "Long";
				case ScriptFieldType::UByte:   return "UByte";
				case ScriptFieldType::UShort:  return "UShort";
				case ScriptFieldType::UInt:    return "UInt";
				case ScriptFieldType::ULong:   return "ULong";
				case ScriptFieldType::Vector2: return "Vector2";
				case ScriptFieldType::Vector3: return "Vector3";
				case ScriptFieldType::Vector4: return "Vector4";
				case ScriptFieldType::Entity:  return "Entity";
			}
			HZ_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return "None";
		}

		/**
		 * @brief 将字符串转换为脚本字段类型
		 * @param fieldType 类型名称字符串
		 * @return 字段类型枚举
		 * 
		 * 用于反序列化场景文件中的字段类型。
		 */
		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")    return ScriptFieldType::None;
			if (fieldType == "Float")   return ScriptFieldType::Float;
			if (fieldType == "Double")  return ScriptFieldType::Double;
			if (fieldType == "Bool")    return ScriptFieldType::Bool;
			if (fieldType == "Char")    return ScriptFieldType::Char;
			if (fieldType == "Byte")    return ScriptFieldType::Byte;
			if (fieldType == "Short")   return ScriptFieldType::Short;
			if (fieldType == "Int")     return ScriptFieldType::Int;
			if (fieldType == "Long")    return ScriptFieldType::Long;
			if (fieldType == "UByte")   return ScriptFieldType::UByte;
			if (fieldType == "UShort")  return ScriptFieldType::UShort;
			if (fieldType == "UInt")    return ScriptFieldType::UInt;
			if (fieldType == "ULong")   return ScriptFieldType::ULong;
			if (fieldType == "Vector2") return ScriptFieldType::Vector2;
			if (fieldType == "Vector3") return ScriptFieldType::Vector3;
			if (fieldType == "Vector4") return ScriptFieldType::Vector4;
			if (fieldType == "Entity")  return ScriptFieldType::Entity;

			HZ_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}

	}

}
