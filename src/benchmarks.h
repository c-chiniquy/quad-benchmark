#pragma once

struct Vertex
{
	float x = 0;
	float y = 0;
	ig::Color32 color = ig::Colors::Black;
};

struct Quad
{
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	ig::Color32 color = ig::Colors::Black;
};

#ifdef IGLO_VULKAN
struct StructuredQuad
{
	float x = 0;
	float y = 0;
	float width = 0;
	float height = 0;
	ig::Color32 color = ig::Colors::Black;
	uint32_t padding = 0; // Vulkan requires a padding of 8 bytes for structured buffers
};
#else
using StructuredQuad = Quad;
#endif

struct PushConstants
{
	uint32_t rawOrStructuredBufferIndex = IGLO_UINT32_MAX;
	ig::Vector2 screenSize;
};

void UpdateQuadsCPU(ig::Extent2D viewExtent, Quad* quads_CPU, uint32_t numQuads);
void UpdateStructuredQuadsCPU(ig::Extent2D viewExtent, StructuredQuad* quads_CPU, uint32_t numQuads);

struct BenchmarkParams
{
	Quad* quads = nullptr;
	StructuredQuad* structuredQuads = nullptr;
	uint32_t numQuads = 0;
	ig::Extent2D viewExtent;
};

class Benchmark
{
public:
	Benchmark(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
	{
		this->context = &context;
		this->params = params;
	};
	virtual ~Benchmark() = default;

	virtual std::string GetName() const { return ""; };
	virtual void OnUpdate() {};
	virtual void OnRender(ig::CommandList&) {};

	const ig::IGLOContext* context = nullptr;
	BenchmarkParams params;
};

class Benchmark_Nothing : public Benchmark
{
public:
	Benchmark_Nothing(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params)
		: Benchmark(context, cmd, params) {};
	std::string GetName() const override { return "Baseline (no quads are rendered)"; }

	void OnUpdate() override {};
	void OnRender(ig::CommandList&) override {};

private:
};

class Benchmark_1DrawCall : public Benchmark
{
public:
	Benchmark_1DrawCall(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "One draw call per quad"; }

	void OnUpdate()
	{
		UpdateQuadsCPU(params.viewExtent, params.quads, params.numQuads);
	};
	void OnRender(ig::CommandList&) override;

private:
	ig::Pipeline pipeline;
};

class Benchmark_BatchedTriangleList : public Benchmark
{
public:
	Benchmark_BatchedTriangleList(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Batched Triangle List"; }

	void OnUpdate()
	{
		UpdateQuadsCPU(params.viewExtent, params.quads, params.numQuads);
	};
	void OnRender(ig::CommandList&) override;

private:
	std::vector<Vertex> vertices;
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_DynamicIndexBuffer : public Benchmark
{
public:
	Benchmark_DynamicIndexBuffer(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Dynamic Index Buffer"; }

	void OnUpdate()
	{
		UpdateQuadsCPU(params.viewExtent, params.quads, params.numQuads);
	};
	void OnRender(ig::CommandList&) override;

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	ig::Buffer vertexBuffer;
	ig::Buffer indexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_StaticIndexBuffer : public Benchmark
{
public:
	Benchmark_StaticIndexBuffer(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Static Index Buffer"; }

	void OnUpdate()
	{
		UpdateQuadsCPU(params.viewExtent, params.quads, params.numQuads);
	};
	void OnRender(ig::CommandList&) override;

private:
	std::vector<Vertex> vertices;
	ig::Buffer vertexBuffer;
	ig::Buffer indexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_RawVertexPulling : public Benchmark
{
public:
	Benchmark_RawVertexPulling(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Raw Vertex Pulling"; }

	void OnUpdate()
	{
		UpdateQuadsCPU(params.viewExtent, params.quads, params.numQuads);
	};
	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer rawBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_StructuredVertexPulling : public Benchmark
{
public:
	Benchmark_StructuredVertexPulling(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Structured Vertex Pulling"; }

	void OnUpdate()
	{
		UpdateStructuredQuadsCPU(params.viewExtent, params.structuredQuads, params.numQuads);
	};
	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer structuredBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_Instancing : public Benchmark
{
public:
	Benchmark_Instancing(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Instancing"; }

	void OnUpdate()
	{
		UpdateQuadsCPU(params.viewExtent, params.quads, params.numQuads);
	};
	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUTriangles : public Benchmark
{
public:
	Benchmark_GPUTriangles(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Rendering only (Batched Triangle List)"; }

	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUIndexBuffer : public Benchmark
{
public:
	Benchmark_GPUIndexBuffer(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Rendering only (Index Buffer)"; }

	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer vertexBuffer;
	ig::Buffer indexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPURaw : public Benchmark
{
public:
	Benchmark_GPURaw(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Rendering only (Raw Vertex Pulling)"; }

	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer rawBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUStructured : public Benchmark
{
public:
	Benchmark_GPUStructured(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Rendering only (Structured Vertex Pulling)"; }

	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer structuredBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUInstancing : public Benchmark
{
public:
	Benchmark_GPUInstancing(const ig::IGLOContext& context, ig::CommandList& cmd, const BenchmarkParams& params);
	std::string GetName() const override { return "Rendering only (Instancing)"; }

	void OnRender(ig::CommandList&) override;

private:
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};
