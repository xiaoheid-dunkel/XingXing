# XingXing 3D 功能集成指南

## 概述
本文档介绍如何在 XingXing 引擎中使用新添加的 3D 渲染功能，特别是用于开发类似 Minecraft 的方块沙盒游戏。

## 快速开始

### 1. 基础设置

在你的 Layer 中初始化 3D 渲染器：

```cpp
#include <XingXing.h>

class MyGameLayer : public Hazel::Layer
{
public:
    void OnAttach() override
    {
        // 初始化 3D 渲染器
        Hazel::Renderer3D::Init();
        
        // 创建透视相机 (FOV, 宽高比, 近平面, 远平面)
        m_Camera = Hazel::PerspectiveCamera(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
        m_Camera.SetPosition(glm::vec3(0.0f, 5.0f, 10.0f));
    }
    
    void OnDetach() override
    {
        // 清理
        Hazel::Renderer3D::Shutdown();
    }

private:
    Hazel::PerspectiveCamera m_Camera;
};
```

### 2. 渲染 3D 方块

```cpp
void OnUpdate(Hazel::Timestep ts)
{
    // 清屏
    Hazel::RenderCommand::SetClearColor({ 0.53f, 0.81f, 0.92f, 1.0f });
    Hazel::RenderCommand::Clear();
    
    // 开始 3D 场景
    Hazel::Renderer3D::BeginScene(m_Camera);
    
    // 绘制单个方块
    Hazel::Renderer3D::DrawCube(
        glm::vec3(0.0f, 0.0f, 0.0f),  // 位置
        glm::vec3(1.0f),                // 大小
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)  // 红色
    );
    
    // 绘制带纹理的方块
    Hazel::Renderer3D::DrawCube(
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(1.0f),
        m_Texture,
        glm::vec4(1.0f)  // 白色 tint
    );
    
    // 结束场景
    Hazel::Renderer3D::EndScene();
}
```

### 3. 相机控制

```cpp
void OnUpdate(Hazel::Timestep ts)
{
    float speed = 5.0f * ts;
    glm::vec3 position = m_Camera.GetPosition();
    
    // 简单的 WASD 移动
    if (Hazel::Input::IsKeyPressed(Hazel::Key::W))
        position.z -= speed;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::S))
        position.z += speed;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::A))
        position.x -= speed;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::D))
        position.x += speed;
    
    m_Camera.SetPosition(position);
}
```

## API 参考

### PerspectiveCamera

透视相机类，用于 3D 场景渲染。

```cpp
// 构造函数
PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip);

// 方法
void SetPosition(const glm::vec3& position);
void SetRotation(const glm::vec3& rotation);  // Pitch, Yaw, Roll (弧度)
const glm::vec3& GetPosition() const;
const glm::mat4& GetViewMatrix() const;
const glm::mat4& GetViewProjectionMatrix() const;
```

### Renderer3D

3D 渲染器类，提供批量渲染功能。

```cpp
// 初始化和清理
static void Init();
static void Shutdown();

// 场景控制
static void BeginScene(const PerspectiveCamera& camera);
static void EndScene();

// 绘制立方体
static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
static void DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture);
static void DrawCube(const glm::mat4& transform, const glm::vec4& color);
static void DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture);

// 统计信息
struct Statistics {
    uint32_t DrawCalls;
    uint32_t CubeCount;
    uint32_t GetTotalVertexCount() const;
    uint32_t GetTotalIndexCount() const;
};
static Statistics GetStats();
static void ResetStats();
```

## 示例项目

查看 `Sandbox/src/VoxelWorldLayer.cpp` 获取完整的可运行示例。

### 运行示例

1. 打开项目：`XingXing.sln`
2. 将 `Sandbox` 设置为启动项目
3. 编译并运行（F5）

### 控制说明

- **WASD** - 水平移动
- **Q/E** - 上下移动
- **方向键** - 旋转相机

## 性能优化建议

### 1. 批量渲染
Renderer3D 自动批量处理多个立方体的渲染，最多支持 1000 个立方体/批次。

### 2. 纹理图集
使用纹理图集减少纹理切换：

```cpp
// 创建包含多个方块纹理的图集
Ref<Texture2D> blockAtlas = Texture2D::Create("assets/textures/blocks.png");

// 使用不同的 UV 坐标访问不同的方块纹理
// (需要扩展 API 支持自定义 UV)
```

### 3. 视锥裁剪
只渲染相机视野内的方块（未来实现）。

### 4. 面剔除
启用面剔除以提高性能：

```cpp
// 在 Renderer3D::Init() 中已默认启用
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
```

## 与 2D 功能共存

3D 功能不会影响现有的 2D 渲染功能。你可以在同一个应用中同时使用 2D 和 3D：

```cpp
void OnUpdate(Hazel::Timestep ts)
{
    // 2D 渲染
    Hazel::Renderer2D::BeginScene(m_Camera2D);
    Hazel::Renderer2D::DrawQuad(...);
    Hazel::Renderer2D::EndScene();
    
    // 3D 渲染
    Hazel::Renderer3D::BeginScene(m_Camera3D);
    Hazel::Renderer3D::DrawCube(...);
    Hazel::Renderer3D::EndScene();
}
```

## 光照系统

当前实现包含简单的方向光照：

```glsl
// 在 Renderer3D_Cube.glsl 中
vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
float diff = max(dot(normal, lightDir), 0.0);
float ambient = 0.3;
float lighting = ambient + (1.0 - ambient) * diff;
```

你可以通过修改着色器来自定义光照效果。

## 下一步开发

参考 `docs/3D_UPGRADE_PLAN.md` 查看完整的升级计划，包括：

1. 分块系统（Chunk System）
2. 方块类型管理
3. 世界生成器
4. 高级光照
5. 阴影系统
6. 物理碰撞

## 故障排除

### 问题：方块不显示
- 检查相机位置是否正确
- 确保调用了 `Renderer3D::Init()`
- 验证着色器文件是否存在：`assets/shaders/Renderer3D_Cube.glsl`

### 问题：性能低下
- 减少渲染的方块数量
- 检查是否有不必要的状态切换
- 考虑实现分块裁剪

### 问题：光照异常
- 检查法线向量是否正确
- 验证着色器编译无误

## 贡献

欢迎提交 Issue 和 Pull Request 来改进 3D 功能！

## 许可证

本项目遵循与 XingXing 引擎相同的许可协议。请查看根目录的 LICENSE 文件。
