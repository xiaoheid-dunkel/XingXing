#include "hzpch.h"
#include "World.h"
#include "BlockRegistry.h"
#include "XingXing/Renderer/Renderer2D.h"
#include "XingXing/Core/Log.h"
#include <cmath>

namespace Hazel {

	World::World()
	{
		HZ_CORE_INFO("Creating new world...");
	}

	World::~World()
	{
		HZ_CORE_INFO("Destroying world with {0} chunks", m_Chunks.size());
		m_Chunks.clear();
	}

	BlockID World::GetBlock(int worldX, int worldY) const
	{
		glm::ivec2 chunkCoord = WorldToChunk(worldX, worldY);
		const Chunk* chunk = GetChunk(chunkCoord.x, chunkCoord.y);
		
		if (!chunk)
			return BLOCK_AIR;

		glm::ivec2 localCoord = WorldToLocal(worldX, worldY);
		return chunk->GetBlock(localCoord.x, localCoord.y);
	}

	void World::SetBlock(int worldX, int worldY, BlockID blockID)
	{
		glm::ivec2 chunkCoord = WorldToChunk(worldX, worldY);
		Chunk* chunk = GetOrCreateChunk(chunkCoord.x, chunkCoord.y);

		glm::ivec2 localCoord = WorldToLocal(worldX, worldY);
		chunk->SetBlock(localCoord.x, localCoord.y, blockID);
	}

	Chunk* World::GetChunk(int chunkX, int chunkY)
	{
		glm::ivec2 coord(chunkX, chunkY);
		auto it = m_Chunks.find(coord);
		if (it != m_Chunks.end())
			return it->second.get();
		return nullptr;
	}

	const Chunk* World::GetChunk(int chunkX, int chunkY) const
	{
		glm::ivec2 coord(chunkX, chunkY);
		auto it = m_Chunks.find(coord);
		if (it != m_Chunks.end())
			return it->second.get();
		return nullptr;
	}

	Chunk* World::GetOrCreateChunk(int chunkX, int chunkY)
	{
		Chunk* chunk = GetChunk(chunkX, chunkY);
		if (chunk)
			return chunk;

		// 创建新区块
		glm::ivec2 coord(chunkX, chunkY);
		m_Chunks[coord] = CreateRef<Chunk>(chunkX, chunkY);
		chunk = m_Chunks[coord].get();

		// 生成地形
		GenerateChunk(chunk);

		HZ_CORE_TRACE("Created chunk at ({0}, {1})", chunkX, chunkY);
		return chunk;
	}

	void World::UnloadChunk(int chunkX, int chunkY)
	{
		glm::ivec2 coord(chunkX, chunkY);
		auto it = m_Chunks.find(coord);
		if (it != m_Chunks.end())
		{
			HZ_CORE_TRACE("Unloading chunk at ({0}, {1})", chunkX, chunkY);
			m_Chunks.erase(it);
		}
	}

	void World::Update(const glm::vec2& playerPosition)
	{
		// 加载玩家周围的区块
		LoadChunksAroundPlayer(playerPosition);

		// 卸载远离玩家的区块
		UnloadDistantChunks(playerPosition);
	}

	void World::Render(const glm::mat4& viewProjection)
	{
		// 遍历所有区块
		for (auto& [coord, chunk] : m_Chunks)
		{
			// 渲染区块中的每个方块
			for (int localY = 0; localY < CHUNK_SIZE; localY++)
			{
				for (int localX = 0; localX < CHUNK_SIZE; localX++)
				{
					BlockID blockID = chunk->GetBlock(localX, localY);
					if (blockID == BLOCK_AIR)
						continue;

					const Block* block = BlockRegistry::GetBlock(blockID);
					if (!block)
						continue;

					// 计算世界位置
					float worldX = coord.x * CHUNK_SIZE + localX;
					float worldY = coord.y * CHUNK_SIZE + localY;

					// 使用Renderer2D绘制方块
					const auto& props = block->GetProperties();
					if (props.Texture)
					{
						Renderer2D::DrawQuad(
							{ worldX, worldY, 0.0f },
							{ 1.0f, 1.0f },
							props.Texture,
							1.0f,
							props.Color
						);
					}
					else
					{
						Renderer2D::DrawQuad(
							{ worldX, worldY, 0.0f },
							{ 1.0f, 1.0f },
							props.Color
						);
					}
				}
			}

			// 清除脏标记
			chunk->ClearDirty();
		}
	}

	glm::ivec2 World::WorldToChunk(int worldX, int worldY)
	{
		// 处理负数坐标的向下取整
		int chunkX = worldX < 0 ? (worldX + 1) / CHUNK_SIZE - 1 : worldX / CHUNK_SIZE;
		int chunkY = worldY < 0 ? (worldY + 1) / CHUNK_SIZE - 1 : worldY / CHUNK_SIZE;
		return { chunkX, chunkY };
	}

	glm::ivec2 World::WorldToLocal(int worldX, int worldY)
	{
		// 计算区块内坐标
		int localX = worldX % CHUNK_SIZE;
		int localY = worldY % CHUNK_SIZE;

		// 处理负数取模
		if (localX < 0) localX += CHUNK_SIZE;
		if (localY < 0) localY += CHUNK_SIZE;

		return { localX, localY };
	}

	glm::ivec2 World::ChunkAndLocalToWorld(const glm::ivec2& chunkCoord, const glm::ivec2& localCoord)
	{
		return {
			chunkCoord.x * CHUNK_SIZE + localCoord.x,
			chunkCoord.y * CHUNK_SIZE + localCoord.y
		};
	}

	void World::LoadChunksAroundPlayer(const glm::vec2& playerPosition)
	{
		// 获取玩家所在区块
		glm::ivec2 playerChunk = WorldToChunk(
			static_cast<int>(std::floor(playerPosition.x)),
			static_cast<int>(std::floor(playerPosition.y))
		);

		// 在玩家周围加载区块
		for (int dy = -m_LoadRadius; dy <= m_LoadRadius; dy++)
		{
			for (int dx = -m_LoadRadius; dx <= m_LoadRadius; dx++)
			{
				int chunkX = playerChunk.x + dx;
				int chunkY = playerChunk.y + dy;

				// 如果区块不存在，创建它
				GetOrCreateChunk(chunkX, chunkY);
			}
		}
	}

	void World::UnloadDistantChunks(const glm::vec2& playerPosition)
	{
		// 获取玩家所在区块
		glm::ivec2 playerChunk = WorldToChunk(
			static_cast<int>(std::floor(playerPosition.x)),
			static_cast<int>(std::floor(playerPosition.y))
		);

		// 收集需要卸载的区块
		std::vector<glm::ivec2> chunksToUnload;
		for (const auto& [coord, chunk] : m_Chunks)
		{
			int dx = coord.x - playerChunk.x;
			int dy = coord.y - playerChunk.y;
			int distance = std::max(std::abs(dx), std::abs(dy));

			// 如果区块距离玩家太远，标记为卸载
			if (distance > m_LoadRadius + 2)
			{
				chunksToUnload.push_back(coord);
			}
		}

		// 卸载标记的区块
		for (const auto& coord : chunksToUnload)
		{
			UnloadChunk(coord.x, coord.y);
		}
	}

	void World::GenerateChunk(Chunk* chunk)
	{
		glm::ivec2 chunkCoord = chunk->GetChunkCoord();

		// 简单的地形生成算法
		for (int localX = 0; localX < CHUNK_SIZE; localX++)
		{
			int worldX = chunkCoord.x * CHUNK_SIZE + localX;

			// 使用简单的正弦波生成地形高度
			float noise = std::sin(worldX * 0.1f) * 3.0f;
			int terrainHeight = static_cast<int>(10.0f + noise);

			for (int localY = 0; localY < CHUNK_SIZE; localY++)
			{
				int worldY = chunkCoord.y * CHUNK_SIZE + localY;

				if (worldY < terrainHeight - 3)
				{
					// 深层石头
					chunk->SetBlock(localX, localY, BLOCK_STONE);
				}
				else if (worldY < terrainHeight)
				{
					// 泥土层
					chunk->SetBlock(localX, localY, BLOCK_DIRT);
				}
				else if (worldY == terrainHeight)
				{
					// 草方块表层
					chunk->SetBlock(localX, localY, BLOCK_GRASS);
				}
				else
				{
					// 空气
					chunk->SetBlock(localX, localY, BLOCK_AIR);
				}
			}
		}
	}

} // namespace Hazel
