#include "Sandbox2D.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f), m_SquareColor({ 0.2f, 0.3f, 0.8f, 1.0f }),
	  m_WorldGenerator(12345), m_Chunk(glm::ivec3(0, 0, 0))
{
	// Initialize block colors for visualization
	m_BlockColors[Hazel::BlockType::Air] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_BlockColors[Hazel::BlockType::Grass] = { 0.2f, 0.8f, 0.2f, 1.0f };
	m_BlockColors[Hazel::BlockType::Dirt] = { 0.6f, 0.4f, 0.2f, 1.0f };
	m_BlockColors[Hazel::BlockType::Stone] = { 0.5f, 0.5f, 0.5f, 1.0f };
	m_BlockColors[Hazel::BlockType::Wood] = { 0.6f, 0.3f, 0.1f, 1.0f };
	m_BlockColors[Hazel::BlockType::Leaves] = { 0.1f, 0.6f, 0.1f, 0.8f };
	m_BlockColors[Hazel::BlockType::Sand] = { 0.9f, 0.9f, 0.5f, 1.0f };
	m_BlockColors[Hazel::BlockType::Water] = { 0.2f, 0.4f, 0.9f, 0.7f };
	m_BlockColors[Hazel::BlockType::Bedrock] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_BlockColors[Hazel::BlockType::Coal] = { 0.3f, 0.3f, 0.3f, 1.0f };
	m_BlockColors[Hazel::BlockType::Iron] = { 0.8f, 0.7f, 0.6f, 1.0f };
	m_BlockColors[Hazel::BlockType::Gold] = { 1.0f, 0.8f, 0.0f, 1.0f };
	m_BlockColors[Hazel::BlockType::Diamond] = { 0.4f, 0.8f, 1.0f, 1.0f };
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	m_CheckerboardTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
	
	// Generate initial chunk terrain
	m_WorldGenerator.GenerateChunk(m_Chunk, m_TerrainParams);
	
	// Initialize player inventory with some blocks
	m_Inventory.AddItem(Hazel::BlockType::Grass, 64);
	m_Inventory.AddItem(Hazel::BlockType::Stone, 64);
	m_Inventory.AddItem(Hazel::BlockType::Wood, 64);
	m_Inventory.AddItem(Hazel::BlockType::Dirt, 64);
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(ts);

	// Render
	Hazel::Renderer2D::ResetStats();
	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		Hazel::RenderCommand::SetClearColor({ 0.53f, 0.81f, 0.92f, 1.0f }); // Sky blue
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Sandbox World Render");
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		
		// Render the voxel world (2D cross-section view)
		RenderChunk(m_Chunk);
		
		Hazel::Renderer2D::EndScene();
	}
}

void Sandbox2D::RenderChunk(Hazel::ChunkComponent& chunk)
{
	// Render a 2D cross-section of the chunk (X-Y plane at Z=8)
	const int viewZ = 8;
	const float blockSize = 0.5f;
	
	for (int x = 0; x < CHUNK_SIZE_X; x++)
	{
		for (int y = 0; y < 128; y++) // Only render first 128 blocks high for performance
		{
			const auto& block = chunk.GetBlock(x, y, viewZ);
			
			// Skip air blocks
			if (block.Type == Hazel::BlockType::Air)
				continue;
			
			// Get block color
			glm::vec4 color = m_BlockColors[block.Type];
			
			// Calculate position
			glm::vec3 position = {
				(x - CHUNK_SIZE_X / 2.0f) * blockSize,
				(y - 64.0f) * blockSize,
				0.0f
			};
			
			// Draw block
			Hazel::Renderer2D::DrawQuad(position, { blockSize, blockSize }, color);
		}
	}
}

void Sandbox2D::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Sandbox World Settings");

	auto stats = Hazel::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	
	ImGui::Separator();
	ImGui::Text("World Info:");
	ImGui::Text("Chunk Position: (%d, %d, %d)", 
		m_Chunk.ChunkPosition.x, m_Chunk.ChunkPosition.y, m_Chunk.ChunkPosition.z);
	ImGui::Text("Chunk Loaded: %s", m_Chunk.IsLoaded ? "Yes" : "No");
	
	ImGui::Separator();
	ImGui::Text("Terrain Generation:");
	if (ImGui::SliderFloat("Height Multiplier", &m_TerrainParams.HeightMultiplier, 10.0f, 64.0f))
		m_Chunk.NeedsMeshRebuild = true;
	if (ImGui::SliderInt("Base Height", &m_TerrainParams.BaseHeight, 32, 128))
		m_Chunk.NeedsMeshRebuild = true;
	if (ImGui::SliderInt("Water Level", &m_TerrainParams.WaterLevel, 32, 96))
		m_Chunk.NeedsMeshRebuild = true;
	
	if (ImGui::Button("Regenerate Terrain"))
	{
		m_WorldGenerator.GenerateChunk(m_Chunk, m_TerrainParams);
	}
	
	ImGui::Separator();
	ImGui::Text("Player Inventory:");
	ImGui::Text("Selected Slot: %d", m_Inventory.SelectedSlot);
	
	// Show first 9 inventory slots (hotbar)
	for (int i = 0; i < 9; i++)
	{
		auto& item = m_Inventory.Items[i];
		if (item.Type != Hazel::BlockType::Air && item.Count > 0)
		{
			ImGui::Text("Slot %d: Block %d x%d", i, (int)item.Type, item.Count);
		}
	}

	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& e)
{
	m_CameraController.OnEvent(e);
}
