#include "BlockSandboxLayer.h"
#include "XingXing/BlockWorld/BlockRegistry.h"

#include <imgui/imgui.h>

BlockSandboxLayer::BlockSandboxLayer()
	: Layer("BlockSandbox"), m_CameraController(1280.0f / 720.0f, true)
{
}

void BlockSandboxLayer::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	// 初始化方块注册表
	Hazel::BlockRegistry::Init();

	// 创建世界
	m_World = Hazel::CreateRef<Hazel::World>();

	HZ_INFO("Block Sandbox Layer attached!");
	HZ_INFO("Controls:");
	HZ_INFO("  A/D - Move left/right");
	HZ_INFO("  Space - Jump");
	HZ_INFO("  Left Click - Break block");
	HZ_INFO("  Right Click - Place block");
	HZ_INFO("  1-6 - Select block type");
	HZ_INFO("  Scroll - Zoom camera");
}

void BlockSandboxLayer::OnDetach()
{
	HZ_PROFILE_FUNCTION();
	Hazel::BlockRegistry::Shutdown();
}

void BlockSandboxLayer::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	// 处理输入
	HandleInput(ts);

	// 更新物理
	UpdatePhysics(ts);

	// 更新摄像机（跟随玩家）
	m_CameraController.GetCamera().SetPosition({ m_PlayerPosition.x, m_PlayerPosition.y, 0.0f });
	m_CameraController.OnUpdate(ts);

	// 更新世界（加载/卸载区块）
	m_World->Update(m_PlayerPosition);

	// 渲染
	Hazel::RenderCommand::SetClearColor({ 0.53f, 0.81f, 0.92f, 1.0f }); // 天空蓝
	Hazel::RenderCommand::Clear();

	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

	// 渲染世界
	m_World->Render(m_CameraController.GetCamera().GetViewProjectionMatrix());

	// 渲染玩家（红色方块）
	Hazel::Renderer2D::DrawQuad(
		{ m_PlayerPosition.x, m_PlayerPosition.y, 0.1f },
		m_PlayerSize,
		{ 1.0f, 0.0f, 0.0f, 1.0f }
	);

	// 渲染目标方块指示器
	glm::ivec2 targetBlock = GetTargetBlock();
	Hazel::Renderer2D::DrawRect(
		{ targetBlock.x, targetBlock.y, 0.2f },
		{ 1.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 0.5f }
	);

	Hazel::Renderer2D::EndScene();
}

void BlockSandboxLayer::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Block Sandbox Settings");

	// 显示统计信息
	ImGui::Text("Player Position: (%.1f, %.1f)", m_PlayerPosition.x, m_PlayerPosition.y);
	ImGui::Text("Player Velocity: (%.1f, %.1f)", m_PlayerVelocity.x, m_PlayerVelocity.y);
	ImGui::Text("On Ground: %s", m_IsOnGround ? "Yes" : "No");
	ImGui::Text("Loaded Chunks: %zu", m_World->GetLoadedChunkCount());

	ImGui::Separator();

	// 方块选择
	ImGui::Text("Selected Block:");
	
	const char* blockNames[] = { "Stone", "Dirt", "Grass", "Wood", "Sand" };
	Hazel::BlockID blockIDs[] = { 
		Hazel::BLOCK_STONE, 
		Hazel::BLOCK_DIRT, 
		Hazel::BLOCK_GRASS, 
		Hazel::BLOCK_WOOD, 
		Hazel::BLOCK_SAND 
	};

	for (int i = 0; i < 5; i++)
	{
		if (ImGui::RadioButton(blockNames[i], m_SelectedBlock == blockIDs[i]))
		{
			m_SelectedBlock = blockIDs[i];
		}
	}

	ImGui::Separator();

	// 目标方块信息
	glm::ivec2 targetBlock = GetTargetBlock();
	ImGui::Text("Target Block: (%d, %d)", targetBlock.x, targetBlock.y);
	Hazel::BlockID blockAtTarget = m_World->GetBlock(targetBlock.x, targetBlock.y);
	const Hazel::Block* block = Hazel::BlockRegistry::GetBlock(blockAtTarget);
	if (block)
	{
		ImGui::Text("Block Type: %s", block->GetName().c_str());
	}

	ImGui::Separator();

	// 世界设置
	int loadRadius = m_World->GetLoadRadius();
	if (ImGui::SliderInt("Chunk Load Radius", &loadRadius, 1, 10))
	{
		m_World->SetLoadRadius(loadRadius);
	}

	ImGui::End();
}

void BlockSandboxLayer::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);

	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(BlockSandboxLayer::OnMouseButtonPressed));
	dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(BlockSandboxLayer::OnKeyPressed));
}

void BlockSandboxLayer::HandleInput(Hazel::Timestep ts)
{
	// 移动控制
	float speed = 5.0f;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::A))
		m_PlayerVelocity.x = -speed;
	else if (Hazel::Input::IsKeyPressed(Hazel::Key::D))
		m_PlayerVelocity.x = speed;
	else
		m_PlayerVelocity.x = 0.0f;

	// 跳跃
	if (Hazel::Input::IsKeyPressed(Hazel::Key::Space) && m_IsOnGround)
	{
		m_PlayerVelocity.y = 12.0f;
		m_IsOnGround = false;
	}
}

void BlockSandboxLayer::HandleBlockInteraction()
{
	glm::ivec2 targetBlock = GetTargetBlock();
	Hazel::BlockID currentBlock = m_World->GetBlock(targetBlock.x, targetBlock.y);

	// 左键破坏方块
	if (Hazel::Input::IsMouseButtonPressed(Hazel::Mouse::ButtonLeft))
	{
		if (currentBlock != Hazel::BLOCK_AIR)
		{
			m_World->SetBlock(targetBlock.x, targetBlock.y, Hazel::BLOCK_AIR);
			HZ_INFO("Broke block at ({0}, {1})", targetBlock.x, targetBlock.y);
		}
	}
	// 右键放置方块
	else if (Hazel::Input::IsMouseButtonPressed(Hazel::Mouse::ButtonRight))
	{
		if (currentBlock == Hazel::BLOCK_AIR)
		{
			// 检查是否与玩家碰撞
			glm::vec2 blockPos = { targetBlock.x, targetBlock.y };
			bool colliding = CheckCollision(blockPos, { 1.0f, 1.0f });

			if (!colliding)
			{
				m_World->SetBlock(targetBlock.x, targetBlock.y, m_SelectedBlock);
				HZ_INFO("Placed block at ({0}, {1})", targetBlock.x, targetBlock.y);
			}
		}
	}
}

bool BlockSandboxLayer::OnMouseButtonPressed(Hazel::MouseButtonPressedEvent& e)
{
	HandleBlockInteraction();
	return false;
}

bool BlockSandboxLayer::OnKeyPressed(Hazel::KeyPressedEvent& e)
{
	// 数字键选择方块
	switch (e.GetKeyCode())
	{
	case Hazel::Key::D1: m_SelectedBlock = Hazel::BLOCK_STONE; break;
	case Hazel::Key::D2: m_SelectedBlock = Hazel::BLOCK_DIRT; break;
	case Hazel::Key::D3: m_SelectedBlock = Hazel::BLOCK_GRASS; break;
	case Hazel::Key::D4: m_SelectedBlock = Hazel::BLOCK_WOOD; break;
	case Hazel::Key::D5: m_SelectedBlock = Hazel::BLOCK_SAND; break;
	}
	return false;
}

void BlockSandboxLayer::UpdatePhysics(Hazel::Timestep ts)
{
	// 应用重力
	m_PlayerVelocity.y -= 30.0f * ts;

	// 限制下落速度
	if (m_PlayerVelocity.y < -20.0f)
		m_PlayerVelocity.y = -20.0f;

	// 更新位置
	glm::vec2 newPosition = m_PlayerPosition + m_PlayerVelocity * ts;

	// 简单的碰撞检测
	// X轴碰撞
	if (!CheckCollision({ newPosition.x, m_PlayerPosition.y }, m_PlayerSize))
	{
		m_PlayerPosition.x = newPosition.x;
	}
	else
	{
		m_PlayerVelocity.x = 0.0f;
	}

	// Y轴碰撞
	if (!CheckCollision({ m_PlayerPosition.x, newPosition.y }, m_PlayerSize))
	{
		m_PlayerPosition.y = newPosition.y;
		m_IsOnGround = false;
	}
	else
	{
		// 如果向下移动时碰撞，说明在地面上
		if (m_PlayerVelocity.y < 0.0f)
		{
			m_IsOnGround = true;
		}
		m_PlayerVelocity.y = 0.0f;
	}
}

bool BlockSandboxLayer::CheckCollision(const glm::vec2& position, const glm::vec2& size)
{
	// 检查玩家边界框与方块的碰撞
	float halfWidth = size.x / 2.0f;
	float halfHeight = size.y / 2.0f;

	// 玩家的四个角
	glm::vec2 corners[4] = {
		{ position.x - halfWidth, position.y - halfHeight },  // 左下
		{ position.x + halfWidth, position.y - halfHeight },  // 右下
		{ position.x - halfWidth, position.y + halfHeight },  // 左上
		{ position.x + halfWidth, position.y + halfHeight }   // 右上
	};

	// 检查每个角是否与固体方块碰撞
	for (const auto& corner : corners)
	{
		int blockX = static_cast<int>(std::floor(corner.x));
		int blockY = static_cast<int>(std::floor(corner.y));

		Hazel::BlockID blockID = m_World->GetBlock(blockX, blockY);
		const Hazel::Block* block = Hazel::BlockRegistry::GetBlock(blockID);

		if (block && block->IsSolid())
		{
			return true;
		}
	}

	return false;
}

glm::ivec2 BlockSandboxLayer::GetTargetBlock()
{
	// 获取玩家面前的方块（简单实现）
	glm::vec2 target = m_PlayerPosition + glm::vec2(1.0f, 0.0f);
	return { 
		static_cast<int>(std::floor(target.x)), 
		static_cast<int>(std::floor(target.y)) 
	};
}
