# 沙盒游戏编程指南 (Sandbox Game Programming Guide)

## 📖 概述

本文档旨在帮助开发者理解沙盒游戏（如《我的世界》）的核心编程概念，并为使用 XingXing 引擎开发自己的沙盒游戏提供指导。

---

## 🎮 什么是沙盒游戏？

沙盒游戏（Sandbox Game）是一种为玩家提供高度自由的游戏类型，玩家可以在游戏世界中自由探索、建造、破坏和创造。最著名的代表作是《我的世界》（Minecraft）。

### 核心特征
- **开放世界**：超大规模的可探索世界
- **可交互环境**：几乎所有物体都可以被修改
- **创造性自由**：玩家可以自由创作和建造
- **程序生成**：世界内容通过算法自动生成

---

## 🧱 沙盒游戏的核心编程概念

### 1. 世界表示 (World Representation)

沙盒游戏需要表示一个巨大的三维世界。直接存储每个方块的信息是不现实的，因此需要：

#### 关键挑战：
- **内存限制**：无法将整个世界加载到内存中
- **性能要求**：需要快速访问和修改世界数据
- **无限世界**：理论上世界可以无限扩展

#### 解决方案：
使用 **区块系统 (Chunk System)** 和 **世界坐标系统 (World Coordinate System)**

---

## 📍 世界坐标系统

详细内容请参阅：[世界坐标系统文档](./WorldCoordinates.md)

### 基本概念
世界坐标系统定义了游戏世界中每个位置的唯一标识。

```cpp
// 世界坐标示例
struct WorldPosition {
    int x;  // X 轴坐标（东西方向）
    int y;  // Y 轴坐标（高度/垂直方向）
    int z;  // Z 轴坐标（南北方向）
};
```

### 坐标类型
1. **世界坐标 (World Coordinates)**：全局绝对坐标
2. **区块坐标 (Chunk Coordinates)**：区块在世界中的位置
3. **局部坐标 (Local Coordinates)**：方块在区块内的相对位置

---

## 🗂️ 区块系统 (Chunk System)

详细内容请参阅：[区块系统文档](./ChunkSystem.md)

### 什么是区块？
区块（Chunk）是将世界划分成固定大小的立方体区域。在《我的世界》中，一个区块通常是 16×16×256（Java版）或 16×16×384（基岩版1.18+）。

### 为什么需要区块？
```
优势：
✓ 只加载玩家周围的区块，节省内存
✓ 简化渲染和物理计算
✓ 支持动态加载和卸载
✓ 便于多线程处理
✓ 简化保存和加载机制
```

### 区块的基本结构
```cpp
class Chunk {
public:
    static const int CHUNK_SIZE_X = 16;
    static const int CHUNK_SIZE_Y = 256;
    static const int CHUNK_SIZE_Z = 16;
    
    // 区块在世界中的位置
    int chunkX;
    int chunkZ;
    
    // 存储方块数据
    BlockType blocks[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
    
    // 区块状态
    bool isLoaded;
    bool isDirty;  // 是否需要重新生成网格
    
    // 方法
    void Generate();      // 生成地形
    void BuildMesh();     // 构建渲染网格
    void Save();          // 保存到磁盘
    void Load();          // 从磁盘加载
};
```

---

## 🔄 坐标转换

在沙盒游戏中，经常需要在不同坐标系统之间转换：

### 世界坐标 → 区块坐标
```cpp
// 将世界坐标转换为区块坐标
int GetChunkCoordinate(int worldCoord) {
    if (worldCoord >= 0) {
        return worldCoord / CHUNK_SIZE;
    } else {
        return (worldCoord - CHUNK_SIZE + 1) / CHUNK_SIZE;
    }
}

// 示例
// 世界坐标 35 → 区块坐标 2 (35 / 16 = 2)
// 世界坐标 -17 → 区块坐标 -2
```

### 世界坐标 → 局部坐标
```cpp
// 获取方块在区块内的局部坐标
int GetLocalCoordinate(int worldCoord) {
    int local = worldCoord % CHUNK_SIZE;
    if (local < 0) {
        local += CHUNK_SIZE;
    }
    return local;
}

// 示例
// 世界坐标 35 → 局部坐标 3 (35 % 16 = 3)
// 世界坐标 -17 → 局部坐标 15
```

### 完整转换示例
```cpp
// 访问世界坐标 (x, y, z) 处的方块
BlockType GetBlockAt(int worldX, int worldY, int worldZ) {
    // 1. 计算区块坐标
    int chunkX = GetChunkCoordinate(worldX);
    int chunkZ = GetChunkCoordinate(worldZ);
    
    // 2. 获取或加载区块
    Chunk* chunk = GetChunk(chunkX, chunkZ);
    if (!chunk) return BlockType::Air;
    
    // 3. 计算局部坐标
    int localX = GetLocalCoordinate(worldX);
    int localY = worldY;  // Y 坐标通常直接使用
    int localZ = GetLocalCoordinate(worldZ);
    
    // 4. 返回方块数据
    return chunk->GetBlock(localX, localY, localZ);
}
```

---

## 🎨 渲染优化

### 1. 视锥剔除 (Frustum Culling)
只渲染摄像机视野内的区块。

```cpp
void RenderWorld(Camera& camera) {
    for (auto& chunk : loadedChunks) {
        if (camera.IsChunkInView(chunk)) {
            chunk->Render();
        }
    }
}
```

### 2. 面剔除 (Face Culling)
只渲染可见的方块面，隐藏被遮挡的面。

```cpp
// 检查相邻方块，只渲染暴露的面
if (GetBlockAt(x+1, y, z) == BlockType::Air) {
    // 渲染右侧面
    AddFaceToMesh(Face::Right);
}
```

### 3. 网格合批 (Mesh Batching)
将整个区块的方块合并成一个网格，减少 Draw Call。

---

## 🌍 世界生成 (World Generation)

### 程序化生成
使用噪声算法（如 Perlin Noise）生成地形：

```cpp
void GenerateChunk(Chunk* chunk) {
    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            // 计算世界坐标
            int worldX = chunk->chunkX * CHUNK_SIZE_X + x;
            int worldZ = chunk->chunkZ * CHUNK_SIZE_Z + z;
            
            // 使用 Perlin 噪声生成地形高度
            float noise = PerlinNoise(worldX * 0.01f, worldZ * 0.01f);
            int height = (int)(noise * 32.0f) + 64;  // 基准高度 64
            
            // 填充方块
            for (int y = 0; y < height; y++) {
                if (y < height - 4) {
                    chunk->SetBlock(x, y, z, BlockType::Stone);
                } else if (y < height - 1) {
                    chunk->SetBlock(x, y, z, BlockType::Dirt);
                } else {
                    chunk->SetBlock(x, y, z, BlockType::Grass);
                }
            }
        }
    }
}
```

---

## 💾 数据存储

### 区块保存格式
```cpp
struct ChunkData {
    int32_t chunkX;
    int32_t chunkZ;
    uint32_t version;
    uint8_t blocks[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
    // 可以使用压缩算法减小文件大小
};

void SaveChunk(Chunk* chunk, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    ChunkData data;
    // 填充数据...
    file.write(reinterpret_cast<char*>(&data), sizeof(ChunkData));
}
```

### 优化存储
- **运行长度编码 (RLE)**：压缩连续相同的方块
- **区块版本控制**：支持数据格式升级
- **延迟保存**：只保存修改过的区块

---

## 🚀 性能优化建议

### 1. 多线程
- **地形生成**：在后台线程生成区块
- **网格构建**：并行构建多个区块的网格
- **IO操作**：异步加载和保存区块

### 2. 内存管理
- **区块池**：重用区块对象，避免频繁分配
- **智能加载**：根据玩家移动方向预加载区块
- **内存限制**：设置最大加载区块数量

### 3. LOD (Level of Detail)
远处的区块使用简化模型或降低细节。

---

## 📚 实现示例：使用 XingXing 引擎

### 基本世界管理器
```cpp
class WorldManager {
private:
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>> chunks;
    std::queue<ChunkPos> chunksToGenerate;
    int renderDistance = 8;  // 渲染距离（区块数）

public:
    void Update(const glm::vec3& playerPosition) {
        // 计算玩家所在区块
        int playerChunkX = GetChunkCoordinate(playerPosition.x);
        int playerChunkZ = GetChunkCoordinate(playerPosition.z);
        
        // 加载周围区块
        for (int x = -renderDistance; x <= renderDistance; x++) {
            for (int z = -renderDistance; z <= renderDistance; z++) {
                ChunkPos pos(playerChunkX + x, playerChunkZ + z);
                if (!IsChunkLoaded(pos)) {
                    LoadChunk(pos);
                }
            }
        }
        
        // 卸载远处区块
        UnloadDistantChunks(playerChunkX, playerChunkZ);
    }
    
    void Render(Hazel::Renderer2D& renderer, const Camera& camera) {
        for (auto& [pos, chunk] : chunks) {
            if (chunk->isLoaded && !chunk->isDirty) {
                chunk->Render(renderer);
            }
        }
    }
};
```

---

## 🎯 开发路线图

### 阶段 1：基础框架
1. 实现基本的坐标系统
2. 创建简单的区块结构
3. 实现基础的方块类型

### 阶段 2：渲染系统
1. 区块网格生成
2. 面剔除优化
3. 纹理系统

### 阶段 3：世界管理
1. 动态加载/卸载
2. 地形生成算法
3. 数据持久化

### 阶段 4：交互系统
1. 方块放置/破坏
2. 碰撞检测
3. 物理系统

### 阶段 5：高级功能
1. 多生物群系
2. 光照系统
3. 多人游戏支持

---

## 📖 相关资源

- [世界坐标系统详解](./WorldCoordinates.md)
- [区块系统详解](./ChunkSystem.md)
- [XingXing 引擎文档](../README.md)

---

## 💡 最佳实践

1. **先做小规模原型**：从小世界开始，逐步扩展
2. **性能监控**：持续监测 FPS 和内存使用
3. **模块化设计**：保持代码的可维护性
4. **参考现有实现**：学习 Minecraft、Minetest 等开源项目
5. **迭代开发**：不要追求一开始就完美

---

## ❓ 常见问题

**Q: 为什么不直接存储整个世界？**
A: 对于大型世界，存储所有方块会消耗巨大内存。区块系统允许按需加载。

**Q: 区块大小应该设置多少？**
A: 通常 16×16 是经过验证的良好选择，平衡了性能和内存。

**Q: 如何处理区块边界的渲染？**
A: 需要检查相邻区块的边缘方块来正确生成网格。

**Q: 世界有多大？**
A: 理论上可以无限大，但受限于坐标数据类型（int32 约 ±2000万格）。

---

祝你开发顺利！如有问题，欢迎在 Issues 中讨论。
