#include "iglo.h"
#include "benchmarks.h"

// Shaders
#include "shaders/PS_Color.h"
#include "shaders/VS_InstancedRect.h"
#include "shaders/VS_RawRect.h"
#include "shaders/VS_StructuredRect.h"
#include "shaders/VS_Triangles.h"

#define SHADER_VS(a) ig::Shader(a, sizeof(a), "VSMain")
#define SHADER_PS(a) ig::Shader(a, sizeof(a), "PSMain")

void UpdateQuadsCPU(const ig::IGLOContext& context, Quad* quads_CPU, uint32_t numQuads)
{
	// Move quads to the right
	for (uint32_t i = 0; i < numQuads; i++)
	{
		quads_CPU[i].x += 1.0f;
		if (quads_CPU[i].x > context.GetWidth()) quads_CPU[i].x -= (float)context.GetWidth();
	}
}

void Benchmark_1DrawCall::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION"),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR"),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_Triangles);
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_1DrawCall::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);

	for (uint32_t i = 0; i < numQuads; i++)
	{
		Vertex data[6] =
		{
			{quad[i].x, quad[i].y, quad[i].color},
			{quad[i].x + quad[i].width, quad[i].y, quad[i].color},
			{quad[i].x, quad[i].y + quad[i].height, quad[i].color},
			{quad[i].x, quad[i].y + quad[i].height, quad[i].color},
			{quad[i].x + quad[i].width, quad[i].y, quad[i].color},
			{quad[i].x + quad[i].width, quad[i].y + quad[i].height, quad[i].color},
		};
		cmd.SetTempVertexBuffer(&data, sizeof(data), sizeof(Vertex));
		cmd.Draw(6);
	}
}

void Benchmark_BatchedTriangleList::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertices = std::vector<Vertex>(numQuads * 6);
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), numQuads * 6, ig::BufferUsage::Dynamic);

	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION"),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR"),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_Triangles);
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_BatchedTriangleList::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < numQuads; i++)
	{
		vertices[currentVertex] = { quad[i].x, quad[i].y, quad[i].color };
		vertices[currentVertex + 1] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertices[currentVertex + 2] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertices[currentVertex + 3] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertices[currentVertex + 4] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertices[currentVertex + 5] = { quad[i].x + quad[i].width, quad[i].y + quad[i].height, quad[i].color };
		currentVertex += 6;
	}
	vertexBuffer.SetDynamicData(vertices.data());

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.Draw(vertexBuffer.GetNumElements());
}

void Benchmark_DynamicIndexBuffer::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertices = std::vector<Vertex>(numQuads * 4);
	indices = std::vector<uint32_t>(numQuads * 6);

	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), numQuads * 4, ig::BufferUsage::Dynamic);
	indexBuffer.LoadAsIndexBuffer(context, ig::IndexFormat::UINT32, numQuads * 6, ig::BufferUsage::Dynamic);

	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION"),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR"),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_Triangles);
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_DynamicIndexBuffer::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	uint32_t currentVertex = 0;
	uint32_t currentIndex = 0;
	for (uint32_t i = 0; i < numQuads; i++)
	{
		vertices[currentVertex] = { quad[i].x, quad[i].y, quad[i].color };
		vertices[currentVertex + 1] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertices[currentVertex + 2] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertices[currentVertex + 3] = { quad[i].x + quad[i].width, quad[i].y + quad[i].height, quad[i].color };

		uint32_t baseVertex = currentVertex;
		indices[currentIndex] = baseVertex;
		indices[currentIndex + 1] = baseVertex + 1;
		indices[currentIndex + 2] = baseVertex + 2;
		indices[currentIndex + 3] = baseVertex + 2;
		indices[currentIndex + 4] = baseVertex + 1;
		indices[currentIndex + 5] = baseVertex + 3;

		currentVertex += 4;
		currentIndex += 6;
	}
	vertexBuffer.SetDynamicData(vertices.data());
	indexBuffer.SetDynamicData(indices.data());

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetIndexBuffer(indexBuffer);
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.DrawIndexed(indexBuffer.GetNumElements());
}

void Benchmark_StaticIndexBuffer::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertices = std::vector<Vertex>(numQuads * 4);

	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), numQuads * 4, ig::BufferUsage::Dynamic);
	indexBuffer.LoadAsIndexBuffer(context, ig::IndexFormat::UINT32, numQuads * 6, ig::BufferUsage::Default);

	// Populate the index buffer at initialization.
	std::vector<uint32_t> indices(numQuads * 6);
	for (uint32_t quadIndex = 0; quadIndex < numQuads; quadIndex++)
	{
		uint32_t baseVertex = quadIndex * 4;  // 4 vertices per quad
		uint32_t indexOffset = quadIndex * 6;  // 6 indices per quad

		indices[indexOffset] = baseVertex;
		indices[indexOffset + 1] = baseVertex + 1;
		indices[indexOffset + 2] = baseVertex + 2;
		indices[indexOffset + 3] = baseVertex + 2;
		indices[indexOffset + 4] = baseVertex + 1;
		indices[indexOffset + 5] = baseVertex + 3;
	}
	indexBuffer.SetData(cmd, indices.data());

	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION"),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR"),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_Triangles);
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_StaticIndexBuffer::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < numQuads; i++)
	{
		vertices[currentVertex] = { quad[i].x, quad[i].y, quad[i].color };
		vertices[currentVertex + 1] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertices[currentVertex + 2] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertices[currentVertex + 3] = { quad[i].x + quad[i].width, quad[i].y + quad[i].height, quad[i].color };
		currentVertex += 4;
	}
	vertexBuffer.SetDynamicData(vertices.data());

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetIndexBuffer(indexBuffer);
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.DrawIndexed(indexBuffer.GetNumElements());
}

void Benchmark_RawVertexPulling::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	rawBuffer.LoadAsRawBuffer(context, sizeof(Quad) * numQuads, ig::BufferUsage::Dynamic);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_RawRect); // Use a Raw Vertex Pulling shader
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_RawVertexPulling::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	rawBuffer.SetDynamicData((void*)quad);

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());
	pushConstants.rawOrStructuredBufferIndex = rawBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.Draw(numQuads * 6);
}

void Benchmark_StructuredVertexPulling::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	structuredBuffer.LoadAsStructuredBuffer(context, sizeof(Quad), numQuads, ig::BufferUsage::Dynamic);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_StructuredRect); // Use a Structured Vertex Pulling shader
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_StructuredVertexPulling::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	structuredBuffer.SetDynamicData((void*)quad);

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());
	pushConstants.rawOrStructuredBufferIndex = structuredBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.Draw(numQuads * 6);
}

void Benchmark_Instancing::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Quad), numQuads, ig::BufferUsage::Dynamic);

	// Use a per-instance vertex layout
	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION", 0, 0, ig::InputClass::PerInstance, 1),
		ig::VertexElement(ig::Format::FLOAT, "WIDTH", 0, 0, ig::InputClass::PerInstance, 1),
		ig::VertexElement(ig::Format::FLOAT, "HEIGHT", 0, 0,ig::InputClass::PerInstance, 1),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR", 0, 0, ig::InputClass::PerInstance, 1),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_InstancedRect); // Use an Instancing shader
	desc.primitiveTopology = ig::Primitive::TriangleStrip; // Use triangle strips
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_Instancing::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertexBuffer.SetDynamicData((void*)quad);

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleStrip);
	cmd.DrawInstanced(4, numQuads);
}

void Benchmark_GPUTriangles::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), numQuads * 6, ig::BufferUsage::Default);

	std::vector<Vertex> vertexData(numQuads * 6);
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < numQuads; i++)
	{
		vertexData[currentVertex] = { quad[i].x, quad[i].y, quad[i].color };
		vertexData[currentVertex + 1] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertexData[currentVertex + 2] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertexData[currentVertex + 3] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertexData[currentVertex + 4] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertexData[currentVertex + 5] = { quad[i].x + quad[i].width, quad[i].y + quad[i].height, quad[i].color };
		currentVertex += 6;
	}
	vertexBuffer.SetData(cmd, vertexData.data());

	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION"),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR"),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_Triangles);
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_GPUTriangles::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.Draw(vertexBuffer.GetNumElements());
}

void Benchmark_GPUIndexBuffer::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), numQuads * 6, ig::BufferUsage::Default);
	indexBuffer.LoadAsIndexBuffer(context, ig::IndexFormat::UINT32, numQuads * 6, ig::BufferUsage::Default);

	std::vector<Vertex> vertexData(numQuads * 6);
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < numQuads; i++)
	{
		vertexData[currentVertex] = { quad[i].x, quad[i].y, quad[i].color };
		vertexData[currentVertex + 1] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertexData[currentVertex + 2] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertexData[currentVertex + 3] = { quad[i].x, quad[i].y + quad[i].height, quad[i].color };
		vertexData[currentVertex + 4] = { quad[i].x + quad[i].width, quad[i].y, quad[i].color };
		vertexData[currentVertex + 5] = { quad[i].x + quad[i].width, quad[i].y + quad[i].height, quad[i].color };
		currentVertex += 6;
	}
	vertexBuffer.SetData(cmd, vertexData.data());

	std::vector<uint32_t> indices(numQuads * 6);
	for (uint32_t quadIndex = 0; quadIndex < numQuads; quadIndex++)
	{
		uint32_t baseVertex = quadIndex * 4;  // 4 vertices per quad
		uint32_t indexOffset = quadIndex * 6;  // 6 indices per quad

		indices[indexOffset] = baseVertex;
		indices[indexOffset + 1] = baseVertex + 1;
		indices[indexOffset + 2] = baseVertex + 2;
		indices[indexOffset + 3] = baseVertex + 2;
		indices[indexOffset + 4] = baseVertex + 1;
		indices[indexOffset + 5] = baseVertex + 3;
	}
	indexBuffer.SetData(cmd, indices.data());

	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION"),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR"),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_Triangles);
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_GPUIndexBuffer::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetIndexBuffer(indexBuffer);
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.Draw(vertexBuffer.GetNumElements());
}

void Benchmark_GPURaw::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	rawBuffer.LoadAsRawBuffer(context, sizeof(Quad) * numQuads, ig::BufferUsage::Default);
	rawBuffer.SetData(cmd, (void*)quad);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_RawRect); // Use a Raw Vertex Pulling shader
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_GPURaw::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());
	pushConstants.rawOrStructuredBufferIndex = rawBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.Draw(numQuads * 6);
}

void Benchmark_GPUStructured::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	structuredBuffer.LoadAsRawBuffer(context, sizeof(Quad) * numQuads, ig::BufferUsage::Default);
	structuredBuffer.SetData(cmd, (void*)quad);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_StructuredRect); // Use a Structured Vertex Pulling shader
	desc.primitiveTopology = ig::Primitive::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_GPUStructured::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());
	pushConstants.rawOrStructuredBufferIndex = structuredBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleList);
	cmd.Draw(numQuads * 6);
}

void Benchmark_GPUInstancing::Init(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Quad), numQuads, ig::BufferUsage::Default);
	vertexBuffer.SetData(cmd, (void*)quad);

	// Use a per-instance vertex layout
	const std::vector<ig::VertexElement> vertexLayout =
	{
		ig::VertexElement(ig::Format::FLOAT_FLOAT, "POSITION", 0, 0, ig::InputClass::PerInstance, 1),
		ig::VertexElement(ig::Format::FLOAT, "WIDTH", 0, 0, ig::InputClass::PerInstance, 1),
		ig::VertexElement(ig::Format::FLOAT, "HEIGHT", 0, 0,ig::InputClass::PerInstance, 1),
		ig::VertexElement(ig::Format::BYTE_BYTE_BYTE_BYTE, "COLOR", 0, 0, ig::InputClass::PerInstance, 1),
	};

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_InstancedRect); // Use an Instancing shader
	desc.primitiveTopology = ig::Primitive::TriangleStrip; // Use triangle strips
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_GPUInstancing::OnRender(const ig::IGLOContext& context, ig::CommandList& cmd, const Quad* quad, uint32_t numQuads)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)context.GetWidth(), (float)context.GetHeight());

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetPrimitiveTopology(ig::Primitive::TriangleStrip);
	cmd.DrawInstanced(4, numQuads);
}
