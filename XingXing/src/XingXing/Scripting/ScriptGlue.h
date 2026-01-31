#pragma once

namespace Hazel {

	/**
	 * @brief 脚本胶水层（Script Glue Layer）
	 * 
	 * ScriptGlue 是连接 C++ 引擎和 C# 脚本的桥梁，负责将引擎功能暴露给脚本。
	 * 
	 * 核心职责：
	 * 1. 注册 InternalCalls：将 C++ 函数注册到 Mono，使 C# 能够调用它们
	 * 2. 数据封送（Marshalling）：在 C++ 和 C# 之间转换数据类型
	 * 3. 组件注册：让脚本能够检测和访问引擎组件
	 * 
	 * 工作原理：
	 * 
	 * C# 端：
	 * ```csharp
	 * // 声明外部函数（标记为 InternalCall）
	 * [MethodImpl(MethodImplOptions.InternalCall)]
	 * internal static extern void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
	 * ```
	 * 
	 * C++ 端：
	 * ```cpp
	 * // 实现函数
	 * static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation) {
	 *     // 实现代码...
	 * }
	 * 
	 * // 注册函数
	 * mono_add_internal_call("Hazel.InternalCalls::TransformComponent_SetTranslation", 
	 *                        TransformComponent_SetTranslation);
	 * ```
	 * 
	 * 这样，C# 脚本就能调用引擎的 C++ 功能了！
	 * 
	 * 注意事项：
	 * - 函数签名必须精确匹配（参数类型、顺序）
	 * - 使用指针传递复杂类型（如 Vector3）
	 * - 字符串使用 MonoString* 类型
	 * - UUID 映射为 C# 的 ulong (uint64_t)
	 */
	class ScriptGlue
	{
	public:
		/**
		 * @brief 注册组件类型
		 * 
		 * 将引擎的所有组件类型注册到脚本系统，使脚本能够：
		 * - 使用 HasComponent<T>() 检测组件
		 * - 访问组件的属性和方法
		 * 
		 * 调用时机：
		 * - 脚本引擎初始化时
		 * - 程序集重载后
		 * 
		 * 实现原理：
		 * 使用模板元编程遍历 AllComponents 列表，为每个组件生成检测函数。
		 */
		static void RegisterComponents();
		
		/**
		 * @brief 注册 C++ 函数到 Mono
		 * 
		 * 将所有引擎 API 函数注册为 InternalCalls，使 C# 能够调用。
		 * 
		 * 注册的函数类别：
		 * - 实体操作：HasComponent, FindEntityByName
		 * - 组件访问：TransformComponent, Rigidbody2DComponent, TextComponent 等
		 * - 输入处理：Input_IsKeyDown
		 * - 调试工具：NativeLog
		 * 
		 * 调用时机：
		 * - 脚本引擎初始化时（仅一次）
		 */
		static void RegisterFunctions();
	};

}
