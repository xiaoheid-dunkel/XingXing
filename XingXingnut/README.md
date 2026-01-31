# 📝 XingXingnut - 可视化编辑器教程

## 目录
- [什么是 XingXingnut](#什么是-xingxingnut)
- [目录结构详解](#目录结构详解)
- [核心组件说明](#核心组件说明)
- [SandboxProject 示例项目](#sandboxproject-示例项目)
- [如何使用编辑器](#如何使用编辑器)
- [面板系统详解](#面板系统详解)

---

## 什么是 XingXingnut

**XingXingnut** 是 XingXing 引擎的**可视化场景编辑器**。它提供了一个类似于 Unity、Unreal 等主流引擎的图形化编辑界面，让开发者可以通过直观的方式创建和编辑游戏场景，而不需要完全通过代码来构建游戏世界。

### 主要功能
- 🎨 **场景编辑**：可视化创建和编辑游戏场景
- 📊 **实体层级管理**：通过层级面板管理场景中的所有游戏对象
- 📁 **资源浏览器**：方便地浏览和管理项目资源（纹理、脚本、场景等）
- 🎮 **实时预览**：在编辑器中直接运行和测试游戏场景
- 🔧 **组件编辑**：为实体添加和配置各种组件（Transform、Sprite、物理组件等）

---

## 目录结构详解

```
XingXingnut/               # 可视化编辑器根目录
│
├── src/                   # 源代码目录
│   ├── EditorLayer.cpp    # 编辑器主逻辑实现
│   ├── EditorLayer.h      # 编辑器主逻辑头文件
│   ├── HazelnutApp.cpp    # 编辑器应用程序入口
│   └── Panels/            # UI 面板目录
│       ├── SceneHierarchyPanel.h      # 场景层级面板（头文件）
│       ├── SceneHierarchyPanel.cpp    # 场景层级面板（实现）
│       ├── ContentBrowserPanel.h      # 内容浏览器面板（头文件）
│       └── ContentBrowserPanel.cpp    # 内容浏览器面板（实现）
│
├── SandboxProject/        # 示例项目
│   ├── Assets/            # 项目资源文件夹
│   │   ├── Scenes/        # 场景文件（.hazel）
│   │   ├── Scripts/       # C# 脚本源码和编译后的 DLL
│   │   └── Textures/      # 纹理图片资源
│   └── Sandbox.hproj      # 项目配置文件
│
├── Resources/             # 编辑器资源
│   ├── Icons/             # 编辑器 UI 图标（播放、暂停、停止等）
│   └── Scripts/           # 编辑器脚本核心库
│
├── assets/                # 编辑器内部资源
│   ├── fonts/             # 字体文件（OpenSans 等）
│   ├── shaders/           # 着色器程序
│   └── cache/             # 着色器编译缓存
│
├── mono/                  # Mono C# 运行时库
│
├── imgui.ini              # ImGui 界面配置文件（窗口布局等）
│
└── premake5.lua           # 项目构建配置
```

---

## 核心组件说明

### 1. EditorLayer（编辑器主逻辑层）

**文件位置**：`src/EditorLayer.cpp` 和 `src/EditorLayer.h`

**作用**：EditorLayer 是整个编辑器的核心控制类，负责：

#### 主要职责
- **场景管理**
  - 创建新场景（NewScene）
  - 打开现有场景（OpenScene）
  - 保存场景（SaveScene、SaveSceneAs）
  - 场景序列化和反序列化

- **项目管理**
  - 打开项目（OpenProject）
  - 保存项目配置（SaveProject）
  - 管理项目资源路径

- **编辑器状态控制**
  - 编辑模式（Edit）：在此模式下编辑场景
  - 播放模式（Play）：运行游戏逻辑
  - 模拟模式（Simulate）：仅模拟物理，不运行脚本
  - 暂停/步进：调试功能

- **视口渲染**
  - 使用 Framebuffer 渲染场景
  - 编辑器相机控制（EditorCamera）
  - 场景视口大小调整

- **实体操作**
  - 实体选择和高亮
  - 实体复制（OnDuplicateEntity）
  - Gizmo 操作（平移、旋转、缩放工具）

#### 代码示例片段
```cpp
// 场景状态枚举
enum class SceneState
{
    Edit = 0,      // 编辑模式
    Play = 1,      // 播放模式  
    Simulate = 2   // 模拟模式
};
```

### 2. HazelnutApp（编辑器应用入口）

**文件位置**：`src/HazelnutApp.cpp`

**作用**：这是编辑器应用程序的入口点，负责：
- 创建应用程序实例
- 初始化 EditorLayer
- 设置应用程序规格（名称、命令行参数等）

这是一个非常简洁的文件，主要工作是将 EditorLayer 推入应用程序的层栈中。

#### 代码结构
```cpp
class Hazelnut : public Application
{
    // 构造函数中推入 EditorLayer
    PushLayer(new EditorLayer());
};
```

---

## 面板系统详解

XingXingnut 使用 **ImGui** 库来构建用户界面。所有的 UI 面板都放在 `src/Panels/` 目录下。

### 1. SceneHierarchyPanel（场景层级面板）

**文件位置**：`src/Panels/SceneHierarchyPanel.h` 和 `.cpp`

**功能描述**：
- 显示当前场景中的所有实体（Entity）的树形结构
- 允许选择、重命名、删除实体
- 显示选中实体的所有组件（Component）
- 为实体添加新组件（Transform、Sprite、Camera、Rigidbody 等）
- 编辑组件的属性（位置、旋转、缩放、颜色等）

**使用场景**：
当你想要查看场景中有哪些游戏对象，或者需要修改某个对象的属性时，就使用这个面板。

**主要方法**：
- `SetContext()`：设置当前编辑的场景
- `OnImGuiRender()`：渲染面板 UI
- `DrawEntityNode()`：绘制单个实体节点
- `DrawComponents()`：绘制实体的所有组件
- `GetSelectedEntity()`：获取当前选中的实体

### 2. ContentBrowserPanel（内容浏览器面板）

**文件位置**：`src/Panels/ContentBrowserPanel.h` 和 `.cpp`

**功能描述**：
- 浏览项目的 Assets 文件夹
- 以图标形式显示文件和文件夹
- 支持导航到子文件夹
- 显示纹理、场景文件、脚本等资源
- 双击文件可以打开或加载资源

**使用场景**：
当你需要从项目文件夹中查找纹理、场景或脚本文件时，使用这个面板。

**关键成员**：
- `m_BaseDirectory`：项目的基础目录（通常是 Assets 文件夹）
- `m_CurrentDirectory`：当前浏览的目录
- `m_DirectoryIcon`：文件夹图标纹理
- `m_FileIcon`：文件图标纹理

---

## SandboxProject 示例项目

**目录位置**：`XingXingnut/SandboxProject/`

### 什么是 SandboxProject？

SandboxProject 是一个**完整的示例项目**，用于：
1. **演示如何组织项目结构**
2. **提供可直接运行的场景示例**
3. **作为学习和实验的沙盒环境**

### 项目结构

```
SandboxProject/
│
├── Sandbox.hproj          # 项目配置文件
│   包含项目名称、启动场景、资源目录、脚本模块路径等信息
│
└── Assets/                # 资源文件夹（所有游戏资源都放在这里）
    │
    ├── Scenes/            # 场景文件
    │   ├── Example.hazel       # 基础示例场景
    │   ├── Physics2D.hazel     # 2D 物理演示场景
    │   ├── PinkCube.hazel      # 简单立方体场景
    │   └── 3DExample.hazel     # 3D 场景示例
    │
    ├── Scripts/           # C# 脚本
    │   ├── Source/        # 脚本源代码
    │   │   ├── Player.cs       # 玩家控制脚本示例
    │   │   └── Camera.cs       # 相机控制脚本示例
    │   └── Binaries/      # 编译后的 DLL 文件
    │       ├── Sandbox.dll     # 项目脚本编译后的动态库
    │       └── Hazel-ScriptCore.dll  # 脚本核心库
    │
    └── Textures/          # 纹理图片
        ├── Checkerboard.png    # 棋盘格纹理
        └── ChernoLogo.png      # Logo 图片
```

### 项目配置文件（.hproj）

`Sandbox.hproj` 是一个 **YAML 格式**的项目配置文件，定义了：

```yaml
Project:
  Name: Sandbox                              # 项目名称
  StartScene: "Scenes/Physics2D.hazel"       # 启动场景路径
  AssetDirectory: "Assets"                   # 资源目录
  ScriptModulePath: "Scripts/Binaries/Sandbox.dll"  # 脚本模块路径
```

当你在编辑器中打开这个项目时，编辑器会：
1. 读取 `Sandbox.hproj` 文件
2. 设置资源根目录为 `Assets/`
3. 加载 `Scenes/Physics2D.hazel` 作为默认场景
4. 加载脚本 DLL 以支持 C# 脚本功能

---

## 如何使用编辑器

### 1. 启动编辑器

编译并运行 XingXingnut 项目后，编辑器会：
1. 提示选择或打开一个项目
2. 如果通过命令行传入了项目路径，则直接打开该项目
3. 默认情况下可以打开 `SandboxProject` 来体验

### 2. 编辑器界面布局

启动后你会看到以下主要区域：

```
┌─────────────────────────────────────────────────────────┐
│  [文件] [编辑] [视图]          [▶ 播放] [⏸ 暂停] [⏹ 停止] │  工具栏
├──────────────┬──────────────────────────┬───────────────┤
│              │                          │               │
│  场景层级    │      场景视口            │  属性面板     │
│  面板        │   （3D/2D 场景渲染）     │  （组件属性） │
│              │                          │               │
│  - Entity1   │      [游戏画面]          │  Transform    │
│  - Entity2   │                          │  Sprite       │
│  - Camera    │                          │  ...          │
│              │                          │               │
├──────────────┴──────────────────────────┴───────────────┤
│  内容浏览器面板                                          │
│  [📁 Scenes] [📁 Scripts] [📁 Textures]                 │
└─────────────────────────────────────────────────────────┘
```

### 3. 基本操作流程

#### 创建新场景
1. 点击 `文件 -> 新建场景` 或按 `Ctrl+N`
2. 在场景层级面板中右键创建实体
3. 为实体添加组件（Transform、Sprite 等）

#### 保存场景
1. 点击 `文件 -> 保存场景` 或按 `Ctrl+S`
2. 选择保存位置（通常在 `Assets/Scenes/` 目录下）

#### 运行场景
1. 点击工具栏的 **播放按钮 ▶**
2. 场景将进入运行模式，所有脚本和物理都会激活
3. 点击 **停止按钮 ⏹** 返回编辑模式

#### 添加实体和组件
1. 在场景层级面板空白处右键，选择 `创建空实体`
2. 选中该实体
3. 在属性面板点击 `添加组件` 按钮
4. 选择需要的组件类型（如 Sprite Renderer、Rigidbody2D 等）

---

## 面向模组开发者的提示

### 理解项目结构的重要性

如果你想为基于 XingXing 引擎的游戏开发模组（Mod），理解 XingXingnut 的结构将帮助你：

1. **读懂场景文件**：`.hazel` 场景文件是 YAML 格式，可以手动编辑
2. **理解实体-组件系统**：游戏对象都是由实体（Entity）和组件（Component）组成
3. **脚本交互**：通过 C# 脚本可以访问和修改场景中的实体
4. **资源组织**：了解如何正确组织纹理、音频、场景等资源

### 推荐学习路径

1. **先玩 SandboxProject**：打开示例项目，运行各个场景，观察效果
2. **阅读示例脚本**：查看 `Player.cs` 和 `Camera.cs`，理解脚本如何工作
3. **尝试修改场景**：在编辑器中修改实体的属性，观察变化
4. **创建简单场景**：从零开始创建一个简单的场景
5. **学习组件系统**：了解各种组件的作用（Transform、Sprite、Rigidbody 等）

---

## 技术栈

- **C++17**：编辑器核心逻辑
- **ImGui**：用户界面库
- **ImGuizmo**：3D 变换 Gizmo（平移、旋转、缩放工具）
- **YAML**：场景和项目配置文件格式
- **Mono**：C# 脚本运行时
- **OpenGL**：图形渲染 API

---

## 常见问题

### Q: 为什么叫 "Hazelnut" 而不是 "XingXingnut"？
**A**: 由于历史原因，代码中的命名空间和部分类名仍然使用 "Hazel" 或 "Hazelnut"（源自原始的 Hazel2D 引擎）。在实际使用中，XingXingnut 就是这个编辑器的正式名称。

### Q: 我可以修改编辑器源码吗？
**A**: 可以！开源的目的就是让开发者和模组作者能够深入理解引擎。但请遵守许可协议，不要用于商业用途。

### Q: 如何调试场景脚本？
**A**: 可以通过 Visual Studio 附加到 XingXingnut 进程来调试 C# 脚本。确保编译时使用了 Debug 配置。

### Q: 支持哪些组件类型？
**A**: 目前支持：
- Transform（位置、旋转、缩放）
- Sprite Renderer（2D 精灵渲染）
- Camera（相机）
- Rigidbody2D（2D 物理刚体）
- BoxCollider2D / CircleCollider2D（2D 碰撞器）
- Script Component（C# 脚本组件）
- Text Renderer（文本渲染，基于 MSDF）

---

## 参考资源

- [XingXing 主仓库](https://github.com/xiaoheid-dunkel/XingXing)
- [Hazel2D 原始引擎](https://github.com/TheCherno/Hazel)
- [ImGui 文档](https://github.com/ocornut/imgui)

---

**祝你在 XingXing 引擎上创作愉快！ 🎮✨**
