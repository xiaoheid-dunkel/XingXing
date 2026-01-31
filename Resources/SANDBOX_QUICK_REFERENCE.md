# XingXing Sandbox Game Engine - Quick Reference

## What is XingXing?

XingXing is now a dedicated game engine for Minecraft-like sandbox games. It provides a complete voxel/block-based world system with terrain generation, player interaction, and inventory management.

## Quick Start

### 1. Block Types

```cpp
BlockType::Air       // Invisible, transparent
BlockType::Grass     // Surface block
BlockType::Dirt      // Underground block
BlockType::Stone     // Main underground block
BlockType::Wood      // Building material
BlockType::Water     // Liquid block
BlockType::Coal      // Resource ore
BlockType::Iron      // Resource ore
BlockType::Gold      // Resource ore
BlockType::Diamond   // Rare resource ore
```

### 2. Create a World

```cpp
// Create world entity
auto world = scene->CreateEntity("World");
auto& worldComp = world.AddComponent<WorldComponent>();
worldComp.Seed = 12345;
worldComp.RenderDistance = 8;

// Add terrain generator
auto& terrainGen = world.AddComponent<TerrainGeneratorComponent>();
terrainGen.Scale = 0.05f;
terrainGen.HeightMultiplier = 32.0f;
terrainGen.BaseHeight = 64;

// Generate chunks
WorldGenerator generator(worldComp.Seed);
auto chunk = scene->CreateEntity("Chunk");
auto& chunkComp = chunk.AddComponent<ChunkComponent>(glm::ivec3(0, 0, 0));
generator.GenerateChunk(chunkComp, terrainGen);
```

### 3. Player Inventory

```cpp
auto player = scene->CreateEntity("Player");
auto& inventory = player.AddComponent<PlayerInventoryComponent>();

// Add items
inventory.AddItem(BlockType::Stone, 64);
inventory.AddItem(BlockType::Wood, 32);

// Remove items
inventory.RemoveItem(BlockType::Stone, 10);

// Select hotbar slot
inventory.SelectedSlot = 0;  // First slot
```

### 4. Block Interaction

```cpp
auto& interaction = player.AddComponent<BlockInteractionComponent>();
interaction.ReachDistance = 5.0f;
interaction.IsBreaking = true;
interaction.TargetBlockPos = glm::ivec3(10, 64, 10);
interaction.BreakProgress = 0.5f;  // 50% broken
```

## Component Overview

| Component | Purpose |
|-----------|---------|
| `VoxelComponent` | Single block/voxel entity |
| `ChunkComponent` | 16x256x16 block region |
| `WorldComponent` | World and chunk management |
| `TerrainGeneratorComponent` | Terrain generation parameters |
| `PlayerInventoryComponent` | 36-slot inventory system |
| `BlockInteractionComponent` | Block placement/breaking |

## Architecture

```
World
  └─ Chunks (16x256x16)
       └─ Blocks
            └─ BlockType (Air, Grass, Stone, etc.)

Player
  ├─ Inventory (36 slots)
  └─ Block Interaction
```

## Key Features

✅ **Voxel/Block System** - Complete block type and property management  
✅ **Chunk Management** - Efficient 16x256x16 chunk loading/unloading  
✅ **Terrain Generation** - Procedural terrain using Simplex noise  
✅ **Player Interaction** - Block placement, breaking, and interaction  
✅ **Inventory System** - Full 36-slot inventory management  
✅ **World Management** - Multi-chunk worlds with seed generation  

## Performance Tips

1. **Only load chunks near the player** - Use `RenderDistance`
2. **Cull invisible faces** - Don't render block faces adjacent to solid blocks
3. **Batch rendering** - Use `Renderer2D::BeginScene()/EndScene()`
4. **Chunk caching** - Keep inactive chunks in memory pool

## See Also

- [Detailed Guide](./SANDBOX_GUIDE.md) (Chinese)
- [Main README](../README.md)
- [Example Project](../Sandbox/)
