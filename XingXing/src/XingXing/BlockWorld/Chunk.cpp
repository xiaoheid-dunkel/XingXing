#include "hzpch.h"
#include "Chunk.h"

namespace Hazel {

	Chunk::Chunk(int chunkX, int chunkY)
		: m_ChunkX(chunkX), m_ChunkY(chunkY), m_IsDirty(true)
	{
		// 初始化所有方块为空气
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				m_Blocks[x][y] = BLOCK_AIR;
			}
		}
	}

	BlockID Chunk::GetBlock(int x, int y) const
	{
		if (!IsValidLocalCoord(x, y))
			return BLOCK_AIR;
		
		return m_Blocks[x][y];
	}

	void Chunk::SetBlock(int x, int y, BlockID blockID)
	{
		if (!IsValidLocalCoord(x, y))
			return;

		if (m_Blocks[x][y] != blockID)
		{
			m_Blocks[x][y] = blockID;
			m_IsDirty = true;
		}
	}

	bool Chunk::IsValidLocalCoord(int x, int y) const
	{
		return x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE;
	}

	bool Chunk::IsEmpty() const
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int x = 0; x < CHUNK_SIZE; x++)
			{
				if (m_Blocks[x][y] != BLOCK_AIR)
					return false;
			}
		}
		return true;
	}

} // namespace Hazel
