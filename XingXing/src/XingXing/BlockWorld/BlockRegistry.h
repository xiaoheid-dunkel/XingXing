#pragma once

#include "Block.h"
#include <unordered_map>
#include <memory>

namespace Hazel {

	/**
	 * @brief 方块注册表
	 * 管理所有方块类型的全局单例
	 */
	class BlockRegistry
	{
	public:
		/**
		 * @brief 初始化方块注册表并注册默认方块
		 */
		static void Init();

		/**
		 * @brief 清理方块注册表
		 */
		static void Shutdown();

		/**
		 * @brief 注册一个新的方块类型
		 * @param id 方块ID
		 * @param props 方块属性
		 */
		static void RegisterBlock(BlockID id, const BlockProperties& props);

		/**
		 * @brief 获取指定ID的方块
		 * @param id 方块ID
		 * @return 方块指针，如果不存在返回nullptr
		 */
		static const Block* GetBlock(BlockID id);

		/**
		 * @brief 检查方块ID是否有效
		 * @param id 方块ID
		 * @return 如果方块存在返回true
		 */
		static bool IsValidBlock(BlockID id);

		/**
		 * @brief 获取所有注册的方块
		 * @return 方块映射表的常量引用
		 */
		static const std::unordered_map<BlockID, Ref<Block>>& GetAllBlocks();

	private:
		static std::unordered_map<BlockID, Ref<Block>> s_Blocks;
		static void RegisterDefaultBlocks();
	};

} // namespace Hazel
