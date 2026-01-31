#pragma once

#include <XingXing.h>

class VoxelWorldLayer : public Hazel::Layer
{
public:
	VoxelWorldLayer();
	virtual ~VoxelWorldLayer() = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Hazel::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Hazel::Event& e) override;

private:
	// Camera
	Hazel::PerspectiveCamera m_Camera;
	glm::vec3 m_CameraPosition;
	glm::vec3 m_CameraRotation;
	float m_CameraSpeed = 5.0f;
	float m_CameraRotationSpeed = 1.0f;
	
	// Scene
	std::vector<glm::vec3> m_BlockPositions;
	Hazel::Ref<Hazel::Texture2D> m_GrassTexture;
	Hazel::Ref<Hazel::Texture2D> m_StoneTexture;
};
