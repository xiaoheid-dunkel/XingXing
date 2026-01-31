#pragma once

#include "XingXing.h"
#include "XingXing/BlockWorld/World.h"

/**
 * @brief 方块沙盒演示层
 * 展示如何使用方块世界系统创建简单的沙盒游戏
 */
class BlockSandboxLayer : public Hazel::Layer
{
public:
	BlockSandboxLayer();
	virtual ~BlockSandboxLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;

private:
	// 摄像机控制
	Hazel::OrthographicCameraController m_CameraController;

	// 方块世界
	Hazel::Ref<Hazel::World> m_World;

	// 玩家状态
	glm::vec2 m_PlayerPosition = { 0.0f, 15.0f };
	glm::vec2 m_PlayerVelocity = { 0.0f, 0.0f };
	glm::vec2 m_PlayerSize = { 0.8f, 1.8f };
	Hazel::BlockID m_SelectedBlock = Hazel::BLOCK_STONE;
	bool m_IsOnGround = false;

	// 输入处理
	void HandleInput(Hazel::Timestep ts);
	void HandleBlockInteraction();
	bool OnMouseButtonPressed(Hazel::MouseButtonPressedEvent& e);
	bool OnKeyPressed(Hazel::KeyPressedEvent& e);

	// 物理
	void UpdatePhysics(Hazel::Timestep ts);
	bool CheckCollision(const glm::vec2& position, const glm::vec2& size);

	// 辅助功能
	glm::ivec2 GetTargetBlock();
};
