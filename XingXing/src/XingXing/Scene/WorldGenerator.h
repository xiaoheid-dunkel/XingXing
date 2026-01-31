#pragma once

#include "XingXing/Scene/Components.h"
#include "XingXing/Scene/SandboxComponents.h"
#include <glm/glm.hpp>
#include <cmath>
#include <random>

namespace Hazel {

	// Simple noise generation for terrain
	class SimplexNoise
	{
	public:
		SimplexNoise(int seed = 0) : m_Seed(seed)
		{
			// Initialize permutation table with seed
			m_Permutation.resize(256);
			for (int i = 0; i < 256; i++)
				m_Permutation[i] = i;
			
			// Shuffle based on seed
			std::mt19937 rng(seed);
			std::shuffle(m_Permutation.begin(), m_Permutation.end(), rng);
			
			// Duplicate for wrapping
			m_Permutation.insert(m_Permutation.end(), m_Permutation.begin(), m_Permutation.end());
		}
		
		float Noise2D(float x, float y)
		{
			// Simple hash-based noise
			int xi = (int)std::floor(x) & 255;
			int yi = (int)std::floor(y) & 255;
			
			float xf = x - std::floor(x);
			float yf = y - std::floor(y);
			
			// Interpolate
			float u = Fade(xf);
			float v = Fade(yf);
			
			int aa = m_Permutation[m_Permutation[xi] + yi];
			int ab = m_Permutation[m_Permutation[xi] + yi + 1];
			int ba = m_Permutation[m_Permutation[xi + 1] + yi];
			int bb = m_Permutation[m_Permutation[xi + 1] + yi + 1];
			
			float x1 = Lerp(Grad(aa, xf, yf), Grad(ba, xf - 1, yf), u);
			float x2 = Lerp(Grad(ab, xf, yf - 1), Grad(bb, xf - 1, yf - 1), u);
			
			return Lerp(x1, x2, v);
		}
		
	private:
		float Fade(float t)
		{
			return t * t * t * (t * (t * 6 - 15) + 10);
		}
		
		float Lerp(float a, float b, float t)
		{
			return a + t * (b - a);
		}
		
		float Grad(int hash, float x, float y)
		{
			int h = hash & 15;
			float grad = 1.0f + (h & 7);
			if (h & 8) grad = -grad;
			return (h & 1 ? grad * x : grad * y);
		}
		
		int m_Seed;
		std::vector<int> m_Permutation;
	};

	// World generator class for creating terrain
	class WorldGenerator
	{
	public:
		WorldGenerator(int seed) : m_Noise(seed), m_Seed(seed) {}
		
		// Generate terrain for a chunk
		void GenerateChunk(ChunkComponent& chunk, const TerrainGeneratorComponent& params)
		{
			glm::ivec3 worldOffset = chunk.ChunkPosition * glm::ivec3(CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z);
			
			for (int x = 0; x < CHUNK_SIZE_X; x++)
			{
				for (int z = 0; z < CHUNK_SIZE_Z; z++)
				{
					int worldX = worldOffset.x + x;
					int worldZ = worldOffset.z + z;
					
					// Generate height using noise
					float height = GenerateHeight(worldX, worldZ, params);
					int terrainHeight = params.BaseHeight + (int)(height * params.HeightMultiplier);
					
					// Clamp height to chunk bounds
					terrainHeight = glm::clamp(terrainHeight, 0, CHUNK_SIZE_Y - 1);
					
					for (int y = 0; y < CHUNK_SIZE_Y; y++)
					{
						int worldY = y;
						BlockType blockType = BlockType::Air;
						
						if (y == 0)
						{
							// Bedrock layer at bottom
							blockType = BlockType::Bedrock;
						}
						else if (y <= terrainHeight)
						{
							// Determine block type based on height
							if (y == terrainHeight && terrainHeight > params.WaterLevel)
							{
								// Top layer - grass
								blockType = BlockType::Grass;
							}
							else if (y > terrainHeight - 4 && terrainHeight > params.WaterLevel)
							{
								// Few layers below grass - dirt
								blockType = BlockType::Dirt;
							}
							else
							{
								// Deep underground - stone
								blockType = BlockType::Stone;
								
								// Add ores
								float oreNoise = m_Noise.Noise2D(worldX * 0.1f, y * 0.1f + worldZ * 0.1f);
								if (y < 32 && oreNoise > 0.7f)
									blockType = BlockType::Coal;
								else if (y < 24 && oreNoise > 0.8f)
									blockType = BlockType::Iron;
								else if (y < 16 && oreNoise > 0.85f)
									blockType = BlockType::Gold;
								else if (y < 12 && oreNoise > 0.9f)
									blockType = BlockType::Diamond;
							}
						}
						else if (y <= params.WaterLevel)
						{
							// Fill with water up to water level
							blockType = BlockType::Water;
						}
						
						chunk.SetBlock(x, y, z, Block(blockType));
					}
				}
			}
			
			chunk.IsLoaded = true;
			chunk.NeedsMeshRebuild = true;
		}
		
	private:
		float GenerateHeight(int x, int z, const TerrainGeneratorComponent& params)
		{
			// Multi-octave noise for varied terrain
			float height = 0.0f;
			float amplitude = 1.0f;
			float frequency = params.Scale;
			
			// Multiple octaves for detail
			for (int i = 0; i < 4; i++)
			{
				height += m_Noise.Noise2D(x * frequency, z * frequency) * amplitude;
				amplitude *= 0.5f;
				frequency *= 2.0f;
			}
			
			return height;
		}
		
		SimplexNoise m_Noise;
		int m_Seed;
	};

}
