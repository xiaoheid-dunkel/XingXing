#pragma once

#include "XingXing/Core/Base.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace Hazel {

	// Block type enumeration for different types of blocks in the sandbox world
	enum class BlockType : uint8_t
	{
		Air = 0,      // Empty space
		Grass,        // Grass block
		Dirt,         // Dirt block
		Stone,        // Stone block
		Wood,         // Wood block
		Leaves,       // Leaves block
		Sand,         // Sand block
		Water,        // Water block
		Bedrock,      // Unbreakable bedrock
		Coal,         // Coal ore
		Iron,         // Iron ore
		Gold,         // Gold ore
		Diamond       // Diamond ore
	};

	// Represents a single block in the voxel world
	struct Block
	{
		BlockType Type = BlockType::Air;
		bool IsActive = true;
		
		Block() = default;
		Block(BlockType type) : Type(type) {}
		
		bool IsTransparent() const
		{
			return Type == BlockType::Air || Type == BlockType::Water || Type == BlockType::Leaves;
		}
		
		bool IsSolid() const
		{
			return Type != BlockType::Air && Type != BlockType::Water;
		}
	};

	// Component for entities that represent individual blocks/voxels
	struct VoxelComponent
	{
		Block BlockData;
		glm::ivec3 GridPosition = { 0, 0, 0 };  // Position in the voxel grid
		bool IsDirty = true;  // Needs mesh rebuild
		
		VoxelComponent() = default;
		VoxelComponent(const VoxelComponent&) = default;
		VoxelComponent(BlockType type, const glm::ivec3& pos)
			: BlockData(type), GridPosition(pos) {}
	};

	// Chunk dimensions
	constexpr int CHUNK_SIZE_X = 16;
	constexpr int CHUNK_SIZE_Y = 256;  // Height limit similar to Minecraft
	constexpr int CHUNK_SIZE_Z = 16;

	// Component for chunk management (a chunk is a collection of blocks)
	struct ChunkComponent
	{
		glm::ivec3 ChunkPosition = { 0, 0, 0 };  // Position in chunk coordinates
		Block Blocks[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];  // 3D array of blocks
		bool IsLoaded = false;
		bool NeedsMeshRebuild = true;
		
		ChunkComponent() = default;
		ChunkComponent(const ChunkComponent&) = default;
		ChunkComponent(const glm::ivec3& pos) : ChunkPosition(pos)
		{
			// Initialize all blocks as air
			for (int x = 0; x < CHUNK_SIZE_X; x++)
				for (int y = 0; y < CHUNK_SIZE_Y; y++)
					for (int z = 0; z < CHUNK_SIZE_Z; z++)
						Blocks[x][y][z] = Block(BlockType::Air);
		}
		
		Block& GetBlock(int x, int y, int z)
		{
			HZ_CORE_ASSERT(x >= 0 && x < CHUNK_SIZE_X, "X out of bounds");
			HZ_CORE_ASSERT(y >= 0 && y < CHUNK_SIZE_Y, "Y out of bounds");
			HZ_CORE_ASSERT(z >= 0 && z < CHUNK_SIZE_Z, "Z out of bounds");
			return Blocks[x][y][z];
		}
		
		const Block& GetBlock(int x, int y, int z) const
		{
			HZ_CORE_ASSERT(x >= 0 && x < CHUNK_SIZE_X, "X out of bounds");
			HZ_CORE_ASSERT(y >= 0 && y < CHUNK_SIZE_Y, "Y out of bounds");
			HZ_CORE_ASSERT(z >= 0 && z < CHUNK_SIZE_Z, "Z out of bounds");
			return Blocks[x][y][z];
		}
		
		void SetBlock(int x, int y, int z, const Block& block)
		{
			if (x >= 0 && x < CHUNK_SIZE_X && 
			    y >= 0 && y < CHUNK_SIZE_Y && 
			    z >= 0 && z < CHUNK_SIZE_Z)
			{
				Blocks[x][y][z] = block;
				NeedsMeshRebuild = true;
			}
		}
	};

	// Component for managing the entire world (collection of chunks)
	struct WorldComponent
	{
		std::unordered_map<int64_t, UUID> LoadedChunks;  // Map chunk position hash to entity UUID
		int RenderDistance = 8;  // Number of chunks to render around player
		int Seed = 12345;  // World generation seed
		
		WorldComponent() = default;
		WorldComponent(const WorldComponent&) = default;
		
		// Hash function for chunk position
		static int64_t HashChunkPos(const glm::ivec3& pos)
		{
			return ((int64_t)pos.x << 32) | ((int64_t)pos.y << 16) | (int64_t)pos.z;
		}
		
		bool IsChunkLoaded(const glm::ivec3& chunkPos) const
		{
			return LoadedChunks.find(HashChunkPos(chunkPos)) != LoadedChunks.end();
		}
	};

	// Component for player inventory system
	struct PlayerInventoryComponent
	{
		struct ItemStack
		{
			BlockType Type = BlockType::Air;
			int Count = 0;
			
			ItemStack() = default;
			ItemStack(BlockType type, int count) : Type(type), Count(count) {}
		};
		
		static constexpr int INVENTORY_SIZE = 36;  // 9 hotbar + 27 main inventory
		ItemStack Items[INVENTORY_SIZE];
		int SelectedSlot = 0;  // Currently selected hotbar slot (0-8)
		
		PlayerInventoryComponent() = default;
		PlayerInventoryComponent(const PlayerInventoryComponent&) = default;
		
		bool AddItem(BlockType type, int count = 1)
		{
			// Try to stack with existing items
			for (int i = 0; i < INVENTORY_SIZE; i++)
			{
				if (Items[i].Type == type && Items[i].Count < 64)
				{
					int space = 64 - Items[i].Count;
					int toAdd = std::min(space, count);
					Items[i].Count += toAdd;
					count -= toAdd;
					if (count <= 0) return true;
				}
			}
			
			// Find empty slot
			for (int i = 0; i < INVENTORY_SIZE; i++)
			{
				if (Items[i].Type == BlockType::Air || Items[i].Count == 0)
				{
					Items[i].Type = type;
					Items[i].Count = std::min(count, 64);
					count -= Items[i].Count;
					if (count <= 0) return true;
				}
			}
			
			return count == 0;  // Returns true if all items were added
		}
		
		bool RemoveItem(BlockType type, int count = 1)
		{
			for (int i = 0; i < INVENTORY_SIZE; i++)
			{
				if (Items[i].Type == type && Items[i].Count > 0)
				{
					int toRemove = std::min(Items[i].Count, count);
					Items[i].Count -= toRemove;
					count -= toRemove;
					
					if (Items[i].Count == 0)
						Items[i].Type = BlockType::Air;
					
					if (count <= 0) return true;
				}
			}
			return count == 0;
		}
		
		ItemStack& GetSelectedItem()
		{
			return Items[SelectedSlot];
		}
	};

	// Component for block interaction (placing and breaking blocks)
	struct BlockInteractionComponent
	{
		float BreakProgress = 0.0f;  // 0.0 to 1.0
		glm::ivec3 TargetBlockPos = { 0, 0, 0 };
		bool IsBreaking = false;
		float ReachDistance = 5.0f;  // Maximum distance to interact with blocks
		
		BlockInteractionComponent() = default;
		BlockInteractionComponent(const BlockInteractionComponent&) = default;
	};

	// Component for terrain generation parameters
	struct TerrainGeneratorComponent
	{
		// Perlin noise parameters
		float Scale = 0.05f;
		float HeightMultiplier = 32.0f;
		int BaseHeight = 64;
		int WaterLevel = 62;
		
		// Biome parameters
		float TemperatureScale = 0.02f;
		float MoistureScale = 0.02f;
		
		TerrainGeneratorComponent() = default;
		TerrainGeneratorComponent(const TerrainGeneratorComponent&) = default;
	};

}
