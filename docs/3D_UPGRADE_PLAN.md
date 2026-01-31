# XingXing 引擎 2D 到 3D 升级开发计划

## 项目目标
将 XingXing (基于 Hazel2D) 引擎升级为支持类似《Minecraft》风格的 3D 方块沙盒游戏引擎，同时保持与现有 2D 功能的兼容性。

---

## 一、核心技术挑战与解决方案

### 1.1 渲染管线升级
**挑战：**
- 从 2D 正交投影转换到 3D 透视投影
- 需要处理深度缓冲（Z-buffer）
- 光照计算（法线、光源）
- 3D 纹理映射和 UV 坐标

**解决方案：**
- 保留 `Renderer2D`，新增 `Renderer3D` 类
- 实现 `PerspectiveCamera` 类用于透视投影
- 在 `Framebuffer` 中添加深度附件支持
- 创建基础的 Phong 光照着色器

### 1.2 坐标系统变更
**挑战：**
- 2D 使用 XY 平面，3D 需要完整的 XYZ 空间
- 相机控制从 2D 平移/缩放到 3D 旋转/移动

**解决方案：**
- 使用右手坐标系（Y 轴向上）
- 实现第一人称/第三人称相机控制器
- 保持 GLM 数学库的一致性

### 1.3 性能优化
**挑战：**
- 3D 方块世界的顶点数量巨大
- 需要高效的批渲染和剔除

**解决方案：**
- 实现视锥体裁剪（Frustum Culling）
- 面剔除（Face Culling）- 只渲染可见面
- 分块（Chunk）系统 - 将世界分成 16x16x16 的区块
- 实例化渲染（Instanced Rendering）用于相同方块

### 1.4 物理系统
**挑战：**
- Box2D 仅支持 2D 物理

**解决方案：**
- 阶段一：实现简单的 AABB 碰撞检测
- 长期：考虑集成 Bullet Physics 或 PhysX（可选）

---

## 二、分阶段开发任务清单

### 阶段 0：准备工作（1-2 天）
**目标：** 搭建 3D 开发基础设施

**任务清单：**
- [x] ✅ 分析现有代码架构
- [ ] 📝 创建 3D 数学工具类（扩展 Math.h）
- [ ] 📝 准备 3D 着色器模板
- [ ] 📝 配置构建系统

**交付物：**
- `Math3D.h/cpp` - 3D 向量、四元数等工具
- `shaders/Basic3D.glsl` - 基础 3D 着色器
- 更新的 `premake5.lua`

---

### 阶段 1：透视相机系统（2-3 天）
**目标：** 实现 3D 透视相机

**任务清单：**
- [ ] 🎥 创建 `PerspectiveCamera` 类
- [ ] 🎮 实现第一人称相机控制器 `FirstPersonCameraController`
- [ ] 🔧 添加深度缓冲支持到 `Framebuffer`
- [ ] 🧪 测试透视投影矩阵

**交付物：**
- `XingXing/src/XingXing/Renderer/PerspectiveCamera.h/cpp`
- `XingXing/src/XingXing/Renderer/FirstPersonCameraController.h/cpp`
- 更新的 `Framebuffer.h/cpp`

**测试标准：**
- 相机可以通过 WASD 移动
- 鼠标可以控制视角旋转
- 透视投影正确显示深度

---

### 阶段 2：基础 3D 渲染器（3-4 天）
**目标：** 构建 Renderer3D 框架

**任务清单：**
- [ ] 🎨 创建 `Renderer3D` 类（类似 Renderer2D 架构）
- [ ] 📦 实现 `DrawCube()` 方法
- [ ] 💡 添加基础光照系统（方向光）
- [ ] 🎨 创建 3D 着色器（顶点/片段着色器）
- [ ] 🧊 实现基础的立方体网格生成

**交付物：**
- `XingXing/src/XingXing/Renderer/Renderer3D.h/cpp`
- `Sandbox/assets/shaders/Cube.glsl`
- 简单的立方体渲染示例

**测试标准：**
- 可以渲染带纹理的单个立方体
- 光照计算正确（漫反射/环境光）
- 深度测试工作正常

---

### 阶段 3：方块系统与批渲染（4-5 天）
**目标：** 实现高效的方块渲染

**任务清单：**
- [ ] 🧱 设计 `Block` 组件（类型、纹理、属性）
- [ ] 🗺️ 实现 `Chunk` 系统（16x16x16 方块）
- [ ] ⚡ 优化：仅渲染可见面（面剔除）
- [ ] 🎯 实现视锥体裁剪
- [ ] 📦 批量渲染相邻方块

**交付物：**
- `XingXing/src/XingXing/World/Block.h`
- `XingXing/src/XingXing/World/Chunk.h/cpp`
- `XingXing/src/XingXing/World/ChunkMeshBuilder.h/cpp`

**测试标准：**
- 渲染 16x16x16 的区块性能 > 60 FPS
- 内部方块面不渲染（优化生效）
- 支持至少 10 种方块类型

---

### 阶段 4：简单的世界生成（2-3 天）
**目标：** 创建 Minecraft 风格的基础场景

**任务清单：**
- [ ] 🌍 实现简单的地形生成（平地 + 随机方块）
- [ ] 🎨 添加纹理图集（Texture Atlas）
- [ ] 🏗️ 实现方块放置/销毁
- [ ] 👤 添加玩家碰撞检测

**交付物：**
- `Sandbox/src/VoxelWorld.h/cpp`
- `Sandbox/assets/textures/blocks.png`（方块纹理图集）
- 可玩的 Demo 场景

**测试标准：**
- 场景包含至少 20x20x5 的地形
- 玩家可以移动并与方块碰撞
- 可以添加/删除方块

---

### 阶段 5：优化与工具集成（3-4 天）
**目标：** 性能优化与开发者工具

**任务清单：**
- [ ] 📊 添加性能分析工具（帧率、顶点数）
- [ ] 🔍 实现射线检测（方块拾取）
- [ ] 🎨 集成到 XingXingEditor（可选）
- [ ] 📚 编写 API 文档和教程

**交付物：**
- 性能监控面板
- 射线投射工具类
- `docs/3D_API_GUIDE.md`
- 示例项目

**测试标准：**
- 渲染 10+ 区块时帧率 > 60 FPS
- 射线检测准确识别方块
- 文档清晰可用

---

## 三、推荐的技术栈

### 3.1 3D 数学库
**已有：GLM（OpenGL Mathematics）**
- ✅ 已集成在项目中
- ✅ 提供向量、矩阵、四元数运算
- ✅ 与 OpenGL 完美兼容

**建议扩展：**
```cpp
// 添加到 Math.h
namespace Hazel::Math {
    glm::mat4 CreatePerspectiveProjection(float fov, float aspect, float near, float far);
    glm::mat4 CreateLookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
    glm::quat RotationBetweenVectors(const glm::vec3& start, const glm::vec3& dest);
}
```

### 3.2 渲染 API
**当前：OpenGL 3.3+（通过 Glad）**
- ✅ 跨平台支持
- ✅ 成熟稳定
- ⚠️ 建议未来考虑 Vulkan（已有部分依赖）

**3D 渲染特性：**
- 深度测试（glEnable(GL_DEPTH_TEST)）
- 面剔除（glEnable(GL_CULL_FACE)）
- 帧缓冲对象（FBO）- 已支持

### 3.3 性能优化工具
**推荐添加：**
1. **Tracy Profiler** - 性能分析
2. **RenderDoc** - 图形调试
3. **Optick** - 轻量级性能监控

**内置工具：**
- `Renderer::Statistics` - 扩展支持 3D 统计
- ImGui 性能面板

---

## 四、最小侵入式集成方案

### 4.1 代码架构原则
**保持兼容性：**
- ✅ 不修改现有 `Renderer2D` 代码
- ✅ 新增 `Renderer3D` 模块
- ✅ 共享 `Shader`、`Texture`、`Buffer` 等基础类

**目录结构：**
```
XingXing/src/XingXing/
├── Renderer/
│   ├── Renderer2D.h/cpp      [保持不变]
│   ├── Renderer3D.h/cpp      [新增]
│   ├── PerspectiveCamera.h   [新增]
│   └── ...
├── World/                     [新增模块]
│   ├── Block.h
│   ├── Chunk.h/cpp
│   └── WorldGenerator.h
└── ...
```

### 4.2 集成步骤
1. **并行开发** - 3D 功能与 2D 功能独立
2. **共享基础设施** - Buffer、Shader、Texture 复用
3. **可选启用** - 通过编译选项控制 3D 功能
4. **渐进式迁移** - 允许项目逐步采用 3D 功能

### 4.3 向后兼容性
```cpp
// 示例：应用层可选择使用 2D 或 3D
class MyLayer : public Layer {
    void OnAttach() override {
        // 2D 渲染器（现有功能）
        Renderer2D::Init();
        
        // 3D 渲染器（新功能，可选）
        Renderer3D::Init();
    }
    
    void OnUpdate() {
        // 可以同时使用 2D 和 3D
        Renderer2D::BeginScene(m_Camera2D);
        // ... 2D 绘制
        Renderer2D::EndScene();
        
        Renderer3D::BeginScene(m_Camera3D);
        // ... 3D 绘制
        Renderer3D::EndScene();
    }
};
```

---

## 五、3D 方块场景示例代码

### 5.1 基础场景设置
```cpp
// Sandbox/src/VoxelWorldLayer.h
#pragma once
#include <XingXing.h>

class VoxelWorldLayer : public Hazel::Layer
{
public:
    VoxelWorldLayer();
    virtual ~VoxelWorldLayer() = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Hazel::Timestep ts) override;
    void OnImGuiRender() override;
    void OnEvent(Hazel::Event& e) override;

private:
    // 相机
    Hazel::PerspectiveCamera m_Camera;
    Hazel::FirstPersonCameraController m_CameraController;
    
    // 场景
    std::vector<glm::vec3> m_BlockPositions;
    Hazel::Ref<Hazel::Texture2D> m_BlockTexture;
};
```

### 5.2 实现示例
```cpp
// Sandbox/src/VoxelWorldLayer.cpp
#include "VoxelWorldLayer.h"

VoxelWorldLayer::VoxelWorldLayer()
    : Layer("VoxelWorld"),
      m_Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f),
      m_CameraController(m_Camera)
{
}

void VoxelWorldLayer::OnAttach()
{
    // 初始化 3D 渲染器
    Hazel::Renderer3D::Init();
    
    // 加载方块纹理
    m_BlockTexture = Hazel::Texture2D::Create("assets/textures/grass_block.png");
    
    // 创建简单的地面
    for (int x = -10; x < 10; x++) {
        for (int z = -10; z < 10; z++) {
            m_BlockPositions.push_back(glm::vec3(x, 0, z));
        }
    }
    
    // 设置相机初始位置
    m_CameraController.SetPosition(glm::vec3(0.0f, 5.0f, 10.0f));
}

void VoxelWorldLayer::OnUpdate(Hazel::Timestep ts)
{
    // 更新相机
    m_CameraController.OnUpdate(ts);
    
    // 渲染
    Hazel::RenderCommand::SetClearColor({ 0.53f, 0.81f, 0.92f, 1.0f }); // 天空蓝
    Hazel::RenderCommand::Clear();
    
    Hazel::Renderer3D::BeginScene(m_Camera);
    
    // 绘制所有方块
    for (const auto& pos : m_BlockPositions) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
        Hazel::Renderer3D::DrawCube(transform, m_BlockTexture);
    }
    
    Hazel::Renderer3D::EndScene();
}

void VoxelWorldLayer::OnImGuiRender()
{
    ImGui::Begin("3D Voxel World Stats");
    ImGui::Text("Blocks: %d", m_BlockPositions.size());
    
    auto stats = Hazel::Renderer3D::GetStats();
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    
    ImGui::End();
}
```

### 5.3 验证标准
**引擎升级成功的标志：**
1. ✅ 场景显示 20x20 的草方块平面
2. ✅ 使用 WASD 可以自由移动
3. ✅ 鼠标可以旋转视角
4. ✅ 方块有正确的深度关系
5. ✅ 光照效果可见（明暗面）
6. ✅ 帧率稳定在 60+ FPS

---

## 六、项目时间线估算

| 阶段 | 任务 | 预计时长 | 依赖 |
|------|------|----------|------|
| 0 | 准备工作 | 1-2 天 | 无 |
| 1 | 透视相机 | 2-3 天 | 阶段 0 |
| 2 | 3D 渲染器 | 3-4 天 | 阶段 1 |
| 3 | 方块系统 | 4-5 天 | 阶段 2 |
| 4 | 世界生成 | 2-3 天 | 阶段 3 |
| 5 | 优化工具 | 3-4 天 | 阶段 4 |

**总计：15-21 天（3-4 周）**

---

## 七、风险与缓解措施

### 风险 1：性能不足
**缓解：**
- 提前实现分块系统
- 使用实例化渲染
- 限制初始视距

### 风险 2：与现有代码冲突
**缓解：**
- 模块化设计
- 充分的单元测试
- 代码审查

### 风险 3：学习曲线陡峭
**缓解：**
- 详细的文档和示例
- 渐进式 API 设计
- 社区支持

---

## 八、下一步行动

### 立即开始（优先级 P0）：
1. 创建 `PerspectiveCamera` 类
2. 实现基础的 3D 着色器
3. 添加深度缓冲支持

### 短期目标（1-2 周）：
- 完成阶段 1-2
- 能够渲染单个带纹理的立方体

### 中期目标（3-4 周）：
- 完成完整的方块系统
- 实现简单的可玩 Demo

---

## 附录

### A. 参考资源
- [LearnOpenGL - Getting Started](https://learnopengl.com/)
- [Minecraft Wiki - Block Models](https://minecraft.fandom.com/wiki/Block_models)
- [Hazel Engine Series - TheCherno](https://www.youtube.com/watch?v=JxIZbV_XjAs&list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT)

### B. 社区支持
- 提交 Issue 或 PR 到仓库
- Discord/论坛讨论（如果有）

---

**文档版本：** v1.0  
**创建日期：** 2026-01-31  
**维护者：** XingXing 引擎开发团队
