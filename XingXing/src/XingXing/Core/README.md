# XingXing 核心功能模块 (Core Module)

## 📚 模块概述

Core 模块是 XingXing 引擎的基础核心，提供了引擎运行所需的最基本功能和抽象层。本模块采用现代 C++ 设计模式，为整个引擎提供了稳固的架构基础。

> **💡 命名空间说明**: XingXing 引擎源自 [Hazel2D](https://github.com/TheCherno/Hazel)，为了保持代码的可追溯性和致敬原项目，引擎核心代码仍使用 `Hazel` 命名空间。因此，本文档中的代码示例将使用 `Hazel::` 命名空间前缀。这并不影响引擎的实际功能，您可以将其理解为 XingXing 引擎的内部命名约定。

---

## 🏗️ 核心组件详解

### 1. Application（应用程序管理）

**文件**: `Application.h`, `Application.cpp`

Application 类是整个引擎应用程序的核心管理器，采用单例模式设计。

#### 主要职责：
- **应用程序生命周期管理**：控制应用程序的启动、运行和关闭
- **事件分发**：接收并分发系统事件到各个层
- **层栈管理**：管理和维护渲染层和覆盖层
- **主线程任务队列**：处理需要在主线程执行的任务

#### 关键特性：
```cpp
// 应用程序规范配置
struct ApplicationSpecification {
    std::string Name;              // 应用程序名称
    std::string WorkingDirectory;  // 工作目录
    ApplicationCommandLineArgs CommandLineArgs;  // 命令行参数
};

// 核心方法
void PushLayer(Layer* layer);      // 添加渲染层
void PushOverlay(Layer* layer);    // 添加覆盖层
void OnEvent(Event& e);            // 事件处理
void Close();                      // 关闭应用程序
```

#### 使用示例：
```cpp
// 在客户端代码中创建应用程序实例
Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name = "我的游戏";
    spec.WorkingDirectory = "../assets";
    spec.CommandLineArgs = args;
    
    return new MyGameApp(spec);
}
```

---

### 2. Layer & LayerStack（层系统）

**文件**: `Layer.h`, `Layer.cpp`, `LayerStack.h`, `LayerStack.cpp`

层系统提供了一种模块化的方式来组织应用程序逻辑和渲染。

#### Layer（层）
层是引擎中的一个抽象概念，代表应用程序的一个逻辑单元或渲染阶段。

**核心方法**：
- `OnAttach()`: 层被添加到栈时调用
- `OnDetach()`: 层从栈中移除时调用
- `OnUpdate(Timestep ts)`: 每帧更新时调用
- `OnImGuiRender()`: ImGui 渲染时调用
- `OnEvent(Event& event)`: 事件处理

#### LayerStack（层栈）
层栈管理所有的层，分为两部分：
- **普通层**：从底部开始堆叠
- **覆盖层（Overlay）**：总是渲染在最上层

**特点**：
- 事件从上到下传播（覆盖层 → 普通层）
- 更新和渲染从下到上执行

#### 使用模式：
```cpp
class GameLayer : public Hazel::Layer {
public:
    GameLayer() : Layer("游戏层") {}
    
    virtual void OnAttach() override {
        // 初始化游戏资源
    }
    
    virtual void OnUpdate(Hazel::Timestep ts) override {
        // 更新游戏逻辑
        m_Player.Update(ts);
    }
    
    virtual void OnEvent(Hazel::Event& event) override {
        // 处理输入事件
        if (event.GetEventType() == Hazel::EventType::KeyPressed) {
            // 处理按键
        }
    }
};
```

---

### 3. Window（窗口抽象）

**文件**: `Window.h`, `Window.cpp`

Window 类提供了平台无关的窗口抽象接口。

#### 主要功能：
- **窗口创建和管理**：创建、更新、销毁窗口
- **事件回调**：设置事件回调函数
- **窗口属性**：获取和设置窗口尺寸、标题等
- **垂直同步（VSync）控制**

#### 窗口属性配置：
```cpp
struct WindowProps {
    std::string Title;  // 窗口标题
    uint32_t Width;     // 窗口宽度
    uint32_t Height;    // 窗口高度
    
    WindowProps(const std::string& title = "Hazel Engine",
                uint32_t width = 1600,
                uint32_t height = 900);
};
```

#### 接口方法：
```cpp
virtual void OnUpdate() = 0;                    // 更新窗口
virtual uint32_t GetWidth() const = 0;          // 获取宽度
virtual uint32_t GetHeight() const = 0;         // 获取高度
virtual void SetVSync(bool enabled) = 0;        // 设置垂直同步
virtual void* GetNativeWindow() const = 0;      // 获取原生窗口句柄
```

---

### 4. Input（输入系统）

**文件**: `Input.h`, `KeyCodes.h`, `MouseCodes.h`

输入系统提供了平台无关的输入查询接口。

#### 功能特性：
- **键盘输入检测**：检查特定按键是否被按下
- **鼠标输入检测**：检查鼠标按钮状态
- **鼠标位置获取**：获取鼠标在窗口中的位置

#### API 接口：
```cpp
// 键盘输入
bool IsKeyPressed(KeyCode key);

// 鼠标输入
bool IsMouseButtonPressed(MouseCode button);
glm::vec2 GetMousePosition();
float GetMouseX();
float GetMouseY();
```

#### 使用示例：
```cpp
void OnUpdate(Timestep ts) {
    // 检查键盘输入
    if (Input::IsKeyPressed(Key::W)) {
        m_CameraPosition.y += m_CameraSpeed * ts;
    }
    
    // 检查鼠标输入
    if (Input::IsMouseButtonPressed(Mouse::ButtonLeft)) {
        auto [x, y] = Input::GetMousePosition();
        HZ_INFO("鼠标点击位置: ({0}, {1})", x, y);
    }
}
```

#### 键码定义：
- **KeyCodes.h**: 定义了所有键盘按键的枚举值（如 Key::A, Key::Space 等）
- **MouseCodes.h**: 定义了鼠标按钮的枚举值（如 Mouse::ButtonLeft 等）

---

### 5. Log（日志系统）

**文件**: `Log.h`, `Log.cpp`

基于 spdlog 库的高性能日志系统，支持多种日志级别和格式化输出。

#### 日志分类：
- **核心日志（Core Logger）**：引擎内部使用
- **客户端日志（Client Logger）**：游戏应用层使用

#### 日志级别：
- `TRACE`: 追踪级别，最详细的调试信息
- `INFO`: 信息级别，一般的运行信息
- `WARN`: 警告级别，可能的问题提示
- `ERROR`: 错误级别，错误信息
- `CRITICAL`: 严重错误级别，致命错误

#### 使用宏：
```cpp
// 引擎核心日志
HZ_CORE_TRACE("追踪信息");
HZ_CORE_INFO("启动引擎，版本 {0}", version);
HZ_CORE_WARN("性能警告: FPS 低于 30");
HZ_CORE_ERROR("资源加载失败: {0}", filename);
HZ_CORE_CRITICAL("致命错误: 内存不足");

// 客户端应用日志
HZ_TRACE("玩家位置: ({0}, {1})", x, y);
HZ_INFO("关卡加载完成");
HZ_WARN("敌人数量过多");
HZ_ERROR("保存文件失败");
HZ_CRITICAL("游戏崩溃");
```

#### 特殊功能：
- 支持 GLM 数学库类型的直接输出（vec2, vec3, mat4 等）
- 自动格式化和时间戳
- 可配置的输出目标（控制台、文件等）

---

### 6. Base（基础定义）

**文件**: `Base.h`

Base.h 包含了整个引擎使用的基础宏定义和类型别名。

#### 核心宏定义：
```cpp
// 调试断点
#define HZ_DEBUGBREAK()  // Windows 下使用 __debugbreak()

// 位操作
#define BIT(x) (1 << x)  // 将整数转换为位标志

// 事件绑定辅助宏
#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { \
    return this->fn(std::forward<decltype(args)>(args)...); \
}
```

#### 智能指针别名：
```cpp
// Scope: 独占所有权的智能指针（unique_ptr 的别名）
template<typename T>
using Scope = std::unique_ptr<T>;

// 创建 Scope 指针的辅助函数
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Ref: 共享所有权的智能指针（shared_ptr 的别名）
template<typename T>
using Ref = std::shared_ptr<T>;

// 创建 Ref 指针的辅助函数
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
```

#### 使用场景：
```cpp
// 使用 Scope 管理独占资源
Scope<Window> m_Window = CreateScope<Window>(props);

// 使用 Ref 管理共享资源
Ref<Texture2D> texture = CreateRef<Texture2D>(path);
```

---

### 7. Assert（断言系统）

**文件**: `Assert.h`

提供了增强的断言功能，用于在开发阶段捕获逻辑错误。

#### 断言宏：
```cpp
// 核心断言（引擎内部使用）
HZ_CORE_ASSERT(condition, ...);
HZ_CORE_ASSERT(pointer != nullptr, "指针不能为空！");

// 客户端断言（应用层使用）
HZ_ASSERT(condition, ...);
HZ_ASSERT(index < size, "索引 {0} 超出范围 {1}", index, size);
```

#### 特点：
- 支持格式化错误消息
- Debug 模式下触发断点
- Release 模式下自动禁用

---

### 8. Timestep（时间步长）

**文件**: `Timestep.h`

Timestep 类封装了帧间时间差，用于实现帧率无关的游戏逻辑。

#### 核心概念：
```cpp
class Timestep {
public:
    Timestep(float time = 0.0f) : m_Time(time) {}
    
    float GetSeconds() const;       // 获取秒数
    float GetMilliseconds() const;  // 获取毫秒数
    
    operator float() const;  // 隐式转换为浮点数
};
```

#### 使用示例：
```cpp
void OnUpdate(Timestep ts) {
    // 使用时间步长实现平滑移动
    float distance = m_Speed * ts;  // ts 自动转换为秒数
    m_Position.x += distance;
    
    // 或明确使用秒数
    float timeInSeconds = ts.GetSeconds();
    m_Rotation += m_RotationSpeed * timeInSeconds;
}
```

---

### 9. UUID（通用唯一标识符）

**文件**: `UUID.h`, `UUID.cpp`

UUID 类提供了生成和管理唯一标识符的功能，用于场景对象、资源等的标识。

#### 功能：
- **自动生成唯一 ID**：构造时自动生成随机 UUID
- **显式构造**：可以从 uint64_t 值创建 UUID
- **可哈希**：可用作 unordered_map 的键

#### 使用示例：
```cpp
// 自动生成唯一 ID
UUID entityID;

// 从现有值创建
UUID loadedID(savedValue);

// 用作映射的键
std::unordered_map<UUID, Entity> entities;
entities[entityID] = myEntity;
```

---

### 10. Timer（计时器）

**文件**: `Timer.h`

Timer 类提供了高精度计时功能，常用于性能分析和帧率计算。

#### 典型用途：
- 测量代码段执行时间
- 计算帧时间
- 性能分析和优化

---

### 11. Buffer（缓冲区管理）

**文件**: `Buffer.h`

Buffer 类提供了内存缓冲区的管理功能，用于处理二进制数据。

#### 功能：
- **内存管理**：自动分配和释放内存
- **数据访问**：提供安全的数据读写接口
- **资源加载**：用于从文件加载二进制数据

---

### 12. FileSystem（文件系统）

**文件**: `FileSystem.h`, `FileSystem.cpp`

FileSystem 类提供了跨平台的文件系统操作接口。

#### 功能：
- 文件和目录操作
- 路径处理
- 文件读写

---

### 13. EntryPoint（程序入口）

**文件**: `EntryPoint.h`

EntryPoint.h 定义了应用程序的主入口点，隐藏了平台相关的启动细节。

#### 工作原理：
```cpp
// 客户端只需定义 CreateApplication 函数
extern Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args);

// EntryPoint.h 中自动处理 main 函数
int main(int argc, char** argv) {
    Hazel::Log::Init();
    
    auto app = Hazel::CreateApplication({ argc, argv });
    app->Run();
    delete app;
}
```

---

### 14. PlatformDetection（平台检测）

**文件**: `PlatformDetection.h`

提供编译时平台检测宏，用于编写跨平台代码。

#### 平台宏：
- `HZ_PLATFORM_WINDOWS`: Windows 平台
- `HZ_PLATFORM_LINUX`: Linux 平台
- `HZ_PLATFORM_MACOS`: macOS 平台

#### 使用示例：
```cpp
#ifdef HZ_PLATFORM_WINDOWS
    // Windows 特定代码
    #include <windows.h>
#elif defined(HZ_PLATFORM_LINUX)
    // Linux 特定代码
    #include <X11/Xlib.h>
#endif
```

---

## 🔄 模块交互流程

### 应用程序启动流程：

1. **入口点** (`EntryPoint.h`)
   ```
   main() → 初始化日志系统
   ```

2. **创建应用程序实例**
   ```
   CreateApplication() → 创建自定义 Application 子类
   ```

3. **应用程序初始化**
   ```
   Application 构造函数:
   - 创建 Window
   - 初始化 ImGui
   - 设置事件回调
   ```

4. **运行主循环**
   ```
   Application::Run():
   while (m_Running) {
       - 计算 Timestep
       - 更新所有 Layer (OnUpdate)
       - 渲染 ImGui
       - 更新 Window
       - 处理事件队列
   }
   ```

### 事件处理流程：

```
Window 接收系统事件
    ↓
通过回调传递到 Application::OnEvent()
    ↓
事件分发到 LayerStack（从上到下）
    ↓
每个 Layer::OnEvent() 处理事件
    ↓
事件可以被标记为已处理，停止传播
```

---

## 💡 设计模式和最佳实践

### 1. 单例模式
Application 使用单例模式，确保全局只有一个应用程序实例：
```cpp
static Application& Get() { return *s_Instance; }
```

### 2. 智能指针使用
- 使用 `Scope<T>` 管理独占资源
- 使用 `Ref<T>` 管理共享资源
- 避免手动 new/delete

### 3. 虚函数重写
Layer 系统使用虚函数实现多态：
```cpp
class MyLayer : public Layer {
    virtual void OnUpdate(Timestep ts) override { /* ... */ }
};
```

### 4. 事件回调
使用 `HZ_BIND_EVENT_FN` 宏安全地绑定成员函数：
```cpp
m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));
```

---

## 🎯 使用 Core 模块开发游戏的建议

### 1. 创建自定义应用程序类
```cpp
class MyGame : public Hazel::Application {
public:
    MyGame(const ApplicationSpecification& spec)
        : Application(spec) {
        PushLayer(new GameLayer());
        PushOverlay(new UILayer());
    }
};
```

### 2. 使用层组织游戏逻辑
- **游戏逻辑层**：处理游戏核心逻辑
- **渲染层**：处理游戏渲染
- **UI 层**：作为覆盖层处理 UI

### 3. 善用日志系统
```cpp
HZ_INFO("游戏启动");
HZ_WARN("帧率下降: {0} FPS", fps);
HZ_ERROR("资源加载失败: {0}", path);
```

### 4. 帧率无关的游戏逻辑
```cpp
void OnUpdate(Timestep ts) {
    // 使用 ts 确保在不同帧率下行为一致
    m_Player.Move(direction * speed * ts);
}
```

---

## 📚 相关模块

Core 模块与以下模块紧密协作：
- **Events**: 事件系统，定义和传递各种事件
- **Renderer**: 渲染系统，处理图形渲染
- **Scene**: 场景管理，组织游戏对象
- **ImGui**: 即时模式 GUI，用于调试和编辑器

---

## 🔧 平台相关实现

Core 模块定义接口，具体实现在 Platform 目录：
- `Platform/Windows/`: Windows 平台实现
- `Platform/OpenGL/`: OpenGL 渲染后端

---

## 📖 总结

Core 模块是 XingXing 引擎的基石，提供了：
- ✅ **统一的应用程序框架**
- ✅ **灵活的层系统**
- ✅ **跨平台的抽象接口**
- ✅ **完善的工具类和辅助功能**
- ✅ **清晰的代码组织结构**

理解 Core 模块是深入学习 XingXing 引擎的第一步，也是开发游戏和模组的基础。

---

**注**: 本文档基于 XingXing 引擎源代码分析编写，旨在为模组开发者和学习者提供清晰的技术参考。
