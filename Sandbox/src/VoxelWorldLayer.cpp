#include "VoxelWorldLayer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

VoxelWorldLayer::VoxelWorldLayer()
	: Layer("VoxelWorld"),
	  m_Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f),
	  m_CameraPosition(0.0f, 5.0f, 10.0f),
	  m_CameraRotation(0.0f)
{
}

void VoxelWorldLayer::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	// Initialize 3D renderer
	Hazel::Renderer3D::Init();
	
	// Create simple textures
	m_GrassTexture = Hazel::Texture2D::Create(1, 1);
	uint32_t grassData = 0xFF4CAF50; // Green
	m_GrassTexture->SetData(&grassData, sizeof(uint32_t));
	
	m_StoneTexture = Hazel::Texture2D::Create(1, 1);
	uint32_t stoneData = 0xFF808080; // Gray
	m_StoneTexture->SetData(&stoneData, sizeof(uint32_t));
	
	// Create a simple terrain (20x20 ground + some random blocks)
	for (int x = -10; x < 10; x++)
	{
		for (int z = -10; z < 10; z++)
		{
			// Ground layer
			m_BlockPositions.push_back(glm::vec3(x, 0, z));
			
			// Add some random blocks on top
			if ((x + z) % 5 == 0)
			{
				m_BlockPositions.push_back(glm::vec3(x, 1, z));
			}
			if ((x * z) % 7 == 0 && x != 0 && z != 0)
			{
				m_BlockPositions.push_back(glm::vec3(x, 2, z));
			}
		}
	}
	
	HZ_INFO("VoxelWorldLayer: Created {0} blocks", m_BlockPositions.size());
}

void VoxelWorldLayer::OnDetach()
{
	HZ_PROFILE_FUNCTION();

	Hazel::Renderer3D::Shutdown();
}

void VoxelWorldLayer::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	// Camera movement (WASD + QE for up/down)
	float velocity = m_CameraSpeed * ts;
	
	// Calculate forward and right vectors
	glm::vec3 forward(
		sin(m_CameraRotation.y) * cos(m_CameraRotation.x),
		sin(m_CameraRotation.x),
		cos(m_CameraRotation.y) * cos(m_CameraRotation.x)
	);
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
	
	if (Hazel::Input::IsKeyPressed(Hazel::Key::W))
		m_CameraPosition += forward * velocity;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::S))
		m_CameraPosition -= forward * velocity;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::A))
		m_CameraPosition -= right * velocity;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::D))
		m_CameraPosition += right * velocity;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::Q))
		m_CameraPosition.y -= velocity;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::E))
		m_CameraPosition.y += velocity;
	
	// Camera rotation (Arrow keys)
	float rotSpeed = m_CameraRotationSpeed * ts;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::Up))
		m_CameraRotation.x += rotSpeed;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::Down))
		m_CameraRotation.x -= rotSpeed;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::Left))
		m_CameraRotation.y -= rotSpeed;
	if (Hazel::Input::IsKeyPressed(Hazel::Key::Right))
		m_CameraRotation.y += rotSpeed;
	
	// Clamp pitch
	m_CameraRotation.x = glm::clamp(m_CameraRotation.x, -1.5f, 1.5f);
	
	// Update camera
	m_Camera.SetPosition(m_CameraPosition);
	m_Camera.SetRotation(m_CameraRotation);

	// Render
	Hazel::RenderCommand::SetClearColor({ 0.53f, 0.81f, 0.92f, 1.0f }); // Sky blue
	Hazel::RenderCommand::Clear();
	
	Hazel::Renderer3D::ResetStats();
	Hazel::Renderer3D::BeginScene(m_Camera);
	
	// Draw all blocks
	for (size_t i = 0; i < m_BlockPositions.size(); i++)
	{
		const auto& pos = m_BlockPositions[i];
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos);
		
		// Use grass texture for ground, stone for upper blocks
		if (pos.y == 0)
			Hazel::Renderer3D::DrawCube(transform, m_GrassTexture);
		else
			Hazel::Renderer3D::DrawCube(transform, m_StoneTexture);
	}
	
	Hazel::Renderer3D::EndScene();
}

void VoxelWorldLayer::OnImGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("3D Voxel World");
	
	ImGui::Text("Controls:");
	ImGui::BulletText("WASD - Move horizontally");
	ImGui::BulletText("Q/E - Move up/down");
	ImGui::BulletText("Arrow Keys - Rotate camera");
	ImGui::Separator();
	
	ImGui::Text("Camera Position: %.1f, %.1f, %.1f", 
		m_CameraPosition.x, m_CameraPosition.y, m_CameraPosition.z);
	ImGui::Text("Camera Rotation: %.2f, %.2f, %.2f", 
		m_CameraRotation.x, m_CameraRotation.y, m_CameraRotation.z);
	
	ImGui::SliderFloat("Camera Speed", &m_CameraSpeed, 1.0f, 20.0f);
	ImGui::SliderFloat("Rotation Speed", &m_CameraRotationSpeed, 0.5f, 3.0f);
	
	ImGui::Separator();
	ImGui::Text("Scene Stats:");
	ImGui::Text("Total Blocks: %d", (int)m_BlockPositions.size());
	
	auto stats = Hazel::Renderer3D::GetStats();
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Cubes Rendered: %d", stats.CubeCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	
	ImGui::End();
}

void VoxelWorldLayer::OnEvent(Hazel::Event& e)
{
}
