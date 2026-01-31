# 🧱 XingXing 方块沙盒游戏改造指南

## 📋 目录
1. [概述](#概述)
2. [核心概念与知识](#核心概念与知识)
3. [系统架构设计](#系统架构设计)
4. [详细实现步骤](#详细实现步骤)
5. [代码实现示例](#代码实现示例)
6. [性能优化建议](#性能优化建议)
7. [进阶功能](#进阶功能)

---

## 概述

本指南将帮助你将 XingXing 引擎改造成专门的方块沙盒游戏引擎（类似 Minecraft 或 Terraria）。

### 当前引擎状态
- ✅ 基于 Hazel2D 的 2D 渲染引擎
- ✅ 已有 ECS（Entity Component System）架构
- ✅ 已有 Renderer2D 批处理渲染
- ✅ 已有物理系统（Box2D）
- ✅ 已有场景管理和实体系统

### 目标功能
- 🎯 方块网格世界系统
- 🎯 区块（Chunk）管理和优化
- 🎯 方块放置/破坏交互
- 🎯 方块类型和属性系统
- 🎯 世界生成算法
- 🎯 玩家与方块的碰撞检测
- 🎯 方块更新机制（如重力、流体）

---

## 核心概念与知识

### 1. 方块（Block）系统

**方块**是沙盒游戏的基本单位，每个方块占据世界中的一个网格位置。

#### 关键知识点：
- **方块类型（Block Type）**：定义方块的外观、物理属性、交互行为
- **方块状态（Block State）**：方块的额外信息（如方向、损坏程度）
- **方块属性**：
  - 是否可穿透（Transparent）
  - 硬度（Hardness）
  - 是否受重力影响（Affected by Gravity）
  - 光照属性（Light Emission/Absorption）
  - 碰撞盒（Collision Box）

#### 数据结构设计：
```cpp
// 方块ID类型（使用整数ID节省内存）
using BlockID = uint16_t;

// 方块类型定义
struct BlockType {
    BlockID ID;
    std::string Name;
    Ref<Texture2D> Texture;
    bool IsSolid;           // 是否为固体
    bool IsTransparent;     // 是否透明
    float Hardness;         // 硬度
    bool HasGravity;        // 是否受重力影响
    glm::vec4 Color;        // 基础颜色
};
```

### 2. 区块（Chunk）系统

**区块**是世界的分块单元，用于优化渲染和内存管理。

#### 关键知识点：
- **区块大小**：通常为 16x16 或 32x32 方块
- **区块坐标**：世界坐标 → 区块坐标的转换
- **区块加载/卸载**：根据玩家位置动态加载
- **脏标记（Dirty Flag）**：标记需要重新渲染的区块
- **区块网格优化**：合并相邻的相同方块减少绘制调用

#### 数据结构设计：
```cpp
constexpr int CHUNK_SIZE = 16;

class Chunk {
public:
    Chunk(int chunkX, int chunkY);
    
    // 获取/设置方块
    BlockID GetBlock(int x, int y) const;
    void SetBlock(int x, int y, BlockID blockID);
    
    // 区块状态
    bool IsDirty() const { return m_IsDirty; }
    void MarkDirty() { m_IsDirty = true; }
    
    // 渲染
    void RebuildMesh();
    void Render();
    
private:
    int m_ChunkX, m_ChunkY;
    BlockID m_Blocks[CHUNK_SIZE][CHUNK_SIZE];
    bool m_IsDirty;
    
    // 优化的渲染数据
    std::vector<QuadVertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
};
```

### 3. 世界（World）管理

**世界管理器**负责管理所有区块、世界生成、方块更新。

#### 关键知识点：
- **无限世界生成**：程序化生成（Procedural Generation）
- **噪声算法**：Perlin Noise / Simplex Noise 用于地形生成
- **区块流式加载**：玩家移动时异步加载/卸载区块
- **世界坐标系统**：
  - 世界坐标（World Coordinates）
  - 区块坐标（Chunk Coordinates）
  - 区块内坐标（Local Coordinates）

#### 坐标转换公式：
```cpp
// 世界坐标 → 区块坐标
glm::ivec2 WorldToChunk(const glm::vec2& worldPos) {
    return {
        static_cast<int>(std::floor(worldPos.x / CHUNK_SIZE)),
        static_cast<int>(std::floor(worldPos.y / CHUNK_SIZE))
    };
}

// 世界坐标 → 区块内坐标
glm::ivec2 WorldToLocal(const glm::vec2& worldPos) {
    int x = static_cast<int>(std::floor(worldPos.x));
    int y = static_cast<int>(std::floor(worldPos.y));
    return {
        x % CHUNK_SIZE,
        y % CHUNK_SIZE
    };
}
```

### 4. 方块渲染优化

#### 关键技术：
1. **批处理渲染（Batch Rendering）**
   - 合并多个方块为单个绘制调用
   - XingXing 已有的 Renderer2D 可以复用

2. **面剔除（Face Culling）**
   - 只渲染可见的方块面
   - 相邻方块的共享面不渲染

3. **视锥裁剪（Frustum Culling）**
   - 只渲染摄像机视野内的区块

4. **网格合并（Mesh Merging）**
   - 将相同材质的方块合并为一个大网格

#### 渲染优化示例：
```cpp
void Chunk::RebuildMesh() {
    m_Vertices.clear();
    m_Indices.clear();
    
    for (int y = 0; y < CHUNK_SIZE; y++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            BlockID blockID = m_Blocks[x][y];
            if (blockID == 0) continue; // 空气方块
            
            // 检查相邻方块，只渲染暴露的面
            bool renderLeft   = (x == 0) || IsTransparent(x-1, y);
            bool renderRight  = (x == CHUNK_SIZE-1) || IsTransparent(x+1, y);
            bool renderBottom = (y == 0) || IsTransparent(x, y-1);
            bool renderTop    = (y == CHUNK_SIZE-1) || IsTransparent(x, y+1);
            
            // 添加可见面的顶点
            if (renderLeft)   AddFaceVertices(x, y, Direction::Left);
            if (renderRight)  AddFaceVertices(x, y, Direction::Right);
            if (renderBottom) AddFaceVertices(x, y, Direction::Bottom);
            if (renderTop)    AddFaceVertices(x, y, Direction::Top);
        }
    }
    
    m_IsDirty = false;
}
```

### 5. 玩家交互系统

#### 关键功能：
1. **射线检测（Raycasting）**
   - 检测玩家指向哪个方块
   - DDA 算法或 Bresenham 算法

2. **方块放置/破坏**
   - 左键破坏，右键放置
   - 检测放置位置是否合法

3. **物品栏（Inventory）系统**
   - 存储玩家拥有的方块类型
   - 快捷栏选择

#### 射线检测示例：
```cpp
// DDA 算法进行方块射线检测
std::optional<glm::ivec2> RaycastBlock(
    const glm::vec2& origin,
    const glm::vec2& direction,
    float maxDistance)
{
    glm::vec2 pos = origin;
    glm::vec2 deltaDist = {
        std::abs(1.0f / direction.x),
        std::abs(1.0f / direction.y)
    };
    
    glm::ivec2 step = {
        direction.x < 0 ? -1 : 1,
        direction.y < 0 ? -1 : 1
    };
    
    // ... DDA 算法实现
    
    return std::nullopt; // 未击中任何方块
}
```

### 6. 物理与碰撞检测

#### AABB 碰撞检测：
- 玩家与方块的碰撞使用 AABB（Axis-Aligned Bounding Box）
- 检测玩家边界与周围方块的碰撞
- 处理滑动和重力

```cpp
bool CheckCollision(const AABB& player, const glm::ivec2& blockPos) {
    // 获取方块的 AABB
    AABB blockAABB = {
        glm::vec2(blockPos.x, blockPos.y),
        glm::vec2(blockPos.x + 1.0f, blockPos.y + 1.0f)
    };
    
    // AABB 碰撞检测
    return (player.min.x < blockAABB.max.x &&
            player.max.x > blockAABB.min.x &&
            player.min.y < blockAABB.max.y &&
            player.max.y > blockAABB.min.y);
}
```

---

## 系统架构设计

### 新增组件层次结构

```
XingXing/
├── src/XingXing/
│   ├── BlockWorld/              [新增] 方块世界系统
│   │   ├── Block.h/cpp          方块定义
│   │   ├── BlockRegistry.h/cpp  方块注册表
│   │   ├── Chunk.h/cpp          区块管理
│   │   ├── World.h/cpp          世界管理
│   │   ├── WorldGenerator.h/cpp 世界生成器
│   │   └── ChunkRenderer.h/cpp  区块渲染器
│   ├── Scene/
│   │   ├── Components.h         [修改] 添加方块相关组件
│   │   └── ...
│   └── Renderer/
│       ├── BlockRenderer.h/cpp  [新增] 专门的方块渲染器
│       └── ...
└── Sandbox/
    └── src/
        └── BlockSandboxLayer.cpp [新增] 方块沙盒演示层
```

### ECS 组件设计

添加到 `Components.h`：

```cpp
// 方块世界组件
struct BlockWorldComponent {
    Ref<World> WorldInstance;
    
    BlockWorldComponent() = default;
};

// 玩家控制器组件（用于方块交互）
struct BlockPlayerComponent {
    float MoveSpeed = 5.0f;
    float JumpForce = 10.0f;
    BlockID SelectedBlock = 1;
    int SelectedSlot = 0;
    
    BlockPlayerComponent() = default;
};

// 方块实体组件（如果需要将单个方块作为实体）
struct BlockEntityComponent {
    glm::ivec2 WorldPosition;
    BlockID Type;
    
    BlockEntityComponent() = default;
};
```

---

## 详细实现步骤

### 步骤 1：创建基础方块系统

#### 1.1 创建 Block.h
```cpp
#pragma once
#include "XingXing/Core/Core.h"
#include "XingXing/Renderer/Texture.h"
#include <glm/glm.hpp>

namespace Hazel {

    using BlockID = uint16_t;
    
    // 特殊方块ID
    constexpr BlockID BLOCK_AIR = 0;
    constexpr BlockID BLOCK_STONE = 1;
    constexpr BlockID BLOCK_DIRT = 2;
    constexpr BlockID BLOCK_GRASS = 3;
    
    struct BlockProperties {
        std::string Name;
        bool IsSolid = true;
        bool IsTransparent = false;
        bool HasGravity = false;
        float Hardness = 1.0f;
        glm::vec4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
        Ref<Texture2D> Texture = nullptr;
    };
    
    class Block {
    public:
        Block(BlockID id, const BlockProperties& props)
            : m_ID(id), m_Properties(props) {}
        
        BlockID GetID() const { return m_ID; }
        const BlockProperties& GetProperties() const { return m_Properties; }
        
        bool IsSolid() const { return m_Properties.IsSolid; }
        bool IsTransparent() const { return m_Properties.IsTransparent; }
        
    private:
        BlockID m_ID;
        BlockProperties m_Properties;
    };

} // namespace Hazel
```

#### 1.2 创建 BlockRegistry.h（方块注册表）
```cpp
#pragma once
#include "Block.h"
#include <unordered_map>

namespace Hazel {

    class BlockRegistry {
    public:
        static void Init();
        static void Shutdown();
        
        static void RegisterBlock(BlockID id, const BlockProperties& props);
        static const Block* GetBlock(BlockID id);
        static bool IsValidBlock(BlockID id);
        
    private:
        static std::unordered_map<BlockID, Ref<Block>> s_Blocks;
    };

} // namespace Hazel
```

### 步骤 2：实现区块系统

#### 2.1 创建 Chunk.h
```cpp
#pragma once
#include "Block.h"
#include "XingXing/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <array>

namespace Hazel {

    constexpr int CHUNK_SIZE = 16;
    
    class Chunk {
    public:
        Chunk(int chunkX, int chunkY);
        ~Chunk();
        
        // 方块操作
        BlockID GetBlock(int x, int y) const;
        void SetBlock(int x, int y, BlockID blockID);
        
        // 区块坐标
        glm::ivec2 GetChunkCoord() const { return {m_ChunkX, m_ChunkY}; }
        
        // 渲染
        void RebuildMesh();
        bool IsDirty() const { return m_IsDirty; }
        void SetDirty(bool dirty) { m_IsDirty = dirty; }
        
        // 辅助函数
        bool IsValidLocalCoord(int x, int y) const;
        
    private:
        int m_ChunkX, m_ChunkY;
        std::array<std::array<BlockID, CHUNK_SIZE>, CHUNK_SIZE> m_Blocks;
        bool m_IsDirty;
        
        // 渲染数据（可以使用 Renderer2D 的批处理）
        void GenerateRenderData();
    };

} // namespace Hazel
```

### 步骤 3：创建世界管理器

#### 3.1 创建 World.h
```cpp
#pragma once
#include "Chunk.h"
#include <unordered_map>
#include <glm/glm.hpp>

namespace Hazel {

    // 区块坐标哈希函数
    struct ChunkCoordHash {
        size_t operator()(const glm::ivec2& coord) const {
            return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.y) << 1);
        }
    };
    
    class World {
    public:
        World();
        ~World();
        
        // 方块操作（世界坐标）
        BlockID GetBlock(int worldX, int worldY) const;
        void SetBlock(int worldX, int worldY, BlockID blockID);
        
        // 区块管理
        Chunk* GetChunk(int chunkX, int chunkY);
        Chunk* GetOrCreateChunk(int chunkX, int chunkY);
        void UnloadChunk(int chunkX, int chunkY);
        
        // 更新
        void Update(const glm::vec2& playerPosition);
        void Render(const glm::mat4& viewProjection);
        
        // 坐标转换
        static glm::ivec2 WorldToChunk(int worldX, int worldY);
        static glm::ivec2 WorldToLocal(int worldX, int worldY);
        
    private:
        std::unordered_map<glm::ivec2, Ref<Chunk>, ChunkCoordHash> m_Chunks;
        int m_LoadRadius = 3; // 加载半径（以区块为单位）
        
        void LoadChunksAroundPlayer(const glm::vec2& playerPosition);
        void UnloadDistantChunks(const glm::vec2& playerPosition);
    };

} // namespace Hazel
```

### 步骤 4：集成到现有引擎

#### 4.1 修改 Components.h
在 `AllComponents` 中添加新组件：

```cpp
// 在 Components.h 中添加
struct BlockWorldComponent {
    Ref<World> WorldInstance;
    
    BlockWorldComponent() = default;
};

struct BlockPlayerComponent {
    float MoveSpeed = 5.0f;
    float JumpForce = 10.0f;
    float GravityScale = 1.0f;
    BlockID SelectedBlock = 1;
    
    // 方块交互
    float InteractionRange = 5.0f;
    
    BlockPlayerComponent() = default;
};

// 更新 AllComponents
using AllComponents = 
    ComponentGroup<TransformComponent, SpriteRendererComponent,
        CircleRendererComponent, CameraComponent, ScriptComponent,
        NativeScriptComponent, Rigidbody2DComponent, BoxCollider2DComponent,
        CircleCollider2DComponent, TextComponent,
        BlockWorldComponent, BlockPlayerComponent>; // [新增]
```

#### 4.2 在场景中使用方块世界

```cpp
// 在 Scene::OnRuntimeStart() 中初始化方块世界
void Scene::OnRuntimeStart() {
    // 现有代码...
    
    // 初始化方块世界
    auto view = m_Registry.view<BlockWorldComponent>();
    for (auto entity : view) {
        auto& worldComp = view.get<BlockWorldComponent>(entity);
        if (!worldComp.WorldInstance) {
            worldComp.WorldInstance = CreateRef<World>();
        }
    }
}
```

### 步骤 5：创建示例场景

#### 5.1 创建 BlockSandboxLayer.h
```cpp
#pragma once
#include "XingXing.h"
#include "XingXing/BlockWorld/World.h"

class BlockSandboxLayer : public Hazel::Layer {
public:
    BlockSandboxLayer();
    virtual ~BlockSandboxLayer() = default;
    
    virtual void OnAttach() override;
    virtual void OnDetach() override;
    
    void OnUpdate(Hazel::Timestep ts) override;
    virtual void OnImGuiRender() override;
    void OnEvent(Hazel::Event& e) override;
    
private:
    Hazel::OrthographicCameraController m_CameraController;
    Hazel::Ref<Hazel::World> m_World;
    
    // 玩家状态
    glm::vec2 m_PlayerPosition = {0.0f, 10.0f};
    glm::vec2 m_PlayerVelocity = {0.0f, 0.0f};
    Hazel::BlockID m_SelectedBlock = Hazel::BLOCK_STONE;
    
    // 输入处理
    void HandleInput(Hazel::Timestep ts);
    void HandleBlockInteraction();
    
    // 物理
    void UpdatePhysics(Hazel::Timestep ts);
};
```

#### 5.2 创建 BlockSandboxLayer.cpp（核心逻辑）
```cpp
#include "BlockSandboxLayer.h"
#include "XingXing/BlockWorld/BlockRegistry.h"
#include <imgui/imgui.h>

BlockSandboxLayer::BlockSandboxLayer()
    : Layer("BlockSandbox"), m_CameraController(1280.0f / 720.0f)
{
}

void BlockSandboxLayer::OnAttach() {
    HZ_PROFILE_FUNCTION();
    
    // 初始化方块注册表
    Hazel::BlockRegistry::Init();
    
    // 创建世界
    m_World = Hazel::CreateRef<Hazel::World>();
    
    // 生成一些初始地形
    for (int x = -50; x < 50; x++) {
        for (int y = 0; y < 5; y++) {
            m_World->SetBlock(x, y, Hazel::BLOCK_DIRT);
        }
        m_World->SetBlock(x, 5, Hazel::BLOCK_GRASS);
    }
}

void BlockSandboxLayer::OnUpdate(Hazel::Timestep ts) {
    HZ_PROFILE_FUNCTION();
    
    // 处理输入
    HandleInput(ts);
    
    // 更新物理
    UpdatePhysics(ts);
    
    // 更新摄像机（跟随玩家）
    m_CameraController.OnUpdate(ts);
    
    // 更新世界
    m_World->Update(m_PlayerPosition);
    
    // 渲染
    Hazel::RenderCommand::SetClearColor({0.53f, 0.81f, 0.92f, 1.0f}); // 天空蓝
    Hazel::RenderCommand::Clear();
    
    Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
    
    // 渲染世界
    m_World->Render(m_CameraController.GetCamera().GetViewProjectionMatrix());
    
    // 渲染玩家（简单的方块）
    Hazel::Renderer2D::DrawQuad(
        {m_PlayerPosition.x, m_PlayerPosition.y, 0.1f},
        {0.8f, 1.8f},
        {1.0f, 0.0f, 0.0f, 1.0f} // 红色
    );
    
    Hazel::Renderer2D::EndScene();
}

void BlockSandboxLayer::HandleInput(Hazel::Timestep ts) {
    // 移动控制
    float speed = 5.0f;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::A))
        m_PlayerPosition.x -= speed * ts;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::D))
        m_PlayerPosition.x += speed * ts;
    
    // 跳跃
    if (Hazel::Input::IsKeyPressed(Hazel::Key::Space) && 
        m_PlayerVelocity.y == 0.0f) {
        m_PlayerVelocity.y = 10.0f;
    }
    
    // 方块交互
    if (Hazel::Input::IsMouseButtonPressed(Hazel::Mouse::ButtonLeft)) {
        HandleBlockInteraction();
    }
}

void BlockSandboxLayer::UpdatePhysics(Hazel::Timestep ts) {
    // 应用重力
    m_PlayerVelocity.y -= 20.0f * ts;
    
    // 更新位置
    m_PlayerPosition += m_PlayerVelocity * ts;
    
    // 简单的地面碰撞检测
    int blockX = static_cast<int>(std::floor(m_PlayerPosition.x));
    int blockY = static_cast<int>(std::floor(m_PlayerPosition.y));
    
    if (m_World->GetBlock(blockX, blockY - 1) != Hazel::BLOCK_AIR) {
        m_PlayerPosition.y = std::ceil(m_PlayerPosition.y);
        m_PlayerVelocity.y = 0.0f;
    }
}

void BlockSandboxLayer::HandleBlockInteraction() {
    // 简单的方块放置/破坏
    auto [mouseX, mouseY] = Hazel::Input::GetMousePosition();
    // TODO: 实现射线检测和方块交互
}
```

### 步骤 6：实现世界生成

#### 6.1 创建 WorldGenerator.h
```cpp
#pragma once
#include "World.h"

namespace Hazel {

    class WorldGenerator {
    public:
        static void GenerateChunk(Chunk* chunk);
        
    private:
        // Perlin Noise 或 Simplex Noise
        static float GetNoise(float x, float y);
        static int GetTerrainHeight(int worldX);
    };

} // namespace Hazel
```

#### 6.2 使用 Perlin Noise 生成地形
```cpp
#include "WorldGenerator.h"
#include <cmath>

namespace Hazel {

    void WorldGenerator::GenerateChunk(Chunk* chunk) {
        glm::ivec2 chunkCoord = chunk->GetChunkCoord();
        
        for (int localX = 0; localX < CHUNK_SIZE; localX++) {
            int worldX = chunkCoord.x * CHUNK_SIZE + localX;
            int terrainHeight = GetTerrainHeight(worldX);
            
            for (int localY = 0; localY < CHUNK_SIZE; localY++) {
                int worldY = chunkCoord.y * CHUNK_SIZE + localY;
                
                if (worldY < terrainHeight - 3) {
                    chunk->SetBlock(localX, localY, BLOCK_STONE);
                } else if (worldY < terrainHeight) {
                    chunk->SetBlock(localX, localY, BLOCK_DIRT);
                } else if (worldY == terrainHeight) {
                    chunk->SetBlock(localX, localY, BLOCK_GRASS);
                } else {
                    chunk->SetBlock(localX, localY, BLOCK_AIR);
                }
            }
        }
    }
    
    int WorldGenerator::GetTerrainHeight(int worldX) {
        // 简单的正弦波地形
        float noise = std::sin(worldX * 0.1f) * 3.0f;
        return static_cast<int>(10.0f + noise);
    }
    
    // TODO: 实现真正的 Perlin Noise

} // namespace Hazel
```

---

## 代码实现示例

### 完整的 Chunk 实现示例

```cpp
// Chunk.cpp
#include "Chunk.h"
#include "BlockRegistry.h"
#include "XingXing/Renderer/Renderer2D.h"

namespace Hazel {

    Chunk::Chunk(int chunkX, int chunkY)
        : m_ChunkX(chunkX), m_ChunkY(chunkY), m_IsDirty(true)
    {
        // 初始化为空气
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                m_Blocks[x][y] = BLOCK_AIR;
            }
        }
    }
    
    BlockID Chunk::GetBlock(int x, int y) const {
        if (!IsValidLocalCoord(x, y))
            return BLOCK_AIR;
        return m_Blocks[x][y];
    }
    
    void Chunk::SetBlock(int x, int y, BlockID blockID) {
        if (!IsValidLocalCoord(x, y))
            return;
        
        if (m_Blocks[x][y] != blockID) {
            m_Blocks[x][y] = blockID;
            m_IsDirty = true;
        }
    }
    
    bool Chunk::IsValidLocalCoord(int x, int y) const {
        return x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE;
    }
    
    void Chunk::RebuildMesh() {
        if (!m_IsDirty)
            return;
        
        // 区块网格已重建，不再脏
        m_IsDirty = false;
    }

} // namespace Hazel
```

### 完整的 World 渲染示例

```cpp
// World.cpp
void World::Render(const glm::mat4& viewProjection) {
    for (auto& [coord, chunk] : m_Chunks) {
        if (chunk->IsDirty())
            chunk->RebuildMesh();
        
        // 渲染区块中的每个方块
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                BlockID blockID = chunk->GetBlock(x, y);
                if (blockID == BLOCK_AIR)
                    continue;
                
                const Block* block = BlockRegistry::GetBlock(blockID);
                if (!block)
                    continue;
                
                // 计算世界位置
                float worldX = coord.x * CHUNK_SIZE + x;
                float worldY = coord.y * CHUNK_SIZE + y;
                
                // 使用 Renderer2D 绘制方块
                const auto& props = block->GetProperties();
                if (props.Texture) {
                    Hazel::Renderer2D::DrawQuad(
                        {worldX, worldY, 0.0f},
                        {1.0f, 1.0f},
                        props.Texture,
                        1.0f,
                        props.Color
                    );
                } else {
                    Hazel::Renderer2D::DrawQuad(
                        {worldX, worldY, 0.0f},
                        {1.0f, 1.0f},
                        props.Color
                    );
                }
            }
        }
    }
}
```

---

## 性能优化建议

### 1. 渲染优化
- ✅ **批处理**：使用 Renderer2D 的批处理能力
- ✅ **面剔除**：只渲染可见的方块面
- ✅ **视锥裁剪**：只渲染视野内的区块
- ✅ **网格合并**：将相同纹理的方块合并

### 2. 内存优化
- ✅ **区块卸载**：卸载远离玩家的区块
- ✅ **使用 BlockID**：用整数代替对象节省内存
- ✅ **压缩存储**：对空区块使用特殊优化

### 3. 计算优化
- ✅ **脏标记**：只重建改变的区块
- ✅ **异步加载**：后台线程加载区块
- ✅ **空间索引**：使用网格或四叉树加速查询

### 4. 数据结构优化
```cpp
// 使用位压缩存储简单方块数据
struct CompressedChunk {
    // 每个方块用 16 位存储
    uint16_t blocks[CHUNK_SIZE * CHUNK_SIZE];
    
    // 或者使用 Run-Length Encoding (RLE) 压缩
    struct RLEEntry {
        BlockID blockID;
        uint16_t count;
    };
    std::vector<RLEEntry> rleData;
};
```

---

## 进阶功能

### 1. 流体模拟
```cpp
struct FluidBlock {
    BlockID Type; // BLOCK_WATER, BLOCK_LAVA
    float Level;  // 0.0 - 1.0
};

void UpdateFluid(World* world, int x, int y, float dt);
```

### 2. 光照系统
```cpp
struct LightingData {
    uint8_t sunlight;    // 0-15
    uint8_t blocklight;  // 0-15
};

void PropagateLight(Chunk* chunk, int x, int y);
void RemoveLight(Chunk* chunk, int x, int y);
```

### 3. 方块实体（Tile Entity）
```cpp
class BlockEntity {
public:
    virtual void Update(float dt) = 0;
    virtual void Serialize() = 0;
};

class ChestBlockEntity : public BlockEntity {
    Inventory m_Inventory;
};
```

### 4. 多人联机
- 客户端-服务器架构
- 方块同步协议
- 网络压缩和预测

### 5. 模组支持
- Lua/C# 脚本接口
- 方块注册 API
- 事件系统

---

## 总结

改造 XingXing 引擎为方块沙盒游戏需要：

### 核心系统：
1. ✅ 方块系统（Block System）
2. ✅ 区块系统（Chunk System）
3. ✅ 世界管理（World Management）
4. ✅ 渲染优化（Rendering Optimization）
5. ✅ 玩家交互（Player Interaction）
6. ✅ 物理碰撞（Physics & Collision）

### 关键知识：
- 📚 数据结构设计（方块、区块、世界）
- 📚 程序化生成（Perlin Noise）
- 📚 批处理渲染
- 📚 空间分割和优化
- 📚 射线检测算法

### 建议学习路径：
1. 先实现基础的方块放置/破坏
2. 添加简单的地形生成
3. 优化渲染性能
4. 添加玩家物理
5. 实现高级功能（流体、光照等）

祝你改造成功！🎮✨
