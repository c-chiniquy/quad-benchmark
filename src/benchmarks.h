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

struct PushConstants
{
	uint32_t rawOrStructuredBufferIndex = IGLO_UINT32_MAX;
	ig::Vector2 screenSize;
};

class Benchmark
{
public:
	virtual ~Benchmark() = default;

	virtual std::string GetName() const { return ""; };
	virtual void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) {};
	virtual void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) {};
};

class Benchmark_Nothing : public Benchmark
{
public:
	std::string GetName() const override { return "Baseline (no quads are rendered)"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override {};
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override {};

private:
};

class Benchmark_1DrawCall : public Benchmark
{
public:
	std::string GetName() const override { return "One draw call per quad"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	ig::Pipeline pipeline;
};

class Benchmark_BatchedTriangleList : public Benchmark
{
public:
	std::string GetName() const override { return "Batched Triangle List"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	std::vector<Vertex> vertices;
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_DynamicIndexBuffer : public Benchmark
{
public:
	std::string GetName() const override { return "Dynamic Index Buffer"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

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
	std::string GetName() const override { return "Static Index Buffer"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	std::vector<Vertex> vertices;
	ig::Buffer vertexBuffer;
	ig::Buffer indexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_RawVertexPulling : public Benchmark
{
public:
	std::string GetName() const override { return "Raw Vertex Pulling"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	std::vector<Quad> quads;
	ig::Buffer rawBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_StructuredVertexPulling : public Benchmark
{
public:
	std::string GetName() const override { return "Structured Vertex Pulling"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	std::vector<Quad> quads;
	ig::Buffer structuredBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_Instancing : public Benchmark
{
public:
	std::string GetName() const override { return "Instancing"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	std::vector<Quad> quads;
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUTriangles : public Benchmark
{
public:
	std::string GetName() const override { return "GPU only (Batched Triangle List)"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUIndexBuffer : public Benchmark
{
public:
	std::string GetName() const override { return "GPU only (Index Buffer)"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	ig::Buffer vertexBuffer;
	ig::Buffer indexBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPURaw : public Benchmark
{
public:
	std::string GetName() const override { return "GPU only (Raw Vertex Pulling)"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	ig::Buffer rawBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUStructured : public Benchmark
{
public:
	std::string GetName() const override { return "GPU only (Structured Vertex Pulling)"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	ig::Buffer structuredBuffer;
	ig::Pipeline pipeline;
};

class Benchmark_GPUInstancing : public Benchmark
{
public:
	std::string GetName() const override { return "GPU only (Instancing)"; }

	void Init(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;
	void OnRender(const ig::IGLOContext&, ig::CommandList&, const Quad* src, uint32_t numQuads) override;

private:
	ig::Buffer vertexBuffer;
	ig::Pipeline pipeline;
};
