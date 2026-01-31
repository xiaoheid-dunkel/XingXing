# XingXing 沙盒游戏引擎使用指南

## 概述

XingXing 引擎现在是一个专为沙盒类游戏（类似《我的世界》）设计的游戏引擎。本指南将帮助你了解如何使用引擎的沙盒游戏功能。

## 核心概念

### 1. 体素世界系统

体素世界由方块（Blocks）组成，这些方块被组织成区块（Chunks）：

- **方块 (Block)**: 世界中的最小单位，有不同的类型（草地、石头、泥土等）
- **区块 (Chunk)**: 16x256x16 的方块集合，是世界管理的基本单位
- **世界 (World)**: 由多个区块组成的完整游戏世界

### 2. 方块类型

引擎支持以下方块类型：

```cpp
enum class BlockType : uint8_t
{
    Air = 0,      // 空气（不可见）
    Grass,        // 草方块
    Dirt,         // 泥土
    Stone,        // 石头
    Wood,         // 木头
    Leaves,       // 树叶（半透明）
    Sand,         // 沙子
    Water,        // 水（透明液体）
    Bedrock,      // 基岩（不可破坏）
    Coal,         // 煤矿
    Iron,         // 铁矿
    Gold,         // 金矿
    Diamond       // 钻石矿
};
```

## 使用组件

### VoxelComponent - 单个方块组件

用于表示单个方块实体：

```cpp
// 创建一个方块实体
auto blockEntity = scene->CreateEntity("Block");
auto& voxel = blockEntity.AddComponent<VoxelComponent>(
    BlockType::Stone,              // 方块类型
    glm::ivec3(10, 64, 10)        // 网格位置
);
```

### ChunkComponent - 区块组件

管理一个 16x256x16 的方块区域：

```cpp
// 创建区块实体
auto chunkEntity = scene->CreateEntity("Chunk");
auto& chunk = chunkEntity.AddComponent<ChunkComponent>(
    glm::ivec3(0, 0, 0)           // 区块坐标
);

// 设置方块
chunk.SetBlock(5, 64, 5, Block(BlockType::Grass));

// 获取方块
const Block& block = chunk.GetBlock(5, 64, 5);
```

### WorldComponent - 世界组件

管理整个世界的区块：

```cpp
auto worldEntity = scene->CreateEntity("World");
auto& world = worldEntity.AddComponent<WorldComponent>();

// 设置世界参数
world.RenderDistance = 8;        // 渲染距离（区块数）
world.Seed = 12345;              // 世界生成种子

// 检查区块是否已加载
bool loaded = world.IsChunkLoaded(glm::ivec3(0, 0, 0));
```

## 地形生成

### TerrainGeneratorComponent - 地形生成器

配置地形生成参数：

```cpp
auto& terrainGen = worldEntity.AddComponent<TerrainGeneratorComponent>();

// 配置地形参数
terrainGen.Scale = 0.05f;                 // 噪声缩放
terrainGen.HeightMultiplier = 32.0f;      // 高度倍数
terrainGen.BaseHeight = 64;               // 基础高度
terrainGen.WaterLevel = 62;               // 水位线
```

### 使用 WorldGenerator 生成地形

```cpp
#include "XingXing/Scene/WorldGenerator.h"

// 创建世界生成器
WorldGenerator generator(12345);  // 使用种子

// 生成区块地形
ChunkComponent& chunk = /* 获取区块组件 */;
TerrainGeneratorComponent& params = /* 获取地形参数 */;

generator.GenerateChunk(chunk, params);
```

## 玩家交互系统

### PlayerInventoryComponent - 玩家物品栏

管理玩家的 36 格物品栏（9个快捷栏 + 27个主物品栏）：

```cpp
auto playerEntity = scene->CreateEntity("Player");
auto& inventory = playerEntity.AddComponent<PlayerInventoryComponent>();

// 添加物品
inventory.AddItem(BlockType::Stone, 64);   // 添加64个石头
inventory.AddItem(BlockType::Wood, 32);    // 添加32个木头

// 移除物品
inventory.RemoveItem(BlockType::Stone, 10); // 移除10个石头

// 选择快捷栏格子
inventory.SelectedSlot = 0;  // 选择第一个格子

// 获取当前选中的物品
auto& selectedItem = inventory.GetSelectedItem();
if (selectedItem.Type != BlockType::Air) {
    // 使用选中的物品
}
```

### BlockInteractionComponent - 方块交互

处理方块的放置和破坏：

```cpp
auto& interaction = playerEntity.AddComponent<BlockInteractionComponent>();

// 设置交互参数
interaction.ReachDistance = 5.0f;          // 最大交互距离
interaction.IsBreaking = true;             // 正在破坏方块
interaction.TargetBlockPos = glm::ivec3(10, 64, 10);  // 目标方块位置
interaction.BreakProgress = 0.5f;          // 破坏进度（0.0-1.0）
```

## 完整示例

### 创建一个简单的沙盒世界

```cpp
#include "XingXing/Scene/Scene.h"
#include "XingXing/Scene/Components.h"
#include "XingXing/Scene/SandboxComponents.h"
#include "XingXing/Scene/WorldGenerator.h"

void CreateSandboxWorld(Ref<Scene> scene)
{
    // 1. 创建世界实体
    auto worldEntity = scene->CreateEntity("World");
    auto& world = worldEntity.AddComponent<WorldComponent>();
    world.Seed = 12345;
    world.RenderDistance = 8;
    
    // 2. 添加地形生成参数
    auto& terrainGen = worldEntity.AddComponent<TerrainGeneratorComponent>();
    terrainGen.Scale = 0.05f;
    terrainGen.HeightMultiplier = 32.0f;
    terrainGen.BaseHeight = 64;
    terrainGen.WaterLevel = 62;
    
    // 3. 创建世界生成器
    WorldGenerator generator(world.Seed);
    
    // 4. 生成初始区块
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            auto chunkEntity = scene->CreateEntity("Chunk");
            auto& chunk = chunkEntity.AddComponent<ChunkComponent>(
                glm::ivec3(x, 0, z)
            );
            
            // 生成地形
            generator.GenerateChunk(chunk, terrainGen);
            
            // 注册到世界
            int64_t hash = WorldComponent::HashChunkPos(chunk.ChunkPosition);
            world.LoadedChunks[hash] = chunkEntity.GetUUID();
        }
    }
    
    // 5. 创建玩家实体
    auto player = scene->CreateEntity("Player");
    player.AddComponent<TransformComponent>(glm::vec3(0.0f, 70.0f, 0.0f));
    
    // 添加玩家物品栏
    auto& inventory = player.AddComponent<PlayerInventoryComponent>();
    inventory.AddItem(BlockType::Grass, 64);
    inventory.AddItem(BlockType::Stone, 64);
    inventory.AddItem(BlockType::Wood, 64);
    inventory.AddItem(BlockType::Dirt, 64);
    
    // 添加方块交互
    player.AddComponent<BlockInteractionComponent>();
}
```

### 渲染沙盒世界（2D 截面视图）

```cpp
void RenderSandboxWorld(Scene* scene)
{
    // 获取所有区块
    auto view = scene->GetAllEntitiesWith<ChunkComponent>();
    
    for (auto entity : view) {
        auto& chunk = view.get<ChunkComponent>(entity);
        
        // 渲染 2D 截面（Z=8 平面）
        const int viewZ = 8;
        const float blockSize = 0.5f;
        
        for (int x = 0; x < CHUNK_SIZE_X; x++) {
            for (int y = 0; y < 128; y++) {  // 只渲染前128格
                const Block& block = chunk.GetBlock(x, y, viewZ);
                
                if (block.Type == BlockType::Air)
                    continue;
                
                // 根据方块类型选择颜色
                glm::vec4 color = GetBlockColor(block.Type);
                
                // 计算位置
                glm::vec3 position = {
                    (chunk.ChunkPosition.x * CHUNK_SIZE_X + x) * blockSize,
                    y * blockSize,
                    0.0f
                };
                
                // 绘制方块
                Renderer2D::DrawQuad(position, {blockSize, blockSize}, color);
            }
        }
    }
}
```

## 性能优化建议

### 1. 区块加载和卸载

只加载玩家周围的区块：

```cpp
void UpdateChunks(WorldComponent& world, const glm::vec3& playerPos)
{
    glm::ivec3 playerChunk = {
        (int)floor(playerPos.x / CHUNK_SIZE_X),
        0,
        (int)floor(playerPos.z / CHUNK_SIZE_Z)
    };
    
    // 检查需要加载的区块
    for (int x = -world.RenderDistance; x <= world.RenderDistance; x++) {
        for (int z = -world.RenderDistance; z <= world.RenderDistance; z++) {
            glm::ivec3 chunkPos = playerChunk + glm::ivec3(x, 0, z);
            
            if (!world.IsChunkLoaded(chunkPos)) {
                // 加载新区块
                LoadChunk(chunkPos);
            }
        }
    }
    
    // 卸载远离的区块
    // TODO: 实现区块卸载逻辑
}
```

### 2. 网格优化

只为可见的方块面生成网格：

```cpp
bool ShouldRenderFace(const Block& currentBlock, const Block& neighborBlock)
{
    // 如果邻居是空气或透明方块，则渲染这个面
    return neighborBlock.IsTransparent() && !currentBlock.IsTransparent();
}
```

### 3. 批量渲染

使用 Renderer2D 的批处理系统来减少绘制调用：

```cpp
Renderer2D::BeginScene(camera);

// 批量绘制所有方块
for (auto& chunk : chunks) {
    RenderChunk(chunk);
}

Renderer2D::EndScene();
```

## 扩展功能

### 添加新的方块类型

1. 在 `BlockType` 枚举中添加新类型
2. 在方块颜色映射中添加对应颜色
3. 更新地形生成器以使用新方块类型

### 实现方块破坏动画

```cpp
void UpdateBlockBreaking(BlockInteractionComponent& interaction, Timestep ts)
{
    if (interaction.IsBreaking) {
        interaction.BreakProgress += ts * 0.5f;  // 0.5 = 2秒破坏一个方块
        
        if (interaction.BreakProgress >= 1.0f) {
            // 方块被破坏
            BreakBlock(interaction.TargetBlockPos);
            interaction.BreakProgress = 0.0f;
            interaction.IsBreaking = false;
        }
    }
}
```

### 添加物理系统

结合现有的 Box2D 物理系统来添加掉落物品：

```cpp
void CreateDroppedItem(const glm::vec3& position, BlockType type)
{
    auto item = scene->CreateEntity("DroppedItem");
    item.AddComponent<TransformComponent>(position);
    
    // 添加物理组件
    auto& rb = item.AddComponent<Rigidbody2DComponent>();
    rb.Type = Rigidbody2DComponent::BodyType::Dynamic;
    
    // 添加方块数据
    auto& voxel = item.AddComponent<VoxelComponent>(type, glm::ivec3(position));
}
```

## 故障排查

### 区块生成问题

- 确保世界种子是一致的
- 检查地形生成器参数是否合理
- 验证区块坐标计算是否正确

### 渲染性能问题

- 减少渲染距离
- 实现视锥剔除
- 使用网格优化（只渲染可见面）

### 内存使用问题

- 实现区块卸载系统
- 使用区块缓存池
- 压缩未激活的区块数据

## 下一步

- 实现 3D 渲染系统
- 添加生物群系支持
- 实现多人游戏功能
- 添加更多方块类型和物品
- 实现保存/加载系统

---

有关更多信息，请参阅：
- [XingXing 引擎文档](../README.md)
- [API 参考](./API_Reference.md)
- [示例项目](../Sandbox/)
