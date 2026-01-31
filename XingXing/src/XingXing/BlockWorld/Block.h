#pragma once

#include "XingXing/Core/Core.h"
#include "XingXing/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <string>

namespace Hazel {

	using BlockID = uint16_t;

	// 特殊方块ID常量
	constexpr BlockID BLOCK_AIR = 0;
	constexpr BlockID BLOCK_STONE = 1;
	constexpr BlockID BLOCK_DIRT = 2;
	constexpr BlockID BLOCK_GRASS = 3;
	constexpr BlockID BLOCK_WOOD = 4;
	constexpr BlockID BLOCK_SAND = 5;

	/**
	 * @brief 方块属性结构
	 * 定义方块的外观、物理属性和交互行为
	 */
	struct BlockProperties
	{
		std::string Name = "Unknown";
		bool IsSolid = true;          // 是否为固体（影响碰撞）
		bool IsTransparent = false;   // 是否透明（影响渲染）
		bool HasGravity = false;      // 是否受重力影响（如沙子）
		float Hardness = 1.0f;        // 硬度（影响破坏时间）
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };  // 方块颜色
		Ref<Texture2D> Texture = nullptr;  // 方块纹理（可选）
	};

	/**
	 * @brief 方块类
	 * 表示游戏中的一种方块类型
	 */
	class Block
	{
	public:
		Block() = default;
		Block(BlockID id, const BlockProperties& props)
			: m_ID(id), m_Properties(props) {}

		BlockID GetID() const { return m_ID; }
		const BlockProperties& GetProperties() const { return m_Properties; }
		const std::string& GetName() const { return m_Properties.Name; }

		// 便捷访问器
		bool IsSolid() const { return m_Properties.IsSolid; }
		bool IsTransparent() const { return m_Properties.IsTransparent; }
		bool HasGravity() const { return m_Properties.HasGravity; }
		float GetHardness() const { return m_Properties.Hardness; }
		const glm::vec4& GetColor() const { return m_Properties.Color; }
		Ref<Texture2D> GetTexture() const { return m_Properties.Texture; }

	private:
		BlockID m_ID = BLOCK_AIR;
		BlockProperties m_Properties;
	};

} // namespace Hazel
