# 世界坐标系统详解 (World Coordinate System Deep Dive)

## 📖 概述

世界坐标系统是沙盒游戏中定位和管理所有游戏对象位置的基础。本文档详细讲解坐标系统的设计、转换和应用。

---

## 🌐 坐标系统类型

在《我的世界》类型的沙盒游戏中，通常使用三种坐标系统：

### 1. 世界坐标 (World Coordinates)

**定义**：游戏世界中的绝对坐标，每个位置在整个世界中是唯一的。

```cpp
struct WorldCoordinate {
    int x;  // X 轴：东（正）/ 西（负）
    int y;  // Y 轴：上（正）/ 下（负），高度
    int z;  // Z 轴：南（正）/ 北（负）
};
```

**特点**：
- **唯一性**：每个坐标对应世界中唯一的位置
- **绝对性**：不随玩家或摄像机位置改变
- **范围**：通常使用 int32，范围约 ±21亿（理论上无限）

**示例**：
```
玩家位置：(100, 64, -50)
方块位置：(0, 0, 0) - 世界原点/出生点附近
```

---

### 2. 区块坐标 (Chunk Coordinates)

**定义**：区块在世界中的位置，以区块为单位。

```cpp
struct ChunkCoordinate {
    int x;  // 区块 X 坐标
    int z;  // 区块 Z 坐标
    // 注意：通常不包含 Y，因为区块是竖直的柱状
};
```

**计算公式**：
```cpp
// 世界坐标 → 区块坐标
int WorldToChunkCoord(int worldCoord, int chunkSize = 16) {
    if (worldCoord >= 0) {
        return worldCoord / chunkSize;
    } else {
        // 负数需要特殊处理
        return (worldCoord - chunkSize + 1) / chunkSize;
    }
}

// 示例：
// 世界 X = 35  → 区块 X = 2  (35 / 16 = 2)
// 世界 X = 16  → 区块 X = 1  (16 / 16 = 1)
// 世界 X = 0   → 区块 X = 0  (0 / 16 = 0)
// 世界 X = -1  → 区块 X = -1 ((-1 - 16 + 1) / 16 = -1)
// 世界 X = -16 → 区块 X = -1 ((-16 - 16 + 1) / 16 = -1)
// 世界 X = -17 → 区块 X = -2 ((-17 - 16 + 1) / 16 = -2)
```

**特点**：
- **粗粒度**：每个单位代表一个区块（16×16 方块）
- **管理方便**：便于快速定位和加载区块
- **2D坐标**：只有 X 和 Z，区块在 Y 轴上是完整的柱状

---

### 3. 局部坐标 (Local/Block Coordinates)

**定义**：方块在区块内的相对位置。

```cpp
struct LocalCoordinate {
    int x;  // 0-15（区块内 X 位置）
    int y;  // 0-255 或 0-383（区块内 Y 位置）
    int z;  // 0-15（区块内 Z 位置）
};
```

**计算公式**：
```cpp
// 世界坐标 → 局部坐标
int WorldToLocalCoord(int worldCoord, int chunkSize = 16) {
    int local = worldCoord % chunkSize;
    if (local < 0) {
        local += chunkSize;  // 处理负数
    }
    return local;
}

// 示例：
// 世界 X = 35  → 局部 X = 3  (35 % 16 = 3)
// 世界 X = 16  → 局部 X = 0  (16 % 16 = 0)
// 世界 X = 0   → 局部 X = 0  (0 % 16 = 0)
// 世界 X = -1  → 局部 X = 15 (需要加 16)
// 世界 X = -16 → 局部 X = 0  (需要加 16)
// 世界 X = -17 → 局部 X = 15 (需要加 16)
```

**特点**：
- **细粒度**：精确到单个方块
- **有界性**：范围固定（X, Z: 0-15；Y: 0-255/383）
- **高效访问**：直接映射到数组索引

---

## 🔄 坐标转换详解

### 完整转换类

```cpp
class CoordinateConverter {
public:
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int WORLD_HEIGHT = 256;

    // 1. 世界坐标 → 区块坐标
    static ChunkCoordinate WorldToChunk(int worldX, int worldZ) {
        return ChunkCoordinate{
            WorldToChunkCoord(worldX),
            WorldToChunkCoord(worldZ)
        };
    }
    
    // 2. 世界坐标 → 局部坐标
    static LocalCoordinate WorldToLocal(int worldX, int worldY, int worldZ) {
        return LocalCoordinate{
            WorldToLocalCoord(worldX),
            worldY,  // Y 坐标通常直接使用
            WorldToLocalCoord(worldZ)
        };
    }
    
    // 3. 区块坐标 + 局部坐标 → 世界坐标
    static WorldCoordinate ChunkLocalToWorld(
        const ChunkCoordinate& chunk,
        const LocalCoordinate& local
    ) {
        return WorldCoordinate{
            chunk.x * CHUNK_SIZE + local.x,
            local.y,
            chunk.z * CHUNK_SIZE + local.z
        };
    }
    
    // 4. 世界坐标 → 区块坐标 + 局部坐标（一步完成）
    static void WorldToChunkAndLocal(
        int worldX, int worldY, int worldZ,
        ChunkCoordinate& outChunk,
        LocalCoordinate& outLocal
    ) {
        outChunk.x = WorldToChunkCoord(worldX);
        outChunk.z = WorldToChunkCoord(worldZ);
        
        outLocal.x = WorldToLocalCoord(worldX);
        outLocal.y = worldY;
        outLocal.z = WorldToLocalCoord(worldZ);
    }

private:
    static int WorldToChunkCoord(int worldCoord) {
        return worldCoord >= 0 
            ? worldCoord / CHUNK_SIZE 
            : (worldCoord - CHUNK_SIZE + 1) / CHUNK_SIZE;
    }
    
    static int WorldToLocalCoord(int worldCoord) {
        int local = worldCoord % CHUNK_SIZE;
        return local < 0 ? local + CHUNK_SIZE : local;
    }
};
```

---

## 🎯 实际应用场景

### 场景 1：获取指定位置的方块

```cpp
class World {
private:
    std::unordered_map<ChunkCoordinate, Chunk*> chunks;

public:
    BlockType GetBlockAt(int worldX, int worldY, int worldZ) {
        // 1. 转换为区块坐标
        ChunkCoordinate chunkPos = CoordinateConverter::WorldToChunk(worldX, worldZ);
        
        // 2. 获取区块（如果不存在则加载或生成）
        Chunk* chunk = GetOrLoadChunk(chunkPos);
        if (!chunk) {
            return BlockType::Air;  // 区块未加载
        }
        
        // 3. 转换为局部坐标
        LocalCoordinate localPos = CoordinateConverter::WorldToLocal(worldX, worldY, worldZ);
        
        // 4. 从区块获取方块
        return chunk->GetBlock(localPos.x, localPos.y, localPos.z);
    }
    
    void SetBlockAt(int worldX, int worldY, int worldZ, BlockType type) {
        ChunkCoordinate chunkPos = CoordinateConverter::WorldToChunk(worldX, worldZ);
        Chunk* chunk = GetOrLoadChunk(chunkPos);
        if (!chunk) return;
        
        LocalCoordinate localPos = CoordinateConverter::WorldToLocal(worldX, worldY, worldZ);
        chunk->SetBlock(localPos.x, localPos.y, localPos.z, type);
    }
};
```

---

### 场景 2：玩家移动与区块加载

```cpp
class ChunkManager {
private:
    int renderDistance = 8;  // 渲染距离（区块数）
    std::set<ChunkCoordinate> loadedChunks;

public:
    void UpdatePlayerPosition(const glm::vec3& playerPosition) {
        // 1. 计算玩家所在区块
        ChunkCoordinate playerChunk = CoordinateConverter::WorldToChunk(
            (int)playerPosition.x, (int)playerPosition.z);
        int playerChunkX = playerChunk.x;
        int playerChunkZ = playerChunk.z;
        
        // 2. 确定需要加载的区块范围
        std::set<ChunkCoordinate> requiredChunks;
        
        for (int dx = -renderDistance; dx <= renderDistance; dx++) {
            for (int dz = -renderDistance; dz <= renderDistance; dz++) {
                // 可选：使用圆形加载范围
                if (dx * dx + dz * dz > renderDistance * renderDistance) {
                    continue;  // 跳过圆形外的区块
                }
                
                ChunkCoordinate chunkPos{
                    playerChunkX + dx,
                    playerChunkZ + dz
                };
                requiredChunks.insert(chunkPos);
            }
        }
        
        // 3. 加载新区块
        for (const auto& chunkPos : requiredChunks) {
            if (loadedChunks.find(chunkPos) == loadedChunks.end()) {
                LoadChunk(chunkPos);
                loadedChunks.insert(chunkPos);
            }
        }
        
        // 4. 卸载远处区块
        auto it = loadedChunks.begin();
        while (it != loadedChunks.end()) {
            if (requiredChunks.find(*it) == requiredChunks.end()) {
                UnloadChunk(*it);
                it = loadedChunks.erase(it);
            } else {
                ++it;
            }
        }
    }
};
```

---

### 场景 3：射线投射（方块选择）

```cpp
struct RaycastHit {
    bool hit;
    WorldCoordinate blockPos;
    WorldCoordinate adjacentPos;  // 相邻的空气位置（用于放置方块）
    glm::vec3 hitPoint;
    Face hitFace;
};

RaycastHit RaycastBlocks(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    RaycastHit result;
    result.hit = false;
    
    glm::vec3 pos = origin;
    glm::vec3 step = glm::normalize(direction) * 0.1f;  // 步长
    
    for (float distance = 0; distance < maxDistance; distance += 0.1f) {
        // 转换为世界坐标（整数）
        int blockX = (int)floor(pos.x);
        int blockY = (int)floor(pos.y);
        int blockZ = (int)floor(pos.z);
        
        // 检查该位置是否有方块
        BlockType block = world->GetBlockAt(blockX, blockY, blockZ);
        
        if (block != BlockType::Air) {
            result.hit = true;
            result.blockPos = WorldCoordinate{blockX, blockY, blockZ};
            result.hitPoint = pos;
            
            // 计算击中的面和相邻位置
            // ...（省略详细计算）
            
            return result;
        }
        
        pos += step;
    }
    
    return result;  // 未击中
}
```

---

## 📊 坐标系统的限制与解决方案

### 1. 浮点精度问题

**问题**：当世界坐标过大时，浮点数精度下降导致物体抖动。

```cpp
// 例如：世界坐标 (1000000, 64, 1000000)
glm::vec3 worldPos(1000000.0f, 64.0f, 1000000.0f);
// 在这个位置，float 精度约为 0.125，导致渲染抖动
```

**解决方案 1：使用相对坐标渲染**
```cpp
void RenderWorld(const Camera& camera) {
    glm::vec3 cameraChunkOrigin = GetChunkOrigin(camera.position);
    
    for (auto& chunk : visibleChunks) {
        // 使用相对于摄像机的坐标
        glm::vec3 relativePos = chunk->GetWorldPosition() - cameraChunkOrigin;
        RenderChunk(chunk, relativePos);
    }
}
```

**解决方案 2：使用 double 进行计算，仅渲染时转为 float**
```cpp
struct PreciseWorldPosition {
    double x, y, z;
    
    glm::vec3 ToRenderPosition(const PreciseWorldPosition& cameraPos) const {
        return glm::vec3(
            (float)(x - cameraPos.x),
            (float)(y - cameraPos.y),
            (float)(z - cameraPos.z)
        );
    }
};
```

---

### 2. 整数溢出

**问题**：使用 int32，最大值约 ±21亿，对应约 ±2100万米。

**解决方案**：
```cpp
// 使用 int64_t 扩展范围
struct WorldCoordinate64 {
    int64_t x, y, z;  // 范围：±922京（足够大）
};

// 或设置世界边界
const int32_t WORLD_BORDER = 30000000;  // ±3000万格（Minecraft 的边界）

bool IsWithinWorldBorder(int x, int z) {
    return abs(x) < WORLD_BORDER && abs(z) < WORLD_BORDER;
}
```

---

### 3. 负数坐标处理

**问题**：C++ 的整数除法和取模对负数的行为可能不符合预期。

```cpp
// 错误示例
int chunkX = worldX / 16;  // -17 / 16 = -1 ✗ 期望 -2
int localX = worldX % 16;  // -17 % 16 = -1 ✗ 期望 15

// 正确处理
int WorldToChunkCoord(int worldCoord) {
    return worldCoord >= 0 
        ? worldCoord / CHUNK_SIZE 
        : (worldCoord - CHUNK_SIZE + 1) / CHUNK_SIZE;
}

int WorldToLocalCoord(int worldCoord) {
    int local = worldCoord % CHUNK_SIZE;
    return local < 0 ? local + CHUNK_SIZE : local;
}
```

---

## 🛠️ 工具函数库

### 完整的坐标工具类

```cpp
class CoordinateUtils {
public:
    // 距离计算
    static float Distance2D(const WorldCoordinate& a, const WorldCoordinate& b) {
        float dx = (float)(a.x - b.x);
        float dz = (float)(a.z - b.z);
        return sqrtf(dx * dx + dz * dz);
    }
    
    static float Distance3D(const WorldCoordinate& a, const WorldCoordinate& b) {
        float dx = (float)(a.x - b.x);
        float dy = (float)(a.y - b.y);
        float dz = (float)(a.z - b.z);
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
    
    // 曼哈顿距离（更快）
    static int ManhattanDistance(const WorldCoordinate& a, const WorldCoordinate& b) {
        return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
    }
    
    // 方向向量
    static glm::vec3 GetDirection(const WorldCoordinate& from, const WorldCoordinate& to) {
        return glm::normalize(glm::vec3(
            to.x - from.x,
            to.y - from.y,
            to.z - from.z
        ));
    }
    
    // 获取相邻位置
    static WorldCoordinate GetAdjacentBlock(const WorldCoordinate& pos, Face face) {
        WorldCoordinate adjacent = pos;
        switch (face) {
            case Face::Right:  adjacent.x++; break;
            case Face::Left:   adjacent.x--; break;
            case Face::Top:    adjacent.y++; break;
            case Face::Bottom: adjacent.y--; break;
            case Face::Front:  adjacent.z++; break;
            case Face::Back:   adjacent.z--; break;
        }
        return adjacent;
    }
    
    // 区块中心位置
    static glm::vec3 GetChunkCenter(const ChunkCoordinate& chunk) {
        return glm::vec3(
            chunk.x * CHUNK_SIZE + CHUNK_SIZE / 2.0f,
            WORLD_HEIGHT / 2.0f,
            chunk.z * CHUNK_SIZE + CHUNK_SIZE / 2.0f
        );
    }
};
```

---

## 🎯 性能优化技巧

### 1. 位运算优化（当 CHUNK_SIZE 是 2 的幂）

```cpp
// 假设 CHUNK_SIZE = 16 (2^4)
constexpr int CHUNK_SIZE_BITS = 4;  // log2(16) = 4

// 除法 → 右移
inline int FastWorldToChunkCoord(int worldCoord) {
    return worldCoord >> CHUNK_SIZE_BITS;  // 仅适用于非负数
}

// 取模 → 位掩码
inline int FastWorldToLocalCoord(int worldCoord) {
    return worldCoord & (CHUNK_SIZE - 1);  // 仅适用于非负数
}

// 注意：负数需要特殊处理
inline int FastWorldToChunkCoordSigned(int worldCoord) {
    return worldCoord >= 0 
        ? worldCoord >> CHUNK_SIZE_BITS
        : -(((-worldCoord - 1) >> CHUNK_SIZE_BITS) + 1);
}
```

### 2. 缓存坐标转换结果

```cpp
class CoordinateCache {
private:
    struct CachedPosition {
        ChunkCoordinate chunk;
        LocalCoordinate local;
    };
    
    std::unordered_map<WorldCoordinate, CachedPosition> cache;
    
public:
    const CachedPosition& Get(const WorldCoordinate& world) {
        auto it = cache.find(world);
        if (it != cache.end()) {
            return it->second;
        }
        
        // 计算并缓存
        CachedPosition pos;
        CoordinateConverter::WorldToChunkAndLocal(
            world.x, world.y, world.z,
            pos.chunk, pos.local
        );
        cache[world] = pos;
        return cache[world];
    }
};
```

---

## 📚 总结

### 关键要点
1. **三种坐标系统**：世界坐标、区块坐标、局部坐标
2. **正确的转换**：特别注意负数处理
3. **性能优化**：使用位运算、缓存、相对坐标
4. **精度控制**：大坐标时使用相对渲染

### 最佳实践
- 始终使用封装的转换函数，避免直接计算
- 对负数坐标进行充分测试
- 在渲染时使用相对坐标避免精度问题
- 根据需要选择合适的数据类型（int32/int64）

---

掌握坐标系统是开发沙盒游戏的基础！祝你开发顺利！
