# 快速开始：创建你的第一个 3D 方块场景

本指南将引导你在 5 分钟内创建一个简单的 3D 方块场景。

## 前置要求

- Visual Studio 2022 或更高版本
- XingXing 引擎已编译
- 基本的 C++ 知识

## 步骤 1：创建 Layer 类

创建新文件 `MyVoxelLayer.h`：

```cpp
#pragma once
#include <XingXing.h>

class MyVoxelLayer : public Hazel::Layer
{
public:
    MyVoxelLayer() : Layer("MyVoxel") {}
    
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Hazel::Timestep ts) override;

private:
    Hazel::PerspectiveCamera m_Camera;
    Hazel::Ref<Hazel::Texture2D> m_Texture;
};
```

## 步骤 2：实现 Layer

创建 `MyVoxelLayer.cpp`：

```cpp
#include "MyVoxelLayer.h"

void MyVoxelLayer::OnAttach()
{
    // 初始化 3D 渲染器
    Hazel::Renderer3D::Init();
    
    // 创建相机 (FOV 45度，16:9 宽高比，近平面 0.1，远平面 100)
    m_Camera = Hazel::PerspectiveCamera(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    m_Camera.SetPosition(glm::vec3(0.0f, 2.0f, 5.0f));
    
    // 创建简单的绿色纹理
    m_Texture = Hazel::Texture2D::Create(1, 1);
    uint32_t greenColor = 0xFF00FF00; // ABGR format
    m_Texture->SetData(&greenColor, sizeof(uint32_t));
}

void MyVoxelLayer::OnDetach()
{
    Hazel::Renderer3D::Shutdown();
}

void MyVoxelLayer::OnUpdate(Hazel::Timestep ts)
{
    // 清屏（天空蓝色）
    Hazel::RenderCommand::SetClearColor({ 0.5f, 0.7f, 1.0f, 1.0f });
    Hazel::RenderCommand::Clear();
    
    // 开始渲染 3D 场景
    Hazel::Renderer3D::BeginScene(m_Camera);
    
    // 绘制一个 5x5 的地面
    for (int x = -2; x <= 2; x++)
    {
        for (int z = -2; z <= 2; z++)
        {
            Hazel::Renderer3D::DrawCube(
                glm::vec3(x, 0, z),           // 位置
                glm::vec3(1.0f),              // 大小 (1x1x1)
                m_Texture                     // 纹理
            );
        }
    }
    
    // 在中心添加一个红色方块
    Hazel::Renderer3D::DrawCube(
        glm::vec3(0, 1, 0),
        glm::vec3(1.0f),
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)  // 红色
    );
    
    // 结束场景
    Hazel::Renderer3D::EndScene();
}
```

## 步骤 3：添加到应用

在你的 `Application` 类中添加 Layer：

```cpp
#include "MyVoxelLayer.h"

class MyApp : public Hazel::Application
{
public:
    MyApp(const Hazel::ApplicationSpecification& spec)
        : Hazel::Application(spec)
    {
        PushLayer(new MyVoxelLayer());
    }
};
```

## 步骤 4：运行！

编译并运行项目，你应该看到：
- 一个 5x5 的绿色地面
- 中间有一个红色方块
- 天蓝色的背景

## 下一步

### 添加相机控制

在 `OnUpdate` 中添加键盘控制：

```cpp
void MyVoxelLayer::OnUpdate(Hazel::Timestep ts)
{
    // 获取当前位置
    glm::vec3 pos = m_Camera.GetPosition();
    float speed = 5.0f * ts;
    
    // WASD 移动
    if (Hazel::Input::IsKeyPressed(Hazel::Key::W)) pos.z -= speed;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::S)) pos.z += speed;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::A)) pos.x -= speed;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::D)) pos.x += speed;
    
    // QE 上下移动
    if (Hazel::Input::IsKeyPressed(Hazel::Key::Q)) pos.y -= speed;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::E)) pos.y += speed;
    
    m_Camera.SetPosition(pos);
    
    // ... 其余渲染代码
}
```

### 添加旋转动画

让中心的红色方块旋转：

```cpp
// 在类中添加成员变量
float m_Rotation = 0.0f;

// 在 OnUpdate 中
m_Rotation += ts;  // 每秒旋转 1 弧度

glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1, 0));
transform = glm::rotate(transform, m_Rotation, glm::vec3(0, 1, 0));  // 绕 Y 轴旋转

Hazel::Renderer3D::DrawCube(
    transform,
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
);
```

### 使用真实纹理

```cpp
// 加载图片纹理
m_Texture = Hazel::Texture2D::Create("assets/textures/grass.png");
```

## 完整示例

查看 `Sandbox/src/VoxelWorldLayer.cpp` 获取包含以下特性的完整示例：
- 相机控制（移动 + 旋转）
- 多种方块类型
- ImGui 调试面板
- 性能统计

## 常见问题

**Q: 方块不显示？**
A: 检查相机位置。使用 `m_Camera.SetPosition(glm::vec3(0, 5, 10))` 确保相机不在方块内部。

**Q: 方块是黑色的？**
A: 可能是光照问题。检查着色器是否正确加载：`assets/shaders/Renderer3D_Cube.glsl`

**Q: 如何改变光照方向？**
A: 编辑 `Renderer3D_Cube.glsl` 中的 `lightDir` 向量。

## 更多资源

- [3D API 完整文档](3D_INTEGRATION_GUIDE.md)
- [升级计划](3D_UPGRADE_PLAN.md)
- [实施总结](IMPLEMENTATION_SUMMARY.md)

---

祝你开发愉快！ 🎮
