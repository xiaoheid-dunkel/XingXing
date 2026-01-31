#include "hzpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"

#include "XingXing/Core/UUID.h"
#include "XingXing/Core/KeyCodes.h"
#include "XingXing/Core/Input.h"

#include "XingXing/Scene/Scene.h"
#include "XingXing/Scene/Entity.h"

#include "XingXing/Physics/Physics2D.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"

namespace Hazel {

	// ==================== 工具函数 ====================
	
	namespace Utils {

		/**
		 * @brief 将 MonoString 转换为 C++ std::string
		 * @param string Mono 字符串对象
		 * @return C++ 字符串
		 * 
		 * Mono 字符串是 UTF-16 编码的托管对象，需要转换为 UTF-8 C 字符串。
		 * 注意：必须使用 mono_free 释放 Mono 分配的内存。
		 */
		std::string MonoStringToString(MonoString* string)
		{
			char* cStr = mono_string_to_utf8(string);
			std::string str(cStr);
			mono_free(cStr);  // 重要：释放 Mono 分配的内存
			return str;
		}

	}

	// ==================== 组件检测函数映射 ====================
	
	/**
	 * 存储组件类型 -> HasComponent 检测函数的映射
	 * 
	 * 这个映射表用于实现 C# 中的泛型 HasComponent<T>() 方法。
	 * 
	 * 工作原理：
	 * 1. C# 调用 Entity_HasComponent(entityID, typeof(TransformComponent))
	 * 2. 在此映射表中查找 TransformComponent 对应的检测函数
	 * 3. 调用该函数检查实体是否拥有该组件
	 * 
	 * 为什么需要这个映射？
	 * - C# 的泛型无法直接跨越到 C++
	 * - 使用反射类型 + 函数映射实现伪泛型
	 */
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	// 宏：简化 InternalCall 的注册
	// 将函数名转为字符串并添加命名空间前缀
#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name)

	// ==================== InternalCall 函数实现 ====================
	
	/**
	 * @brief 示例：原生日志函数
	 * 
	 * 演示如何从 C# 调用 C++ 函数并传递参数。
	 * C# 端声明：
	 * [MethodImpl(MethodImplOptions.InternalCall)]
	 * internal static extern void NativeLog(string message, int parameter);
	 */
	static void NativeLog(MonoString* string, int parameter)
	{
		std::string str = Utils::MonoStringToString(string);
		std::cout << str << ", " << parameter << std::endl;
	}

	// ==================== 向量操作示例 ====================
	
	/**
	 * @brief 演示如何传递和返回复杂类型（Vector3）
	 * 
	 * 使用指针传递结构体，避免值拷贝。
	 * C# 端使用 ref 关键字：
	 * internal static extern void NativeLog_Vector(ref Vector3 parameter, out Vector3 result);
	 */
	static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
	{
		HZ_CORE_WARN("Value: {0}", *parameter);
		*outResult = glm::normalize(*parameter);
	}

	static float NativeLog_VectorDot(glm::vec3* parameter)
	{
		HZ_CORE_WARN("Value: {0}", *parameter);
		return glm::dot(*parameter, *parameter);
	}

	// ==================== 脚本实例管理 ====================
	
	/**
	 * @brief 获取脚本实例
	 * @param entityID 实体 UUID
	 * @return 托管对象指针
	 * 
	 * 用于 C# 脚本之间的互相引用。
	 * 例如：Entity other = FindEntityByName("Enemy"); 
	 * 返回的是一个已存在的 C# Entity 对象。
	 */
	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	// ==================== 实体操作 ====================
	
	/**
	 * @brief 检查实体是否拥有指定组件
	 * @param entityID 实体 UUID
	 * @param componentType 组件类型（反射类型）
	 * @return 是否拥有该组件
	 * 
	 * 实现 C# 中的泛型方法：bool HasComponent<T>()
	 * 
	 * 工作流程：
	 * 1. 从场景中获取实体
	 * 2. 从反射类型获取 MonoType
	 * 3. 在映射表中查找对应的检测函数
	 * 4. 调用检测函数
	 */
	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		HZ_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	/**
	 * @brief 根据名称查找实体
	 * @param name 实体名称（MonoString）
	 * @return 实体 UUID（如果不存在返回 0）
	 * 
	 * 允许脚本通过名称引用其他实体。
	 * 例如：Entity player = FindEntityByName("Player");
	 */
	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* nameCStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->FindEntityByName(nameCStr);
		mono_free(nameCStr);

		if (!entity)
			return 0;
		
		return entity.GetUUID();
	}

	// ==================== Transform 组件操作 ====================
	
	/**
	 * @brief 获取实体的位置
	 * @param entityID 实体 UUID
	 * @param outTranslation 输出参数，存储位置向量
	 * 
	 * C# 端调用：Vector3 pos = Transform.Translation;
	 * 实际会调用这个函数获取位置。
	 */
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Translation = *translation;
	}

	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}
	
	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, glm::vec2* outLinearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		return Utils::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}
	
	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utils::Rigidbody2DTypeToBox2DBody(bodyType));
	}

	static MonoString* TextComponent_GetText(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return ScriptEngine::CreateString(tc.TextString.c_str());
	}

	static void TextComponent_SetText(UUID entityID, MonoString* textString)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.TextString = Utils::MonoStringToString(textString);
	}

	static void TextComponent_GetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		*color = tc.Color;
	}

	static void TextComponent_SetColor(UUID entityID, glm::vec4* color)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.Color = *color;
	}

	static float TextComponent_GetKerning(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return tc.Kerning;
	}

	static void TextComponent_SetKerning(UUID entityID, float kerning)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.Kerning = kerning;
	}

	static float TextComponent_GetLineSpacing(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		return tc.LineSpacing;
	}

	static void TextComponent_SetLineSpacing(UUID entityID, float lineSpacing)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		HZ_CORE_ASSERT(entity.HasComponent<TextComponent>());

		auto& tc = entity.GetComponent<TextComponent>();
		tc.LineSpacing = lineSpacing;
	}

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	// ==================== 组件注册（模板元编程）====================
	
	/**
	 * @brief 注册单个或多个组件类型
	 * 
	 * 这是一个模板变参函数，使用 C++17 的折叠表达式（fold expression）
	 * 来一次性注册多个组件。
	 * 
	 * 工作原理：
	 * 1. 对每个组件类型 Component：
	 *    a. 提取类型名称（去掉命名空间）
	 *    b. 构造对应的 C# 类型名（如 "Hazel.TransformComponent"）
	 *    c. 在 Mono 中查找该类型
	 *    d. 创建 lambda 函数：[](Entity e) { return e.HasComponent<Component>(); }
	 *    e. 存储到映射表中
	 * 
	 * 这样，C# 就能通过反射类型调用正确的 HasComponent 函数了。
	 */
	template<typename... Component>
	static void RegisterComponent()
	{
		([]()
		{
			// 提取类型名（如 "class Hazel::TransformComponent" -> "TransformComponent"）
			std::string_view typeName = typeid(Component).name();
			size_t pos = typeName.find_last_of(':');
			std::string_view structName = typeName.substr(pos + 1);
			
			// 构造 C# 类型名
			std::string managedTypename = fmt::format("Hazel.{}", structName);

			// 在核心程序集中查找该类型
			MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
			if (!managedType)
			{
				HZ_CORE_ERROR("Could not find component type {}", managedTypename);
				return;
			}
			
			// 注册检测函数
			s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
		}(), ...);  // 折叠表达式，展开所有组件类型
	}

	/**
	 * @brief 重载版本，接受 ComponentGroup
	 * 
	 * 用于支持 AllComponents{} 这种写法。
	 */
	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	// ==================== 公共注册函数 ====================
	
	/**
	 * @brief 注册所有组件
	 * 
	 * 调用 RegisterComponent，传入引擎定义的 AllComponents 列表。
	 * AllComponents 包含所有可用的组件类型（TransformComponent, SpriteRendererComponent 等）。
	 */
	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent(AllComponents{});
	}

	/**
	 * @brief 注册所有 C++ 函数到 Mono
	 * 
	 * 将所有实现的 InternalCall 函数注册到 Mono 运行时。
	 * C# 端使用 [MethodImpl(MethodImplOptions.InternalCall)] 标记的函数
	 * 会在运行时查找这里注册的 C++ 函数。
	 * 
	 * 注册格式：
	 * mono_add_internal_call("完整C#命名空间和类名::方法名", C++函数指针)
	 * 
	 * 例如：
	 * mono_add_internal_call("Hazel.InternalCalls::Entity_HasComponent", Entity_HasComponent)
	 * 
	 * 注意：
	 * - 函数签名必须精确匹配
	 * - 命名空间和类名必须正确
	 * - 函数名区分大小写
	 */
	void ScriptGlue::RegisterFunctions()
	{
		// 调试和测试函数
		HZ_ADD_INTERNAL_CALL(NativeLog);
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector);
		HZ_ADD_INTERNAL_CALL(NativeLog_VectorDot);

		// 脚本实例管理
		HZ_ADD_INTERNAL_CALL(GetScriptInstance);

		// 实体操作
		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);
		HZ_ADD_INTERNAL_CALL(Entity_FindEntityByName);

		// Transform 组件
		HZ_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		
		// Rigidbody2D 组件
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
		
		// Text 组件
		HZ_ADD_INTERNAL_CALL(TextComponent_GetText);
		HZ_ADD_INTERNAL_CALL(TextComponent_SetText);
		HZ_ADD_INTERNAL_CALL(TextComponent_GetColor);
		HZ_ADD_INTERNAL_CALL(TextComponent_SetColor);
		HZ_ADD_INTERNAL_CALL(TextComponent_GetKerning);
		HZ_ADD_INTERNAL_CALL(TextComponent_SetKerning);
		HZ_ADD_INTERNAL_CALL(TextComponent_GetLineSpacing);
		HZ_ADD_INTERNAL_CALL(TextComponent_SetLineSpacing);

		// 输入系统
		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}

}
