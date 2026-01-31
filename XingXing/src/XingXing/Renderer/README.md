# 渲染系统 (Renderer System)

XingXing 引擎的渲染系统提供了一个高性能、模块化的 2D 渲染解决方案。本系统基于现代图形 API 设计，支持批处理渲染、多种图形原语以及高级文本渲染功能。

## 📁 目录结构

### 核心渲染器
- **Renderer.h/cpp** - 渲染器核心接口，负责场景管理和渲染调度
- **Renderer2D.h/cpp** - 高性能 2D 批处理渲染器，支持精灵、图形原语和文本渲染
- **RendererAPI.h/cpp** - 渲染 API 抽象层，支持多种图形后端
- **RenderCommand.h/cpp** - 渲染命令接口，封装底层渲染调用

### 图形资源管理
- **Buffer.h/cpp** - 顶点缓冲区和索引缓冲区管理
- **VertexArray.h/cpp** - 顶点数组对象 (VAO) 封装
- **Shader.h/cpp** - 着色器程序管理和编译
- **Texture.h/cpp** - 纹理资源管理，支持 2D 纹理
- **Framebuffer.h/cpp** - 帧缓冲对象管理，用于离屏渲染
- **UniformBuffer.h/cpp** - Uniform 缓冲区对象 (UBO) 管理

### 摄像机系统
- **Camera.h** - 摄像机基类定义
- **OrthographicCamera.h/cpp** - 正交摄像机实现
- **OrthographicCameraController.h/cpp** - 正交摄像机控制器
- **EditorCamera.h/cpp** - 编辑器专用摄像机

### 文本渲染
- **Font.h/cpp** - 字体资源管理
- **MSDFData.h** - MSDF (Multi-channel Signed Distance Field) 字体数据结构

### 图形上下文
- **GraphicsContext.h/cpp** - 图形上下文抽象，管理渲染窗口和 OpenGL 上下文

## 🎨 主要功能

### 1. 2D 批处理渲染
Renderer2D 提供高效的批处理渲染功能，减少绘制调用次数：
- 四边形渲染（纯色和纹理）
- 旋转四边形支持
- 圆形渲染
- 线条和矩形渲染
- 精灵渲染

### 2. 文本渲染
基于 MSDF 技术的高质量文本渲染：
- 支持任意缩放而不失真
- 字距调整 (Kerning)
- 行间距控制
- 多种字体支持

### 3. 摄像机系统
灵活的摄像机系统：
- 正交投影摄像机
- 摄像机控制器（支持平移、缩放）
- 编辑器摄像机（支持 3D 导航）

### 4. 资源管理
完整的图形资源管理：
- 顶点缓冲区布局系统
- 着色器热重载支持
- 纹理资源管理
- 帧缓冲管理

### 5. 渲染统计
实时渲染性能统计：
- 绘制调用计数
- 四边形数量
- 顶点和索引数量

## 🔧 架构设计

### 渲染流程
```
1. Renderer::Init() - 初始化渲染器
2. Renderer2D::BeginScene() - 开始场景渲染
3. Renderer2D::DrawXXX() - 提交绘制命令
4. Renderer2D::EndScene() - 结束场景并提交批次
5. Renderer2D::Flush() - 刷新渲染命令
```

### API 抽象层
渲染系统通过 RendererAPI 抽象不同的图形后端：
- 当前支持：OpenGL
- 设计支持：DirectX、Vulkan（未来扩展）

### 批处理系统
Renderer2D 使用智能批处理技术：
- 自动合批相同材质的对象
- 纹理图集支持
- 动态顶点缓冲管理
- 超出批次限制时自动分批

## 📊 性能特性

- **批处理渲染**：最小化绘制调用
- **实例化渲染**：高效渲染大量相同对象
- **纹理图集**：减少纹理切换开销
- **统一缓冲对象**：高效的着色器数据传输
- **离屏渲染**：支持后处理效果

## 🔌 使用示例

### 基础渲染
```cpp
// 初始化
Renderer2D::Init();

// 开始场景
Renderer2D::BeginScene(camera);

// 绘制四边形
Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f});

// 绘制纹理
Renderer2D::DrawQuad({2.0f, 0.0f}, {1.0f, 1.0f}, texture);

// 结束场景
Renderer2D::EndScene();
```

### 文本渲染
```cpp
Renderer2D::TextParams textParams;
textParams.Color = {1.0f, 1.0f, 1.0f, 1.0f};
textParams.Kerning = 0.0f;

Renderer2D::DrawString("Hello, XingXing!", font, transform, textParams);
```

## 📝 注意事项

1. **命名空间**：渲染系统代码位于 `Hazel` 命名空间（源自 Hazel2D）
2. **线程安全**：渲染命令应在主线程调用
3. **资源生命周期**：使用智能指针 (Ref/Scope) 管理资源
4. **批次限制**：单个批次有顶点和纹理槽位限制

## 🔗 相关模块

- **Platform/OpenGL/** - OpenGL 具体实现
- **Scene/** - 场景管理系统
- **Core/** - 核心系统和智能指针

## 📚 扩展阅读

本渲染系统基于 [Hazel2D](https://github.com/TheCherno/Hazel) 引擎架构开发，针对 XingXing 引擎的特定需求进行了优化和扩展。
