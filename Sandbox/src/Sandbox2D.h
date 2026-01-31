#pragma once

#include "xingxing.h"
#include "XingXing/Scene/SandboxComponents.h"
#include "XingXing/Scene/WorldGenerator.h"

class Sandbox2D : public Hazel::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;
private:
	void RenderChunk(Hazel::ChunkComponent& chunk);
	void UpdateBlockInteraction();
	
	Hazel::OrthographicCameraController m_CameraController;
	
	// Sandbox game components
	Hazel::ChunkComponent m_Chunk;
	Hazel::WorldGenerator m_WorldGenerator;
	Hazel::TerrainGeneratorComponent m_TerrainParams;
	Hazel::PlayerInventoryComponent m_Inventory;
	Hazel::BlockInteractionComponent m_Interaction;
	
	// Block textures
	std::unordered_map<Hazel::BlockType, glm::vec4> m_BlockColors;
	
	// Camera zoom for voxel view
	float m_CameraZoom = 1.0f;
	
	// Selected block type for placement
	Hazel::BlockType m_SelectedBlockType = Hazel::BlockType::Stone;
	
	// Temp
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;
	Hazel::Ref<Hazel::Shader> m_FlatColorShader;

	Hazel::Ref<Hazel::Texture2D> m_CheckerboardTexture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
