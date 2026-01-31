# 方块沙盒游戏系统 - 快速开始指南

## 🎮 概述

恭喜！XingXing引擎现在已经具备了完整的方块沙盒游戏开发能力。本指南将帮助你快速上手。

## 📦 已添加的文件

### 核心系统文件
```
XingXing/src/XingXing/BlockWorld/
├── Block.h              - 方块定义和属性
├── BlockRegistry.h/cpp  - 方块注册表（管理所有方块类型）
├── Chunk.h/cpp          - 区块系统（16x16方块）
└── World.h/cpp          - 世界管理器（区块加载/卸载）
```

### 示例代码
```
Sandbox/src/
├── BlockSandboxLayer.h   - 方块沙盒演示层头文件
└── BlockSandboxLayer.cpp - 完整的可玩demo实现
```

### 文档
```
根目录/
├── BLOCK_SANDBOX_GUIDE.md      - 详细技术指南（600+行）
├── IMPLEMENTATION_SUMMARY.md   - 实施总结
└── README.md                   - 已更新，包含方块系统说明
```

## 🚀 如何使用

### 步骤 1: 在你的Layer中使用方块世界

```cpp
#include "XingXing/BlockWorld/World.h"
#include "XingXing/BlockWorld/BlockRegistry.h"

class MyGameLayer : public Hazel::Layer
{
public:
    void OnAttach() override
    {
        // 初始化方块注册表
        Hazel::BlockRegistry::Init();
        
        // 创建世界
        m_World = Hazel::CreateRef<Hazel::World>();
        
        // 生成初始地形（可选）
        for (int x = -50; x < 50; x++)
        {
            for (int y = 0; y < 5; y++)
            {
                m_World->SetBlock(x, y, Hazel::BLOCK_DIRT);
            }
            m_World->SetBlock(x, 5, Hazel::BLOCK_GRASS);
        }
    }
    
    void OnUpdate(Hazel::Timestep ts) override
    {
        // 更新世界（加载/卸载区块）
        m_World->Update(playerPosition);
        
        // 渲染世界
        Hazel::Renderer2D::BeginScene(camera);
        m_World->Render(camera.GetViewProjectionMatrix());
        Hazel::Renderer2D::EndScene();
    }
    
private:
    Hazel::Ref<Hazel::World> m_World;
};
```

### 步骤 2: 操作方块

```cpp
// 放置方块
m_World->SetBlock(worldX, worldY, Hazel::BLOCK_STONE);

// 获取方块
Hazel::BlockID blockID = m_World->GetBlock(worldX, worldY);

// 检查方块属性
const Hazel::Block* block = Hazel::BlockRegistry::GetBlock(blockID);
if (block && block->IsSolid())
{
    // 这是固体方块，玩家不能穿过
}
```

### 步骤 3: 注册自定义方块

```cpp
// 创建方块属性
Hazel::BlockProperties customProps;
customProps.Name = "Diamond";
customProps.IsSolid = true;
customProps.IsTransparent = false;
customProps.Hardness = 3.0f;
customProps.Color = { 0.0f, 1.0f, 1.0f, 1.0f };  // 青色

// 注册方块（在 Init() 之后）
Hazel::BlockRegistry::RegisterBlock(100, customProps);
```

## 🎯 运行示例代码

### 方法 1: 修改 SandboxApp.cpp

在 `Sandbox/src/SandboxApp.cpp` 中添加：

```cpp
#include "BlockSandboxLayer.h"

class Sandbox : public Hazel::Application
{
public:
    Sandbox()
    {
        // 注释掉原有的 Layer
        // PushLayer(new Sandbox2D());
        
        // 添加方块沙盒层
        PushLayer(new BlockSandboxLayer());
    }
};
```

### 方法 2: 创建新的应用程序

复制 `Sandbox` 项目创建一个新的应用，直接使用 `BlockSandboxLayer`。

## 🎮 控制说明

在 `BlockSandboxLayer` 示例中：

| 按键/操作 | 功能 |
|----------|------|
| A | 向左移动 |
| D | 向右移动 |
| 空格 | 跳跃 |
| 鼠标左键 | 破坏方块 |
| 鼠标右键 | 放置方块 |
| 1 | 选择石头 |
| 2 | 选择泥土 |
| 3 | 选择草方块 |
| 4 | 选择木头 |
| 5 | 选择沙子 |
| 鼠标滚轮 | 缩放视角 |

## 🔧 编译项目

1. 运行 `scripts/Win-GenProjects.bat` 生成Visual Studio项目
2. 打开 `XingXing.sln`
3. 设置 `Sandbox` 为启动项目
4. 编译并运行（F5）

## 📚 深入学习

### 必读文档
1. **BLOCK_SANDBOX_GUIDE.md** - 详细的技术指南
   - 核心概念讲解
   - 完整实现步骤
   - 大量代码示例
   - 性能优化建议

2. **IMPLEMENTATION_SUMMARY.md** - 实施总结
   - 已完成功能列表
   - 代码结构说明
   - 使用方法
   - 扩展建议

3. **Sandbox/src/BlockSandboxLayer.cpp** - 实际代码
   - 完整的游戏循环实现
   - 玩家控制逻辑
   - 物理和碰撞检测
   - UI界面

## 🎨 自定义和扩展

### 添加新方块类型

在 `BlockRegistry::RegisterDefaultBlocks()` 中：

```cpp
// 钻石方块
BlockProperties diamondProps;
diamondProps.Name = "Diamond";
diamondProps.IsSolid = true;
diamondProps.Hardness = 3.0f;
diamondProps.Color = { 0.0f, 1.0f, 1.0f, 1.0f };
RegisterBlock(BLOCK_DIAMOND, diamondProps);
```

### 添加方块纹理

```cpp
// 在 BlockProperties 中设置纹理
blockProps.Texture = Hazel::Texture2D::Create("assets/textures/stone.png");
```

### 改进地形生成

修改 `World::GenerateChunk()` 函数，使用 Perlin Noise：

```cpp
// 示例：使用噪声生成更自然的地形
#include <noise/noise.h>  // 需要添加噪声库

void World::GenerateChunk(Chunk* chunk)
{
    noise::module::Perlin perlin;
    perlin.SetFrequency(0.1);
    
    // ... 使用 perlin.GetValue(x, 0, 0) 生成地形
}
```

### 添加更多玩家功能

在 `BlockSandboxLayer` 中扩展：

```cpp
// 飞行模式
bool m_IsFlying = false;
if (Input::IsKeyPressed(Key::F))
    m_IsFlying = !m_IsFlying;

// 如果飞行，禁用重力
if (!m_IsFlying)
    m_PlayerVelocity.y -= 30.0f * ts;
```

## ⚠️ 常见问题

### Q: 编译错误 "找不到 World.h"
A: 确保运行了 premake 生成项目文件，并且新文件被正确包含。

### Q: 方块不显示
A: 检查是否调用了 `BlockRegistry::Init()`，以及摄像机位置是否正确。

### Q: 性能问题
A: 
- 减小区块加载半径 `m_World->SetLoadRadius(2)`
- 未来可以实现面剔除优化
- 使用纹理图集减少纹理切换

### Q: 玩家穿过方块
A: 检查碰撞检测代码，确保使用了正确的玩家尺寸。

## 🚀 下一步

1. **尝试运行示例** - 编译并运行 `BlockSandboxLayer`
2. **阅读详细指南** - 查看 `BLOCK_SANDBOX_GUIDE.md`
3. **自定义方块** - 添加你自己的方块类型
4. **改进地形** - 实现更好的世界生成算法
5. **添加功能** - 参考指南中的进阶功能章节

## 💡 获取帮助

- 查看代码注释 - 所有关键函数都有详细的中文注释
- 阅读 `BLOCK_SANDBOX_GUIDE.md` - 包含深入的技术讲解
- 参考 `BlockSandboxLayer.cpp` - 完整的工作示例

祝你开发顺利！🎮✨

---

**提示**: 这只是一个起点。方块沙盒游戏有无限的可能性，发挥你的创意吧！
