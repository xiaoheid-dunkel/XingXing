# XingXing 3D 引擎架构图

## 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层 (Application)                     │
│                                                               │
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │   2D Layers     │  │   3D Layers     │  │ Editor Layers│ │
│  │ (Sandbox2D)     │  │ (VoxelWorld)    │  │              │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
└───────────────┬─────────────────┬───────────────────────────┘
                │                 │
                ▼                 ▼
┌───────────────────────┐ ┌──────────────────────┐
│    Renderer2D         │ │    Renderer3D        │  ◄── 新增
│  ┌─────────────────┐  │ │  ┌────────────────┐  │
│  │ Quad Batching   │  │ │  │ Cube Batching  │  │
│  │ Sprite Rendering│  │ │  │ Voxel Rendering│  │
│  │ Text Rendering  │  │ │  │ Lighting       │  │
│  └─────────────────┘  │ │  └────────────────┘  │
└───────────┬───────────┘ └──────────┬───────────┘
            │                        │
            └────────────┬───────────┘
                         ▼
          ┌──────────────────────────┐
          │   共享渲染基础设施         │
          │                          │
          │  • Shader                │
          │  • Texture               │
          │  • Buffer (Vertex/Index) │
          │  • VertexArray           │
          │  • Framebuffer           │
          └──────────┬───────────────┘
                     ▼
          ┌──────────────────────────┐
          │   RenderCommand           │
          │   (OpenGL API Wrapper)    │
          └──────────┬───────────────┘
                     ▼
          ┌──────────────────────────┐
          │   OpenGL / Graphics API   │
          └──────────────────────────┘
```

## 相机系统

```
          ┌──────────────────┐
          │   Camera (Base)  │
          └────────┬─────────┘
                   │
         ┌─────────┴──────────┐
         ▼                    ▼
┌─────────────────┐  ┌──────────────────┐
│OrthographicCamera│  │PerspectiveCamera │ ◄── 新增
│                  │  │                  │
│ • 2D Projection  │  │ • 3D Projection  │
│ • Zoom           │  │ • FOV            │
│ • Position (X,Y) │  │ • Position (XYZ) │
└─────────────────┘  │ • Rotation       │
                     │ • LookAt         │
                     └──────────────────┘
```

## 3D 渲染管线

```
应用层
  │
  ├─► Renderer3D::Init()
  │     └─► 创建 Vertex/Index Buffer
  │     └─► 加载着色器
  │     └─► 初始化纹理槽
  │
  ├─► Renderer3D::BeginScene(camera)
  │     └─► 设置 View-Projection 矩阵
  │     └─► 重置批次
  │
  ├─► Renderer3D::DrawCube(...) × N
  │     └─► 填充顶点数据到批次缓冲
  │     └─► 如果批次满，自动 Flush
  │
  ├─► Renderer3D::EndScene()
  │     └─► Flush 剩余数据
  │
  └─► Renderer3D::Shutdown()
        └─► 清理资源

Flush 过程：
  1. 上传顶点数据到 GPU
  2. 绑定所有纹理
  3. 绑定着色器
  4. DrawIndexed 调用
  5. 统计信息更新
```

## 立方体顶点结构

```
每个立方体 = 24 个顶点（6面 × 4顶点）

CubeVertex 结构 (52 bytes):
┌─────────────────────────────────┐
│ Position (vec3)    12 bytes     │
├─────────────────────────────────┤
│ Normal (vec3)      12 bytes     │
├─────────────────────────────────┤
│ Color (vec4)       16 bytes     │
├─────────────────────────────────┤
│ TexCoord (vec2)     8 bytes     │
├─────────────────────────────────┤
│ TexIndex (float)    4 bytes     │
└─────────────────────────────────┘

立方体的 6 个面：
     Top
      │
Left──┼──Right
      │
    Bottom
      │
  Front / Back

每面的顶点布局：
  3───2
  │   │
  0───1
```

## 批量渲染系统

```
┌─────────────────────────────────────────┐
│         批次缓冲区 (Batch Buffer)        │
│                                         │
│  最大容量: 1000 立方体                   │
│  = 24,000 顶点                          │
│  = 36,000 索引                          │
│                                         │
│  ┌───────┬───────┬───────┬─────────┐   │
│  │Cube 1 │Cube 2 │Cube 3 │  ...    │   │
│  │24 vts │24 vts │24 vts │         │   │
│  └───────┴───────┴───────┴─────────┘   │
│                                         │
│  当达到 1000 个或纹理槽满时              │
│  自动 Flush 并开始新批次                │
└─────────────────────────────────────────┘

优点:
• 减少 Draw Call (1000个方块 = 1个Draw Call)
• GPU 友好的数据布局
• 自动纹理批处理
```

## 光照系统

```
简单的 Phong 光照模型

          光源
           │
           │ Light Direction
           │
           ▼
     ┌─────────┐
     │  方块   │ ◄─── 观察者
     │  表面   │      (相机)
     └─────────┘
         │
         │ 反射光
         ▼

计算公式：
  Final Color = Texture Color × (Ambient + Diffuse)
  
  Ambient  = 0.3 (30% 环境光)
  Diffuse  = max(dot(Normal, LightDir), 0.0)
  
着色器代码:
  vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
  float diff = max(dot(normal, lightDir), 0.0);
  float lighting = 0.3 + 0.7 * diff;
```

## 纹理系统

```
┌─────────────────────────────────┐
│    纹理槽 (32个)                 │
│                                 │
│  [0] White Texture (默认)       │
│  [1] Grass Texture              │
│  [2] Stone Texture              │
│  [3] Dirt Texture               │
│  ...                            │
│  [31] 用户纹理                  │
└─────────────────────────────────┘

绘制时自动管理：
1. 检查纹理是否已绑定
2. 如果未绑定，分配新槽位
3. 如果槽位满，Flush 并重新开始
4. 在着色器中使用 TexIndex 访问

未来扩展：纹理图集
┌─────────────────────────┐
│ [0,0] [0,1] [0,2] [0,3] │ Grass variants
│ [1,0] [1,1] [1,2] [1,3] │ Stone variants
│ [2,0] [2,1] [2,2] [2,3] │ Wood variants
│ [3,0] [3,1] [3,2] [3,3] │ Ore variants
└─────────────────────────┘
   通过 UV 坐标访问不同方块
```

## 性能优化策略

```
当前已实现:
┌────────────────────────────────────┐
│ 1. 批量渲染                        │
│    • 1000 cubes/batch              │
│    • 减少 Draw Call                 │
├────────────────────────────────────┤
│ 2. 深度测试                        │
│    • glEnable(GL_DEPTH_TEST)       │
│    • 正确处理遮挡                   │
├────────────────────────────────────┤
│ 3. 背面剔除                        │
│    • glEnable(GL_CULL_FACE)        │
│    • 不渲染看不见的面               │
├────────────────────────────────────┤
│ 4. 多纹理批处理                    │
│    • 32 纹理/批次                   │
│    • 减少状态切换                   │
└────────────────────────────────────┘

未来可实现:
┌────────────────────────────────────┐
│ 5. 视锥裁剪 (Frustum Culling)      │
│    • 只渲染相机视野内的方块         │
├────────────────────────────────────┤
│ 6. 遮挡剔除 (Occlusion Culling)    │
│    • 不渲染被完全遮挡的方块         │
├────────────────────────────────────┤
│ 7. 分块系统 (Chunk System)         │
│    • 16×16×16 方块/区块             │
│    • 只构建可见面的网格             │
├────────────────────────────────────┤
│ 8. LOD (Level of Detail)           │
│    • 远处方块使用简化模型           │
└────────────────────────────────────┘
```

## 文件组织

```
XingXing/
├── src/XingXing/
│   ├── Renderer/
│   │   ├── Renderer2D.h/cpp        [现有]
│   │   ├── Renderer3D.h/cpp        [新增] ⭐
│   │   ├── PerspectiveCamera.h/cpp [新增] ⭐
│   │   ├── Camera.h                [基类]
│   │   ├── Shader.h/cpp            [共享]
│   │   └── Texture.h/cpp           [共享]
│   └── Math/
│       └── Math.h/cpp              [扩展] ⭐
│
├── Sandbox/
│   ├── src/
│   │   ├── Sandbox2D.h/cpp         [2D示例]
│   │   └── VoxelWorldLayer.h/cpp   [3D示例] ⭐
│   └── assets/
│       └── shaders/
│           └── Renderer3D_Cube.glsl [新增] ⭐
│
└── docs/
    ├── 3D_UPGRADE_PLAN.md          [新增] ⭐
    ├── 3D_INTEGRATION_GUIDE.md     [新增] ⭐
    ├── QUICK_START_3D.md           [新增] ⭐
    └── IMPLEMENTATION_SUMMARY.md   [新增] ⭐
```

## 与 2D 系统共存

```
同一应用中可以同时使用 2D 和 3D：

void OnUpdate(Timestep ts)
{
    // 清屏
    RenderCommand::Clear();
    
    // 3D 场景（方块世界）
    Renderer3D::BeginScene(m_Camera3D);
    Renderer3D::DrawCube(...);
    Renderer3D::EndScene();
    
    // 2D UI 叠加层
    Renderer2D::BeginScene(m_Camera2D);
    Renderer2D::DrawQuad(...);  // HUD
    Renderer2D::DrawString(...); // 文本
    Renderer2D::EndScene();
}

渲染顺序:
1. 3D 场景 (深度缓冲启用)
2. 2D UI (深度缓冲可选)
3. ImGui (最后渲染)
```

## 关键类接口

```cpp
// PerspectiveCamera
class PerspectiveCamera {
    PerspectiveCamera(fov, aspect, near, far);
    void SetPosition(glm::vec3);
    void SetRotation(glm::vec3);  // Pitch, Yaw, Roll
    glm::mat4 GetViewProjectionMatrix();
};

// Renderer3D
class Renderer3D {
    static void Init();
    static void Shutdown();
    
    static void BeginScene(camera);
    static void EndScene();
    
    // 绘制方法
    static void DrawCube(pos, size, color);
    static void DrawCube(pos, size, texture);
    static void DrawCube(transform, color);
    static void DrawCube(transform, texture);
    
    // 统计
    static Statistics GetStats();
};
```

---

**图表版本:** v1.0  
**创建日期:** 2026-01-31  
**说明:** 本架构图展示了 XingXing 3D 升级后的完整系统设计
