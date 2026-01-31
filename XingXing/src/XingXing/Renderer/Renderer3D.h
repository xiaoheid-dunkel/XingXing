#pragma once

#include "XingXing/Renderer/PerspectiveCamera.h"
#include "XingXing/Renderer/Camera.h"
#include "XingXing/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace Hazel {

	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const PerspectiveCamera& camera);
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void EndScene();
		static void Flush();

		// Primitives
		static void DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
		static void DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawCube(const glm::mat4& transform, const glm::vec4& color);
		static void DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t CubeCount = 0;

			uint32_t GetTotalVertexCount() const { return CubeCount * 24; } // 6 faces * 4 vertices
			uint32_t GetTotalIndexCount() const { return CubeCount * 36; }  // 6 faces * 6 indices
		};
		static void ResetStats();
		static Statistics GetStats();

	private:
		static void StartBatch();
		static void NextBatch();
	};

}
