# 📚 XingXing 脚本系统教程

## 目录
1. [概述](#概述)
2. [架构设计](#架构设计)
3. [核心组件](#核心组件)
4. [工作原理](#工作原理)
5. [使用示例](#使用示例)
6. [高级主题](#高级主题)

---

## 概述

### 什么是脚本系统？

XingXing 引擎的脚本系统是一个基于 **Mono/.NET** 的 C# 脚本运行环境。它允许游戏开发者使用 C# 语言编写游戏逻辑，而无需重新编译整个引擎。这种设计带来了以下优势：

- 🔄 **热重载**：修改脚本后无需重启引擎即可看到效果
- 🎯 **易于使用**：C# 是一门现代化、易学的编程语言
- 🔌 **与引擎分离**：脚本层与引擎层清晰分离，便于维护
- 🛠 **强大的调试支持**：支持通过调试器附加到脚本进行调试

### 脚本系统在引擎中的位置

```
游戏逻辑 (C# 脚本)
        ↓
脚本系统 (ScriptEngine)
        ↓
引擎核心 (C++ 代码)
        ↓
操作系统/硬件
```

---

## 架构设计

### 整体架构

脚本系统采用 **桥接模式**，将 C++ 引擎层与 C# 脚本层连接起来：

```
┌─────────────────────────────────────────┐
│         C# 脚本层 (用户代码)             │
│  - 游戏逻辑脚本 (继承自 Entity)         │
│  - 组件访问和操作                        │
└─────────────────────────────────────────┘
                    ↕
┌─────────────────────────────────────────┐
│        ScriptGlue (胶水层)               │
│  - InternalCalls: C++ → C# 函数绑定     │
│  - 类型转换和数据封送                    │
└─────────────────────────────────────────┘
                    ↕
┌─────────────────────────────────────────┐
│      ScriptEngine (脚本引擎核心)         │
│  - Mono 运行时管理                       │
│  - 程序集加载和卸载                      │
│  - 脚本类和实例管理                      │
└─────────────────────────────────────────┘
                    ↕
┌─────────────────────────────────────────┐
│         引擎核心 (C++ 代码)              │
│  - Scene, Entity, Components             │
│  - 渲染器、物理引擎等                    │
└─────────────────────────────────────────┘
```

### 关键设计模式

1. **单例模式**：`ScriptEngine` 使用静态方法，确保全局唯一的脚本引擎实例
2. **工厂模式**：`ScriptClass` 负责创建和管理脚本类实例
3. **代理模式**：`ScriptInstance` 作为 C++ Entity 的脚本代理
4. **观察者模式**：文件监视器自动检测脚本程序集的变化并触发热重载

---

## 核心组件

### 1. ScriptEngine（脚本引擎）

**职责**：脚本系统的中央管理器

**主要功能**：
- 初始化和关闭 Mono 运行时
- 加载和管理 C# 程序集（Assembly）
- 维护脚本类和实例的映射关系
- 支持程序集热重载

**生命周期**：
```cpp
ScriptEngine::Init();           // 引擎启动时初始化
ScriptEngine::LoadAssembly();   // 加载核心脚本库
ScriptEngine::LoadAppAssembly(); // 加载游戏脚本
// ... 游戏运行 ...
ScriptEngine::ReloadAssembly(); // 热重载脚本
// ... 继续运行 ...
ScriptEngine::Shutdown();       // 引擎关闭时清理
```

### 2. ScriptClass（脚本类）

**职责**：表示一个 C# 类的 C++ 包装器

**主要功能**：
- 封装 Mono 的 `MonoClass` 对象
- 提供创建类实例的接口
- 管理类的字段（Field）信息
- 提供方法调用的接口

**使用示例**：
```cpp
ScriptClass playerClass("MyGame", "Player");
MonoObject* instance = playerClass.Instantiate();
MonoMethod* updateMethod = playerClass.GetMethod("OnUpdate", 1);
```

### 3. ScriptInstance（脚本实例）

**职责**：表示一个 C# 对象的实例，与引擎中的 Entity 一一对应

**主要功能**：
- 管理单个脚本对象的生命周期
- 调用脚本的生命周期方法（OnCreate、OnUpdate）
- 提供字段值的读写接口

**生命周期**：
```
Entity 创建 → ScriptInstance 创建 → InvokeOnCreate()
                                          ↓
                           每帧更新 ← InvokeOnUpdate(deltaTime)
                                          ↓
                           Entity 销毁 → 实例销毁
```

### 4. ScriptGlue（胶水层）

**职责**：连接 C++ 引擎和 C# 脚本的桥梁

**主要功能**：
- 注册 C++ 函数供 C# 调用（InternalCalls）
- 处理类型转换和数据封送（Marshalling）
- 注册组件类型，使脚本能够访问引擎组件

**典型的绑定函数**：
```cpp
// C++ 端定义
static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
{
    // 实现...
}

// 注册到 Mono
HZ_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

// C# 端声明和调用
[MethodImpl(MethodImplOptions.InternalCall)]
internal static extern void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);
```

### 5. ScriptField（脚本字段）

**职责**：表示脚本类中的一个公共字段

**主要功能**：
- 存储字段的类型信息（int, float, Vector3 等）
- 提供字段值的读写能力
- 支持编辑器中的序列化和反序列化

**支持的字段类型**：
- 基本类型：`Float`, `Double`, `Bool`, `Char`, `Int`, `Long` 等
- 向量类型：`Vector2`, `Vector3`, `Vector4`
- 引用类型：`Entity`（用于引用场景中的其他实体）

---

## 工作原理

### 1. 初始化流程

```
引擎启动
   ↓
ScriptEngine::Init()
   ↓
InitMono() - 初始化 Mono 运行时
   ↓
创建 MonoDomain (应用程序域)
   ↓
LoadAssembly() - 加载核心脚本库 (Hazel-ScriptCore.dll)
   ↓
LoadAppAssembly() - 加载游戏脚本程序集
   ↓
LoadAssemblyClasses() - 扫描并加载所有脚本类
   ↓
ScriptGlue::RegisterFunctions() - 注册 C++ → C# 绑定
   ↓
ScriptGlue::RegisterComponents() - 注册组件类型
   ↓
准备就绪，等待运行时调用
```

### 2. 脚本实例化流程

当场景中的 Entity 拥有 `ScriptComponent` 时：

```
场景开始运行 (OnRuntimeStart)
   ↓
遍历所有带 ScriptComponent 的 Entity
   ↓
对每个 Entity 调用 ScriptEngine::OnCreateEntity()
   ↓
根据类名查找 ScriptClass
   ↓
创建 ScriptInstance
   ↓
Instantiate() - 创建 C# 对象实例
   ↓
调用 Entity 构造函数，传入 UUID
   ↓
复制序列化的字段值到实例
   ↓
调用脚本的 OnCreate() 方法
   ↓
脚本实例准备就绪
```

### 3. 每帧更新流程

```
主循环每帧
   ↓
Scene::OnUpdateRuntime(deltaTime)
   ↓
遍历所有带脚本的 Entity
   ↓
对每个 Entity 调用 ScriptEngine::OnUpdateEntity()
   ↓
获取对应的 ScriptInstance
   ↓
调用 instance->InvokeOnUpdate(deltaTime)
   ↓
执行 C# 脚本中的 OnUpdate() 方法
   ↓
脚本可以调用引擎 API 修改 Entity 状态
```

### 4. 程序集热重载流程

```
检测到脚本 DLL 文件修改 (FileWatcher)
   ↓
触发 OnAppAssemblyFileSystemEvent
   ↓
设置 AssemblyReloadPending = true
   ↓
提交到主线程执行
   ↓
卸载当前 AppDomain
   ↓
创建新的 AppDomain
   ↓
重新加载核心库和游戏程序集
   ↓
重新扫描和加载脚本类
   ↓
重新注册组件
   ↓
如果场景正在运行，重新创建脚本实例
   ↓
热重载完成
```

---

## 使用示例

### 示例 1：创建一个简单的脚本

**C# 脚本 (Player.cs)**:
```csharp
using Hazel;

namespace MyGame
{
    public class Player : Entity
    {
        public float Speed = 5.0f;
        
        void OnCreate()
        {
            Console.WriteLine("Player created!");
        }
        
        void OnUpdate(float ts)
        {
            Vector3 translation = Transform.Translation;
            
            if (Input.IsKeyDown(KeyCode.W))
                translation.Y += Speed * ts;
            if (Input.IsKeyDown(KeyCode.S))
                translation.Y -= Speed * ts;
            if (Input.IsKeyDown(KeyCode.A))
                translation.X -= Speed * ts;
            if (Input.IsKeyDown(KeyCode.D))
                translation.X += Speed * ts;
            
            Transform.Translation = translation;
        }
    }
}
```

### 示例 2：访问组件

```csharp
public class Shooter : Entity
{
    void OnUpdate(float ts)
    {
        // 访问刚体组件
        if (HasComponent<Rigidbody2DComponent>())
        {
            Vector2 impulse = new Vector2(10.0f, 0.0f);
            Rigidbody2DComponent.ApplyLinearImpulse(impulse, true);
        }
        
        // 访问文本组件
        if (HasComponent<TextComponent>())
        {
            TextComponent.Text = "Score: " + score;
        }
    }
}
```

### 示例 3：查找其他实体

```csharp
public class Enemy : Entity
{
    private Entity player;
    
    void OnCreate()
    {
        // 根据名称查找玩家实体
        player = FindEntityByName("Player");
    }
    
    void OnUpdate(float ts)
    {
        if (player != null)
        {
            // 追踪玩家位置
            Vector3 direction = player.Transform.Translation - Transform.Translation;
            direction = Vector3.Normalize(direction);
            Transform.Translation += direction * 2.0f * ts;
        }
    }
}
```

---

## 高级主题

### 1. Mono 运行时

**什么是 Mono？**

Mono 是一个开源的 .NET 框架实现，允许在非 Windows 平台上运行 .NET 应用程序。XingXing 使用 Mono 的嵌入式 API 将 C# 运行时集成到 C++ 引擎中。

**关键概念**：
- **MonoDomain**：应用程序域，类似于沙箱，可以加载和卸载程序集
- **MonoAssembly**：编译后的 .NET 程序集（DLL 文件）
- **MonoImage**：程序集的元数据和代码镜像
- **MonoClass**：类的元数据表示
- **MonoObject**：类的实例对象

### 2. 数据封送（Marshalling）

在 C++ 和 C# 之间传递数据时，需要进行类型转换：

**简单类型**（直接传递）：
- C++ `float` ↔ C# `float`
- C++ `int` ↔ C# `int`
- C++ `bool` ↔ C# `bool`

**复杂类型**（需要转换）：
- C++ `glm::vec3*` ↔ C# `ref Vector3`
- C++ `std::string` ↔ C# `MonoString*`
- C++ `UUID` ↔ C# `ulong`

**示例**：
```cpp
// C++ 字符串 → C# 字符串
MonoString* CreateString(const char* string)
{
    return mono_string_new(s_Data->AppDomain, string);
}

// C# 字符串 → C++ 字符串
std::string MonoStringToString(MonoString* string)
{
    char* cStr = mono_string_to_utf8(string);
    std::string str(cStr);
    mono_free(cStr);
    return str;
}
```

### 3. 字段序列化

脚本字段可以在编辑器中编辑并序列化到场景文件：

**流程**：
1. 扫描脚本类的公共字段
2. 存储字段类型和名称
3. 在编辑器中显示字段编辑器
4. 用户修改字段值
5. 将字段值序列化到场景文件（YAML）
6. 运行时反序列化并应用到脚本实例

**限制**：
- 字段大小不能超过 16 字节（通过 `m_Buffer[16]` 限制）
- 只支持预定义的字段类型（见 `ScriptFieldType` 枚举）

### 4. 调试支持

脚本系统支持通过 Visual Studio 或其他 C# 调试器进行调试：

**启用调试**：
```cpp
#ifdef HZ_DEBUG
    bool EnableDebugging = true;
#else
    bool EnableDebugging = false;
#endif
```

**调试器配置**：
- 监听地址：`127.0.0.1:2550`
- 传输协议：`dt_socket`
- 支持软断点（Soft Breakpoints）

**使用方法**：
1. 在 Visual Studio 中设置断点
2. 附加到进程（Attach to Process）
3. 选择 Mono 调试引擎
4. 连接到 `127.0.0.1:2550`

### 5. 性能考虑

**优化建议**：
- 避免在 `OnUpdate` 中频繁进行字符串操作
- 缓存常用的组件引用，避免每帧查询
- 尽量减少跨语言调用的次数
- 使用对象池减少 GC 压力

**示例**：
```csharp
// ❌ 不好的做法
void OnUpdate(float ts)
{
    if (HasComponent<Rigidbody2DComponent>())
    {
        // 每帧都检查组件
    }
}

// ✅ 好的做法
private bool hasRigidbody;

void OnCreate()
{
    hasRigidbody = HasComponent<Rigidbody2DComponent>();
}

void OnUpdate(float ts)
{
    if (hasRigidbody)
    {
        // 使用缓存的结果
    }
}
```

### 6. 内存管理

**C++ 端**：
- 使用智能指针（`Ref<T>`, `Scope<T>`）管理脚本对象
- 避免悬空指针和内存泄漏

**C# 端**：
- Mono 使用垃圾回收（GC）
- 避免创建过多临时对象
- 注意循环引用

---

## 常见问题

### Q1: 为什么要使用 C# 而不是 Lua 或其他脚本语言？

**A**: C# 提供以下优势：
- 强类型，减少运行时错误
- 完善的 IDE 支持和调试工具
- 丰富的标准库
- 与引擎的互操作性好
- 性能优于解释型脚本语言

### Q2: 热重载时会丢失运行时数据吗？

**A**: 是的。热重载会卸载整个程序集并重新加载，所有运行时状态都会丢失。如果需要保留状态，需要在重载前序列化关键数据。

### Q3: 可以在脚本中使用第三方 C# 库吗？

**A**: 可以，但需要确保：
- 库兼容 Mono 运行时
- 将库的 DLL 放在正确的路径
- 在程序集加载时正确引用

### Q4: 脚本系统支持多线程吗？

**A**: 不建议。Mono 的多线程支持有限，且引擎的大部分 API 不是线程安全的。建议所有游戏逻辑都在主线程上执行。

---

## 扩展阅读

- [Mono 嵌入式 API 文档](https://www.mono-project.com/docs/advanced/embedding/)
- [.NET 互操作性指南](https://docs.microsoft.com/en-us/dotnet/standard/native-interop/)
- Hazel2D 源码（本脚本系统的灵感来源）

---

## 总结

XingXing 的脚本系统通过 Mono 将 C# 脚本能力集成到 C++ 引擎中，提供了灵活、强大且易于使用的游戏逻辑编程方式。理解其架构和工作原理，将帮助你更好地开发游戏和创建模组。

如果你有任何问题或建议，欢迎在 GitHub 上提出 Issue！

---

**作者注**：本文档旨在为模组开发者提供详尽的技术参考。如果你发现任何错误或不清楚的地方，请随时反馈。
