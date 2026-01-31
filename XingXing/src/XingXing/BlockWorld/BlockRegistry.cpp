#include "hzpch.h"
#include "BlockRegistry.h"
#include "XingXing/Core/Log.h"

namespace Hazel {

	std::unordered_map<BlockID, Ref<Block>> BlockRegistry::s_Blocks;

	void BlockRegistry::Init()
	{
		HZ_CORE_INFO("Initializing Block Registry...");
		s_Blocks.clear();
		RegisterDefaultBlocks();
		HZ_CORE_INFO("Block Registry initialized with {0} blocks", s_Blocks.size());
	}

	void BlockRegistry::Shutdown()
	{
		HZ_CORE_INFO("Shutting down Block Registry...");
		s_Blocks.clear();
	}

	void BlockRegistry::RegisterBlock(BlockID id, const BlockProperties& props)
	{
		if (s_Blocks.find(id) != s_Blocks.end())
		{
			HZ_CORE_WARN("Block ID {0} already registered, overwriting", id);
		}

		s_Blocks[id] = CreateRef<Block>(id, props);
		HZ_CORE_TRACE("Registered block: {0} (ID: {1})", props.Name, id);
	}

	const Block* BlockRegistry::GetBlock(BlockID id)
	{
		auto it = s_Blocks.find(id);
		if (it != s_Blocks.end())
			return it->second.get();
		return nullptr;
	}

	bool BlockRegistry::IsValidBlock(BlockID id)
	{
		return s_Blocks.find(id) != s_Blocks.end();
	}

	const std::unordered_map<BlockID, Ref<Block>>& BlockRegistry::GetAllBlocks()
	{
		return s_Blocks;
	}

	void BlockRegistry::RegisterDefaultBlocks()
	{
		// 空气方块（特殊方块，不可见）
		BlockProperties airProps;
		airProps.Name = "Air";
		airProps.IsSolid = false;
		airProps.IsTransparent = true;
		airProps.Color = { 0.0f, 0.0f, 0.0f, 0.0f };
		RegisterBlock(BLOCK_AIR, airProps);

		// 石头
		BlockProperties stoneProps;
		stoneProps.Name = "Stone";
		stoneProps.IsSolid = true;
		stoneProps.IsTransparent = false;
		stoneProps.Hardness = 1.5f;
		stoneProps.Color = { 0.5f, 0.5f, 0.5f, 1.0f };
		RegisterBlock(BLOCK_STONE, stoneProps);

		// 泥土
		BlockProperties dirtProps;
		dirtProps.Name = "Dirt";
		dirtProps.IsSolid = true;
		dirtProps.IsTransparent = false;
		dirtProps.Hardness = 0.5f;
		dirtProps.Color = { 0.55f, 0.35f, 0.2f, 1.0f };
		RegisterBlock(BLOCK_DIRT, dirtProps);

		// 草方块
		BlockProperties grassProps;
		grassProps.Name = "Grass";
		grassProps.IsSolid = true;
		grassProps.IsTransparent = false;
		grassProps.Hardness = 0.6f;
		grassProps.Color = { 0.3f, 0.7f, 0.2f, 1.0f };
		RegisterBlock(BLOCK_GRASS, grassProps);

		// 木头
		BlockProperties woodProps;
		woodProps.Name = "Wood";
		woodProps.IsSolid = true;
		woodProps.IsTransparent = false;
		woodProps.Hardness = 0.8f;
		woodProps.Color = { 0.6f, 0.4f, 0.2f, 1.0f };
		RegisterBlock(BLOCK_WOOD, woodProps);

		// 沙子（受重力影响）
		BlockProperties sandProps;
		sandProps.Name = "Sand";
		sandProps.IsSolid = true;
		sandProps.IsTransparent = false;
		sandProps.HasGravity = true;
		sandProps.Hardness = 0.5f;
		sandProps.Color = { 0.95f, 0.9f, 0.6f, 1.0f };
		RegisterBlock(BLOCK_SAND, sandProps);
	}

} // namespace Hazel
