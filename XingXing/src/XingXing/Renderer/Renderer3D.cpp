#include "hzpch.h"
#include "Renderer3D.h"

#include "XingXing/Renderer/VertexArray.h"
#include "XingXing/Renderer/Shader.h"
#include "XingXing/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {

	struct CubeVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
	};

	struct Renderer3DData
	{
		static const uint32_t MaxCubes = 1000;
		static const uint32_t MaxVertices = MaxCubes * 24;
		static const uint32_t MaxIndices = MaxCubes * 36;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<Shader> CubeShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t CubeIndexCount = 0;
		CubeVertex* CubeVertexBufferBase = nullptr;
		CubeVertex* CubeVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		glm::mat4 ViewProjectionMatrix;

		Renderer3D::Statistics Stats;
	};

	static Renderer3DData s_Data;

	void Renderer3D::Init()
	{
		HZ_PROFILE_FUNCTION();

		// Create vertex array
		s_Data.CubeVertexArray = VertexArray::Create();

		// Create vertex buffer
		s_Data.CubeVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CubeVertex));
		s_Data.CubeVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal" },
			{ ShaderDataType::Float4, "a_Color" },
			{ ShaderDataType::Float2, "a_TexCoord" },
			{ ShaderDataType::Float,  "a_TexIndex" }
		});
		s_Data.CubeVertexArray->AddVertexBuffer(s_Data.CubeVertexBuffer);

		s_Data.CubeVertexBufferBase = new CubeVertex[s_Data.MaxVertices];

		// Create index buffer for cubes
		uint32_t* cubeIndices = new uint32_t[s_Data.MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36)
		{
			// Front face
			cubeIndices[i + 0] = offset + 0; cubeIndices[i + 1] = offset + 1; cubeIndices[i + 2] = offset + 2;
			cubeIndices[i + 3] = offset + 2; cubeIndices[i + 4] = offset + 3; cubeIndices[i + 5] = offset + 0;
			// Back face
			cubeIndices[i + 6] = offset + 4; cubeIndices[i + 7] = offset + 5; cubeIndices[i + 8] = offset + 6;
			cubeIndices[i + 9] = offset + 6; cubeIndices[i + 10] = offset + 7; cubeIndices[i + 11] = offset + 4;
			// Top face
			cubeIndices[i + 12] = offset + 8; cubeIndices[i + 13] = offset + 9; cubeIndices[i + 14] = offset + 10;
			cubeIndices[i + 15] = offset + 10; cubeIndices[i + 16] = offset + 11; cubeIndices[i + 17] = offset + 8;
			// Bottom face
			cubeIndices[i + 18] = offset + 12; cubeIndices[i + 19] = offset + 13; cubeIndices[i + 20] = offset + 14;
			cubeIndices[i + 21] = offset + 14; cubeIndices[i + 22] = offset + 15; cubeIndices[i + 23] = offset + 12;
			// Right face
			cubeIndices[i + 24] = offset + 16; cubeIndices[i + 25] = offset + 17; cubeIndices[i + 26] = offset + 18;
			cubeIndices[i + 27] = offset + 18; cubeIndices[i + 28] = offset + 19; cubeIndices[i + 29] = offset + 16;
			// Left face
			cubeIndices[i + 30] = offset + 20; cubeIndices[i + 31] = offset + 21; cubeIndices[i + 32] = offset + 22;
			cubeIndices[i + 33] = offset + 22; cubeIndices[i + 34] = offset + 23; cubeIndices[i + 35] = offset + 20;

			offset += 24;
		}

		Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, s_Data.MaxIndices);
		s_Data.CubeVertexArray->SetIndexBuffer(cubeIB);
		delete[] cubeIndices;

		// Create white texture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// Create shader
		s_Data.CubeShader = Shader::Create("assets/shaders/Renderer3D_Cube.glsl");

		s_Data.TextureSlots[0] = s_Data.WhiteTexture;
	}

	void Renderer3D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		delete[] s_Data.CubeVertexBufferBase;
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		s_Data.ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		s_Data.CubeShader->Bind();
		s_Data.CubeShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

		StartBatch();
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
		s_Data.ViewProjectionMatrix = viewProj;
		
		s_Data.CubeShader->Bind();
		s_Data.CubeShader->SetMat4("u_ViewProjection", s_Data.ViewProjectionMatrix);

		StartBatch();
	}

	void Renderer3D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer3D::Flush()
	{
		if (s_Data.CubeIndexCount == 0)
			return; // Nothing to draw

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase);
		s_Data.CubeVertexBuffer->SetData(s_Data.CubeVertexBufferBase, dataSize);

		// Bind textures
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			s_Data.TextureSlots[i]->Bind(i);

		s_Data.CubeShader->Bind();
		RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer3D::StartBatch()
	{
		s_Data.CubeIndexCount = 0;
		s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), size);
		DrawCube(transform, color);
	}

	void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), size);
		DrawCube(transform, texture, tintColor);
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		constexpr size_t cubeVertexCount = 24;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		if (s_Data.CubeIndexCount >= Renderer3DData::MaxIndices)
			NextBatch();

		// Define cube vertices (each face has 4 vertices)
		glm::vec3 cubeVertices[24] = {
			// Front face (z = 0.5)
			{ -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f },
			// Back face (z = -0.5)
			{  0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f },
			// Top face (y = 0.5)
			{ -0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f },
			// Bottom face (y = -0.5)
			{ -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f,  0.5f },
			// Right face (x = 0.5)
			{  0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f,  0.5f },
			// Left face (x = -0.5)
			{ -0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f, -0.5f }
		};

		glm::vec3 normals[6] = {
			{  0.0f,  0.0f,  1.0f }, // Front
			{  0.0f,  0.0f, -1.0f }, // Back
			{  0.0f,  1.0f,  0.0f }, // Top
			{  0.0f, -1.0f,  0.0f }, // Bottom
			{  1.0f,  0.0f,  0.0f }, // Right
			{ -1.0f,  0.0f,  0.0f }  // Left
		};

		for (size_t i = 0; i < cubeVertexCount; i++)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * glm::vec4(cubeVertices[i], 1.0f);
			s_Data.CubeVertexBufferPtr->Normal = normals[i / 4];
			s_Data.CubeVertexBufferPtr->Color = color;
			s_Data.CubeVertexBufferPtr->TexCoord = textureCoords[i % 4];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += 36;
		s_Data.Stats.CubeCount++;
	}

	void Renderer3D::DrawCube(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		constexpr size_t cubeVertexCount = 24;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		if (s_Data.CubeIndexCount >= Renderer3DData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer3DData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		// Define cube vertices
		glm::vec3 cubeVertices[24] = {
			// Front face
			{ -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f },
			// Back face
			{  0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f },
			// Top face
			{ -0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f,  0.5f }, {  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f },
			// Bottom face
			{ -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f,  0.5f }, { -0.5f, -0.5f,  0.5f },
			// Right face
			{  0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f, -0.5f }, {  0.5f,  0.5f, -0.5f }, {  0.5f,  0.5f,  0.5f },
			// Left face
			{ -0.5f, -0.5f, -0.5f }, { -0.5f, -0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f, -0.5f }
		};

		glm::vec3 normals[6] = {
			{  0.0f,  0.0f,  1.0f }, // Front
			{  0.0f,  0.0f, -1.0f }, // Back
			{  0.0f,  1.0f,  0.0f }, // Top
			{  0.0f, -1.0f,  0.0f }, // Bottom
			{  1.0f,  0.0f,  0.0f }, // Right
			{ -1.0f,  0.0f,  0.0f }  // Left
		};

		for (size_t i = 0; i < cubeVertexCount; i++)
		{
			s_Data.CubeVertexBufferPtr->Position = transform * glm::vec4(cubeVertices[i], 1.0f);
			s_Data.CubeVertexBufferPtr->Normal = normals[i / 4];
			s_Data.CubeVertexBufferPtr->Color = tintColor;
			s_Data.CubeVertexBufferPtr->TexCoord = textureCoords[i % 4];
			s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CubeVertexBufferPtr++;
		}

		s_Data.CubeIndexCount += 36;
		s_Data.Stats.CubeCount++;
	}

	void Renderer3D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer3D::Statistics Renderer3D::GetStats()
	{
		return s_Data.Stats;
	}

}
