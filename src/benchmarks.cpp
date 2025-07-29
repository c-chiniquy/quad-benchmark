#include "iglo.h"
#include "benchmarks.h"

#ifdef IGLO_D3D12
#include "shaders/PS_Color.h"
#include "shaders/VS_InstancedRect.h"
#include "shaders/VS_RawRect.h"
#include "shaders/VS_StructuredRect.h"
#include "shaders/VS_Triangles.h"
#else
#include "shaders/PS_Color_SPIRV.h"
#include "shaders/VS_InstancedRect_SPIRV.h"
#include "shaders/VS_RawRect_SPIRV.h"
#include "shaders/VS_StructuredRect_SPIRV.h"
#include "shaders/VS_Triangles_SPIRV.h"
#endif

#define SHADER_VS(a) ig::Shader(a, sizeof(a), "VSMain")
#define SHADER_PS(a) ig::Shader(a, sizeof(a), "PSMain")

void UpdateQuadsCPU(ig::Extent2D viewExtent, Quad* quads_CPU, uint32_t numQuads)
{
	// Move quads to the right
	for (uint32_t i = 0; i < numQuads; i++)
	{
		quads_CPU[i].x += 1.0f;
		if (quads_CPU[i].x > viewExtent.width) quads_CPU[i].x -= (float)viewExtent.width;
	}
}
void UpdateStructuredQuadsCPU(ig::Extent2D viewExtent, StructuredQuad* quads_CPU, uint32_t numQuads)
{
	// Move quads to the right
	for (uint32_t i = 0; i < numQuads; i++)
	{
		quads_CPU[i].x += 1.0f;
		if (quads_CPU[i].x > viewExtent.width) quads_CPU[i].x -= (float)viewExtent.width;
	}
}

Benchmark_1DrawCall::Benchmark_1DrawCall(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_1DrawCall::OnRender(ig::CommandList& cmd)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));

	for (uint32_t i = 0; i < params.numQuads; i++)
	{
		const Quad& q = params.quads[i];
		Vertex data[6] =
		{
			{q.x, q.y, q.color},
			{q.x + q.width, q.y, q.color},
			{q.x, q.y + q.height, q.color},
			{q.x, q.y + q.height, q.color},
			{q.x + q.width, q.y, q.color},
			{q.x + q.width, q.y + q.height, q.color},
		};
		cmd.SetTempVertexBuffer(&data, sizeof(data), sizeof(Vertex));
		cmd.Draw(6);
	}
}

Benchmark_BatchedTriangleList::Benchmark_BatchedTriangleList(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	vertices = std::vector<Vertex>(params.numQuads * 6);
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), params.numQuads * 6, ig::BufferUsage::Dynamic);

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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_BatchedTriangleList::OnRender(ig::CommandList& cmd)
{
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < params.numQuads; i++)
	{
		const Quad& q = params.quads[i];
		vertices[currentVertex] = { q.x, q.y, q.color };
		vertices[currentVertex + 1] = { q.x + q.width, q.y, q.color };
		vertices[currentVertex + 2] = { q.x, q.y + q.height, q.color };
		vertices[currentVertex + 3] = { q.x, q.y + q.height, q.color };
		vertices[currentVertex + 4] = { q.x + q.width, q.y, q.color };
		vertices[currentVertex + 5] = { q.x + q.width, q.y + q.height, q.color };
		currentVertex += 6;
	}
	vertexBuffer.SetDynamicData(vertices.data());

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.Draw(vertexBuffer.GetNumElements());
}

Benchmark_DynamicIndexBuffer::Benchmark_DynamicIndexBuffer(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	vertices = std::vector<Vertex>(params.numQuads * 4);
	indices = std::vector<uint32_t>(params.numQuads * 6);

	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), params.numQuads * 4, ig::BufferUsage::Dynamic);
	indexBuffer.LoadAsIndexBuffer(context, ig::IndexFormat::UINT32, params.numQuads * 6, ig::BufferUsage::Dynamic);

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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_DynamicIndexBuffer::OnRender(ig::CommandList& cmd)
{
	uint32_t currentVertex = 0;
	uint32_t currentIndex = 0;
	for (uint32_t i = 0; i < params.numQuads; i++)
	{
		const Quad& q = params.quads[i];
		vertices[currentVertex] = { q.x, q.y, q.color };
		vertices[currentVertex + 1] = { q.x + q.width, q.y, q.color };
		vertices[currentVertex + 2] = { q.x, q.y + q.height, q.color };
		vertices[currentVertex + 3] = { q.x + q.width, q.y + q.height, q.color };

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
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetIndexBuffer(indexBuffer);
	cmd.DrawIndexed(indexBuffer.GetNumElements());
}

Benchmark_StaticIndexBuffer::Benchmark_StaticIndexBuffer(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	vertices = std::vector<Vertex>(params.numQuads * 4);

	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), params.numQuads * 4, ig::BufferUsage::Dynamic);
	indexBuffer.LoadAsIndexBuffer(context, ig::IndexFormat::UINT32, params.numQuads * 6, ig::BufferUsage::Default);

	// Populate the index buffer at initialization
	std::vector<uint32_t> indices(params.numQuads * 6);
	for (uint32_t quadIndex = 0; quadIndex < params.numQuads; quadIndex++)
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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_StaticIndexBuffer::OnRender(ig::CommandList& cmd)
{
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < params.numQuads; i++)
	{
		const Quad& q = params.quads[i];
		vertices[currentVertex] = { q.x, q.y, q.color };
		vertices[currentVertex + 1] = { q.x + q.width, q.y, q.color };
		vertices[currentVertex + 2] = { q.x, q.y + q.height, q.color };
		vertices[currentVertex + 3] = { q.x + q.width, q.y + q.height, q.color };
		currentVertex += 4;
	}
	vertexBuffer.SetDynamicData(vertices.data());

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetIndexBuffer(indexBuffer);
	cmd.DrawIndexed(indexBuffer.GetNumElements());
}

Benchmark_RawVertexPulling::Benchmark_RawVertexPulling(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	rawBuffer.LoadAsRawBuffer(context, sizeof(Quad) * params.numQuads, ig::BufferUsage::Dynamic);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_RawRect); // Use a Raw Vertex Pulling shader
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_RawVertexPulling::OnRender(ig::CommandList& cmd)
{
	rawBuffer.SetDynamicData((void*)params.quads);

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);
	pushConstants.rawOrStructuredBufferIndex = rawBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.Draw(params.numQuads * 6);
}

Benchmark_StructuredVertexPulling::Benchmark_StructuredVertexPulling(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	structuredBuffer.LoadAsStructuredBuffer(context, sizeof(StructuredQuad), params.numQuads, ig::BufferUsage::Dynamic);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_StructuredRect); // Use a Structured Vertex Pulling shader
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_StructuredVertexPulling::OnRender(ig::CommandList& cmd)
{
	structuredBuffer.SetDynamicData((void*)params.structuredQuads);

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);
	pushConstants.rawOrStructuredBufferIndex = structuredBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.Draw(params.numQuads * 6);
}

Benchmark_Instancing::Benchmark_Instancing(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Quad), params.numQuads, ig::BufferUsage::Dynamic);

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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleStrip; // Use triangle strips
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_Instancing::OnRender(ig::CommandList& cmd)
{
	vertexBuffer.SetDynamicData((void*)params.quads);

	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.DrawInstanced(4, params.numQuads);
}

Benchmark_GPUTriangles::Benchmark_GPUTriangles(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), params.numQuads * 6, ig::BufferUsage::Default);

	std::vector<Vertex> vertexData(params.numQuads * 6);
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < params.numQuads; i++)
	{
		const Quad& q = params.quads[i];
		vertexData[currentVertex] = { q.x, q.y, q.color };
		vertexData[currentVertex + 1] = { q.x + q.width, q.y, q.color };
		vertexData[currentVertex + 2] = { q.x, q.y + q.height, q.color };
		vertexData[currentVertex + 3] = { q.x, q.y + q.height, q.color };
		vertexData[currentVertex + 4] = { q.x + q.width, q.y, q.color };
		vertexData[currentVertex + 5] = { q.x + q.width, q.y + q.height, q.color };
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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_GPUTriangles::OnRender(ig::CommandList& cmd)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.Draw(vertexBuffer.GetNumElements());
}

Benchmark_GPUIndexBuffer::Benchmark_GPUIndexBuffer(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Vertex), params.numQuads * 6, ig::BufferUsage::Default);
	indexBuffer.LoadAsIndexBuffer(context, ig::IndexFormat::UINT32, params.numQuads * 6, ig::BufferUsage::Default);

	std::vector<Vertex> vertexData(params.numQuads * 6);
	uint32_t currentVertex = 0;
	for (uint32_t i = 0; i < params.numQuads; i++)
	{
		const Quad& q = params.quads[i];
		vertexData[currentVertex] = { q.x, q.y, q.color };
		vertexData[currentVertex + 1] = { q.x + q.width, q.y, q.color };
		vertexData[currentVertex + 2] = { q.x, q.y + q.height, q.color };
		vertexData[currentVertex + 3] = { q.x, q.y + q.height, q.color };
		vertexData[currentVertex + 4] = { q.x + q.width, q.y, q.color };
		vertexData[currentVertex + 5] = { q.x + q.width, q.y + q.height, q.color };
		currentVertex += 6;
	}
	vertexBuffer.SetData(cmd, vertexData.data());

	std::vector<uint32_t> indices(params.numQuads * 6);
	for (uint32_t quadIndex = 0; quadIndex < params.numQuads; quadIndex++)
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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_GPUIndexBuffer::OnRender(ig::CommandList& cmd)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.SetIndexBuffer(indexBuffer);
	cmd.Draw(vertexBuffer.GetNumElements());
}

Benchmark_GPURaw::Benchmark_GPURaw(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	rawBuffer.LoadAsRawBuffer(context, sizeof(Quad) * params.numQuads, ig::BufferUsage::Default);
	rawBuffer.SetData(cmd, (void*)params.quads);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_RawRect); // Use a Raw Vertex Pulling shader
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_GPURaw::OnRender(ig::CommandList& cmd)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);
	pushConstants.rawOrStructuredBufferIndex = rawBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.Draw(params.numQuads * 6);
}

Benchmark_GPUStructured::Benchmark_GPUStructured(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	structuredBuffer.LoadAsRawBuffer(context, sizeof(StructuredQuad) * params.numQuads, ig::BufferUsage::Default);
	structuredBuffer.SetData(cmd, (void*)params.structuredQuads);

	ig::PipelineDesc desc;
	desc.blendStates = { ig::BlendDesc::BlendDisabled };
	desc.depthState = ig::DepthDesc::DepthDisabled;
	desc.rasterizerState = ig::RasterizerDesc::NoCull;
	desc.renderTargetDesc = context.GetBackBufferRenderTargetDesc();
	desc.PS = SHADER_PS(g_PS_Color);
	desc.VS = SHADER_VS(g_VS_StructuredRect); // Use a Structured Vertex Pulling shader
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleList;
	desc.vertexLayout = {}; // No vertex layout
	pipeline.Load(context, desc);
}

void Benchmark_GPUStructured::OnRender(ig::CommandList& cmd)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);
	pushConstants.rawOrStructuredBufferIndex = structuredBuffer.GetDescriptor()->heapIndex;

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.Draw(params.numQuads * 6);
}

Benchmark_GPUInstancing::Benchmark_GPUInstancing(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	: Benchmark(context, cmd, params)
{
	vertexBuffer.LoadAsVertexBuffer(context, sizeof(Quad), params.numQuads, ig::BufferUsage::Default);
	vertexBuffer.SetData(cmd, (void*)params.quads);

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
	desc.primitiveTopology = ig::PrimitiveTopology::TriangleStrip; // Use triangle strips
	desc.vertexLayout = vertexLayout;
	pipeline.Load(context, desc);
}

void Benchmark_GPUInstancing::OnRender(ig::CommandList& cmd)
{
	PushConstants pushConstants;
	pushConstants.screenSize = ig::Vector2((float)params.viewExtent.width, (float)params.viewExtent.height);

	cmd.SetPipeline(pipeline);
	cmd.SetPushConstants(&pushConstants, sizeof(pushConstants));
	cmd.SetVertexBuffer(vertexBuffer);
	cmd.DrawInstanced(4, params.numQuads);
}
