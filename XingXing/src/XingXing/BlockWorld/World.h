#pragma once

#include "Chunk.h"
#include "XingXing/Core/Core.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>

namespace Hazel {

	/**
	 * @brief 区块坐标哈希函数
	 * 用于在unordered_map中存储区块
	 */
	struct ChunkCoordHash
	{
		size_t operator()(const glm::ivec2& coord) const
		{
			// Cantor pairing function
			size_t a = coord.x >= 0 ? 2 * coord.x : -2 * coord.x - 1;
			size_t b = coord.y >= 0 ? 2 * coord.y : -2 * coord.y - 1;
			return (a + b) * (a + b + 1) / 2 + b;
		}
	};

	/**
	 * @brief 世界类
	 * 管理游戏世界中的所有区块和方块
	 */
	class World
	{
	public:
		World();
		~World();

		/**
		 * @brief 获取世界坐标处的方块
		 * @param worldX 世界X坐标
		 * @param worldY 世界Y坐标
		 * @return 方块ID
		 */
		BlockID GetBlock(int worldX, int worldY) const;

		/**
		 * @brief 设置世界坐标处的方块
		 * @param worldX 世界X坐标
		 * @param worldY 世界Y坐标
		 * @param blockID 要设置的方块ID
		 */
		void SetBlock(int worldX, int worldY, BlockID blockID);

		/**
		 * @brief 获取指定区块
		 * @param chunkX 区块X坐标
		 * @param chunkY 区块Y坐标
		 * @return 区块指针，如果不存在返回nullptr
		 */
		Chunk* GetChunk(int chunkX, int chunkY);
		const Chunk* GetChunk(int chunkX, int chunkY) const;

		/**
		 * @brief 获取或创建区块
		 * @param chunkX 区块X坐标
		 * @param chunkY 区块Y坐标
		 * @return 区块指针
		 */
		Chunk* GetOrCreateChunk(int chunkX, int chunkY);

		/**
		 * @brief 卸载指定区块
		 * @param chunkX 区块X坐标
		 * @param chunkY 区块Y坐标
		 */
		void UnloadChunk(int chunkX, int chunkY);

		/**
		 * @brief 更新世界（加载/卸载区块）
		 * @param playerPosition 玩家位置，用于动态加载区块
		 */
		void Update(const glm::vec2& playerPosition);

		/**
		 * @brief 渲染世界
		 * @param viewProjection 视图投影矩阵
		 */
		void Render(const glm::mat4& viewProjection);

		/**
		 * @brief 设置区块加载半径
		 * @param radius 加载半径（以区块为单位）
		 */
		void SetLoadRadius(int radius) { m_LoadRadius = radius; }

		/**
		 * @brief 获取区块加载半径
		 * @return 加载半径
		 */
		int GetLoadRadius() const { return m_LoadRadius; }

		/**
		 * @brief 获取已加载的区块数量
		 * @return 区块数量
		 */
		size_t GetLoadedChunkCount() const { return m_Chunks.size(); }

		/**
		 * @brief 世界坐标转换为区块坐标
		 * @param worldX 世界X坐标
		 * @param worldY 世界Y坐标
		 * @return 区块坐标
		 */
		static glm::ivec2 WorldToChunk(int worldX, int worldY);

		/**
		 * @brief 世界坐标转换为区块内坐标
		 * @param worldX 世界X坐标
		 * @param worldY 世界Y坐标
		 * @return 区块内坐标 (0-15)
		 */
		static glm::ivec2 WorldToLocal(int worldX, int worldY);

		/**
		 * @brief 区块坐标和区块内坐标转换为世界坐标
		 * @param chunkCoord 区块坐标
		 * @param localCoord 区块内坐标
		 * @return 世界坐标
		 */
		static glm::ivec2 ChunkAndLocalToWorld(const glm::ivec2& chunkCoord, const glm::ivec2& localCoord);

	private:
		std::unordered_map<glm::ivec2, Ref<Chunk>, ChunkCoordHash> m_Chunks;
		int m_LoadRadius = 3;  // 默认加载半径

		void LoadChunksAroundPlayer(const glm::vec2& playerPosition);
		void UnloadDistantChunks(const glm::vec2& playerPosition);
		void GenerateChunk(Chunk* chunk);
	};

} // namespace Hazel
