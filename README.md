# 🌟 XingXing (行星引擎) - 沙盒游戏专属引擎

XingXing 是一个专为**沙盒类游戏**（类似《我的世界》）开发的**独立引擎**。本引擎专注于提供体素/方块世界、地形生成、玩家交互等沙盒游戏核心功能，为独立游戏开发和模组 (Mod) 开发者提供一个透明、易用的框架。

---

## 🛠 引擎溯源
本引擎的最初底层代码源自于 [Hazel2D](https://github.com/TheCherno/Hazel)（由 TheCherno 开发）。在 Hazel2D 优秀的架构基础上，我们针对特定的游戏需求进行了深度的定制和功能扩展。

## 📜 开源目的与许可协议

### 1. 为什么开源？
我们选择开放源码，主要是为了**未来的模组 (Mod) 开发者**。通过阅读源码，开发者可以更好地理解游戏的底层运行逻辑，从而创作出更丰富、更具创意的游戏模组。

### 2. 使用限制 (重要)
虽然源码是开放的，但为了保护原创内容，本引擎遵循以下限制：
* **禁止商用**：严禁将本引擎的代码或其派生版本用于任何形式的商业盈利行为。
* **严禁严重复制**：本项目仅供学习、研究和模组开发使用。严禁直接“换壳”包装成自己的引擎或产品发布。
* **尊重原创**：如果你在非商用的个人学习项目中引用了部分代码，请务必保留对原始项目 (Hazel2D) 和本项目的致谢。

## 🚀 核心功能

### 沙盒游戏特性
* **体素/方块系统** - 完整的方块类型和属性管理
* **区块管理系统** - 高效的 16x256x16 区块加载和卸载
* **地形生成** - 基于 Simplex 噪声的程序化地形生成
* **玩家交互** - 方块放置、破坏和交互系统
* **物品栏系统** - 完整的 36 格物品栏管理
* **世界管理** - 支持多区块世界和种子生成

### 渲染引擎
* **高性能 2D 批处理渲染器** (基于 Renderer2D)
* **MSDF 字体渲染系统**
* **纹理与精灵图管理**
* **正交摄像机系统**

---

## 🏗 如何构建

1.  克隆本仓库：
    ```bash
    git clone [https://github.com/xiaoheid-dunkel/XingXing.git](https://github.com/xiaoheid-dunkel/XingXing.git)
    ```
2.  运行根目录下的 `GenerateProjectFiles.bat` 生成 Visual Studio 解决方案。
3.  使用 Visual Studio 2022 或更高版本打开 `XingXing.sln` 并进行编译。

---

---

## 🎮 沙盒游戏组件系统

### 核心组件

#### 1. VoxelComponent (体素组件)
用于表示单个方块/体素实体：
```cpp
struct VoxelComponent {
    Block BlockData;              // 方块数据
    glm::ivec3 GridPosition;      // 网格位置
    bool IsDirty;                 // 需要重建网格
};
```

#### 2. ChunkComponent (区块组件)
管理 16x256x16 的方块区块：
```cpp
struct ChunkComponent {
    glm::ivec3 ChunkPosition;     // 区块坐标
    Block Blocks[16][256][16];    // 方块数据
    bool IsLoaded;                // 是否已加载
    bool NeedsMeshRebuild;        // 需要重建网格
};
```

#### 3. WorldComponent (世界组件)
管理整个世界和区块加载：
```cpp
struct WorldComponent {
    std::unordered_map<int64_t, UUID> LoadedChunks;  // 已加载区块
    int RenderDistance;                               // 渲染距离
    int Seed;                                         // 世界种子
};
```

#### 4. PlayerInventoryComponent (玩家物品栏组件)
管理玩家的 36 格物品栏系统：
```cpp
struct PlayerInventoryComponent {
    ItemStack Items[36];          // 9 热键栏 + 27 主物品栏
    int SelectedSlot;             // 当前选中的格子
    
    bool AddItem(BlockType type, int count);
    bool RemoveItem(BlockType type, int count);
};
```

#### 5. BlockInteractionComponent (方块交互组件)
处理方块的放置和破坏：
```cpp
struct BlockInteractionComponent {
    float BreakProgress;          // 破坏进度 (0.0 - 1.0)
    glm::ivec3 TargetBlockPos;    // 目标方块位置
    bool IsBreaking;              // 是否正在破坏
    float ReachDistance;          // 交互距离
};
```

#### 6. TerrainGeneratorComponent (地形生成器组件)
控制程序化地形生成参数：
```cpp
struct TerrainGeneratorComponent {
    float Scale;                  // 噪声缩放
    float HeightMultiplier;       // 高度倍数
    int BaseHeight;               // 基础高度
    int WaterLevel;               // 水位线
};
```

### 方块类型

引擎支持以下方块类型：
- **Air** (空气) - 透明、不可见
- **Grass** (草方块) - 地表方块
- **Dirt** (泥土) - 地表下方块
- **Stone** (石头) - 地下主要方块
- **Wood** (木头) - 建筑材料
- **Leaves** (树叶) - 半透明植物
- **Sand** (沙子) - 沙滩/沙漠方块
- **Water** (水) - 液体方块
- **Bedrock** (基岩) - 不可破坏方块
- **Coal** (煤矿) - 资源方块
- **Iron** (铁矿) - 资源方块
- **Gold** (金矿) - 资源方块
- **Diamond** (钻石矿) - 珍稀资源方块

### 地形生成系统

使用 **SimplexNoise** 和 **WorldGenerator** 类实现程序化地形生成：
- 多层噪声叠加产生自然地形
- 支持矿物生成（煤、铁、金、钻石）
- 可配置的世界种子
- 自动生成草地、泥土、石头层级

---

## 📖 使用示例

### 创建沙盒世界

```cpp
// 创建世界实体
auto world = scene->CreateEntity("World");
world.AddComponent<WorldComponent>();
world.AddComponent<TerrainGeneratorComponent>();

// 创建区块
auto chunk = scene->CreateEntity("Chunk");
auto& chunkComp = chunk.AddComponent<ChunkComponent>(glm::ivec3(0, 0, 0));

// 生成地形
WorldGenerator generator(12345);
generator.GenerateChunk(chunkComp, world.GetComponent<TerrainGeneratorComponent>());
```

### 添加玩家交互

```cpp
// 创建玩家实体
auto player = scene->CreateEntity("Player");
player.AddComponent<PlayerInventoryComponent>();
player.AddComponent<BlockInteractionComponent>();

// 添加方块到物品栏
auto& inventory = player.GetComponent<PlayerInventoryComponent>();
inventory.AddItem(BlockType::Stone, 64);
inventory.AddItem(BlockType::Wood, 32);
```

