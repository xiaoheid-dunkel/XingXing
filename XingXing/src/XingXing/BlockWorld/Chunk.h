#pragma once

#include "Block.h"
#include "XingXing/Core/Core.h"
#include <glm/glm.hpp>
#include <array>

namespace Hazel {

	constexpr int CHUNK_SIZE = 16;  // 每个区块16x16方块

	/**
	 * @brief 区块类
	 * 表示游戏世界中的一个方块区域
	 * 用于优化渲染和内存管理
	 */
	class Chunk
	{
	public:
		Chunk(int chunkX, int chunkY);
		~Chunk() = default;

		/**
		 * @brief 获取区块内指定位置的方块
		 * @param x 区块内X坐标 (0-15)
		 * @param y 区块内Y坐标 (0-15)
		 * @return 方块ID
		 */
		BlockID GetBlock(int x, int y) const;

		/**
		 * @brief 设置区块内指定位置的方块
		 * @param x 区块内X坐标 (0-15)
		 * @param y 区块内Y坐标 (0-15)
		 * @param blockID 要设置的方块ID
		 */
		void SetBlock(int x, int y, BlockID blockID);

		/**
		 * @brief 获取区块坐标
		 * @return 区块在世界中的坐标
		 */
		glm::ivec2 GetChunkCoord() const { return { m_ChunkX, m_ChunkY }; }

		/**
		 * @brief 检查区块是否需要重新渲染
		 * @return 如果区块被修改返回true
		 */
		bool IsDirty() const { return m_IsDirty; }

		/**
		 * @brief 标记区块为脏（需要重新渲染）
		 */
		void MarkDirty() { m_IsDirty = true; }

		/**
		 * @brief 清除脏标记
		 */
		void ClearDirty() { m_IsDirty = false; }

		/**
		 * @brief 检查坐标是否在区块范围内
		 * @param x 区块内X坐标
		 * @param y 区块内Y坐标
		 * @return 如果坐标有效返回true
		 */
		bool IsValidLocalCoord(int x, int y) const;

		/**
		 * @brief 检查区块是否为空（全部为空气）
		 * @return 如果全部为空气返回true
		 */
		bool IsEmpty() const;

		/**
		 * @brief 获取区块的世界起始位置（左下角）
		 * @return 世界坐标
		 */
		glm::vec2 GetWorldPosition() const
		{
			return glm::vec2(m_ChunkX * CHUNK_SIZE, m_ChunkY * CHUNK_SIZE);
		}

	private:
		int m_ChunkX, m_ChunkY;  // 区块坐标
		std::array<std::array<BlockID, CHUNK_SIZE>, CHUNK_SIZE> m_Blocks;
		bool m_IsDirty;  // 脏标记
	};

} // namespace Hazel
