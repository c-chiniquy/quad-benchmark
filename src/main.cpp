
#include "iglo.h"
#include "iglo_main_loop.h"
#include <array>
#include "benchmarks.h"

// Agility SDK path and version
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 715; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

static volatile bool consoleCloseRequested = false;
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		consoleCloseRequested = true;
		return TRUE;
	}
	return FALSE;
}

class App
{
public:

	void Run()
	{
		if (context.Load(
			ig::WindowSettings
			{
				.title = "Quad rendering benchmark (D3D12)",
				.width = 640,
				.height = 480,
				.resizable = false,
				.centered = true,
				.bordersVisible = false,
			},
			ig::RenderSettings
			{
				ig::PresentMode::ImmediateWithTearing,
			}))
		{
			context.SetOnDeviceRemovedCallback(std::bind(&App::OnDeviceRemoved, this));
			mainloop.SetWindowMinimizedBehaviour(ig::MainLoop::WindowMinimizedBehaviour::None);

			mainloop.Run(context,
				std::bind(&App::Start, this),
				std::bind(&App::OnLoopExited, this),
				std::bind(&App::Draw, this),
				std::bind(&App::Update, this, std::placeholders::_1),
				std::bind(&App::FixedUpdate, this),
				std::bind(&App::OnEvent, this, std::placeholders::_1));
		}
	}

private:

	ig::IGLOContext context;
	ig::CommandList cmd;
	ig::MainLoop mainloop;

	static constexpr uint32_t numQuads = 1'000'000;
	static constexpr float quadSize = 2;
	static constexpr double secondsPerBenchmark = 5;

	std::vector<Quad> quads;
	std::vector<StructuredQuad> structuredQuads;
	std::vector<std::unique_ptr<Benchmark>> benchmarks;

	size_t currentBenchmark = 0;
	double appStartCooldown = 0.5;
	double benchmarkTick = 0;
	uint32_t numFrames = 0;
	bool benchmarkComplete = false;


	void StartBenchmark(size_t benchmarkIndex)
	{
		numFrames = 0;
		benchmarkTick = 0;
		currentBenchmark = benchmarkIndex;
		ig::Print(ig::ToString("Started benchmark ", currentBenchmark, "/", benchmarks.size() - 1,
			"  -  ", benchmarks.at(currentBenchmark)->GetName(), "\n"));
	}

	void Start()
	{
		ig::Print(ig::ToString("iglo v" IGLO_VERSION_STRING " " IGLO_GRAPHICS_API_STRING "\n"));
		ig::Print(ig::ToString(context.GetGraphicsSpecs().rendererName, " (", context.GetGraphicsSpecs().vendorName, ")\n\n"));

		cmd.Load(context, ig::CommandListType::Graphics);

		// Generate quads
		{
			ig::Print(ig::ToString("Generating ", numQuads, " quads..."));
			ig::Random::SetSeed(1);
			const float max_X = (float)context.GetWidth() - quadSize;
			const float max_Y = (float)context.GetHeight() - quadSize;
			quads = std::vector<Quad>(numQuads);
			structuredQuads = std::vector<StructuredQuad>(numQuads);
			for (uint32_t i = 0; i < numQuads; i++)
			{
				Quad& q = quads[i];
				q.x = ig::Random::NextFloat(0, max_X);
				q.y = ig::Random::NextFloat(0, max_Y);
				q.width = quadSize;
				q.height = quadSize;
				q.color = ig::Random::NextUInt32();

				StructuredQuad& s = structuredQuads[i];
				s.x = q.x;
				s.y = q.y;
				s.width = q.width;
				s.height = q.height;
				s.color = q.color;
#ifdef IGLO_VULKAN
				s.padding = 0;
#endif
			}
			ig::Print(" Done.\n\n");
		}

		// Initialize benchmarks
		{
			cmd.Begin();
			{
				BenchmarkParams params =
				{
					.quads = quads.data(),
					.structuredQuads = structuredQuads.data(),
					.numQuads = numQuads,
					.viewExtent = context.GetBackBufferExtent(),
				};

				benchmarks.clear();
				benchmarks.push_back(std::move(std::make_unique<Benchmark_Nothing>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_1DrawCall>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_BatchedTriangleList>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_DynamicIndexBuffer>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_StaticIndexBuffer>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_RawVertexPulling>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_StructuredVertexPulling>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_Instancing>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUTriangles>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUIndexBuffer>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_GPURaw>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUStructured>(context, cmd, params)));
				benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUInstancing>(context, cmd, params)));
			}
			cmd.End();
			context.WaitForCompletion(context.Submit(cmd));
		}

		StartBenchmark(0);
	}

	void OnLoopExited()
	{
		context.WaitForIdleDevice();
	}

	void Update(double elapsedSeconds)
	{
		if (consoleCloseRequested)
		{
			mainloop.Quit();
			return;
		}

		if (benchmarkComplete) return;

		if (appStartCooldown > 0)
		{
			appStartCooldown -= elapsedSeconds;
			return;
		}

		benchmarkTick += elapsedSeconds;
		numFrames++;

		if (benchmarkTick >= secondsPerBenchmark)
		{
			context.WaitForIdleDevice();

			ig::Print(ig::ToString("FPS: ", (double)numFrames / secondsPerBenchmark, "\n"));
			ig::Print("--------------------\n");

			size_t next = currentBenchmark + 1;
			if (next >= benchmarks.size())
			{
				ig::Print("Benchmark complete!\n"
					"You can now close this window.\n");
				benchmarkComplete = true;
				mainloop.EnableIdleMode(true);
			}
			else
			{
				StartBenchmark(next);
			}
		}
	}

	void FixedUpdate()
	{
	}

	void OnEvent(ig::Event e)
	{
		if (e.type == ig::EventType::CloseRequest)
		{
			mainloop.Quit();
			return;
		}
		else if (e.type == ig::EventType::KeyPress)
		{
			if (e.key == ig::Key::Escape)
			{
				mainloop.Quit();
				return;
			}
		}
	}

	void Draw()
	{
		if (benchmarkComplete) return;

		cmd.Begin();
		{
			cmd.AddTextureBarrier(context.GetBackBuffer(), ig::SimpleBarrier::Discard, ig::SimpleBarrier::RenderTarget);
			cmd.FlushBarriers();

			cmd.SetRenderTarget(&context.GetBackBuffer());
			cmd.SetViewport((float)context.GetWidth(), (float)context.GetHeight());
			cmd.SetScissorRectangle(context.GetWidth(), context.GetHeight());
			cmd.ClearColor(context.GetBackBuffer(), ig::Colors::Red);

			if (currentBenchmark >= benchmarks.size()) throw std::runtime_error("currentBenchmark out of bounds");
			benchmarks[currentBenchmark]->OnUpdate();
			benchmarks[currentBenchmark]->OnRender(cmd);

			cmd.AddTextureBarrier(context.GetBackBuffer(), ig::SimpleBarrier::RenderTarget, ig::SimpleBarrier::Present);
			cmd.FlushBarriers();
		}
		cmd.End();

		context.Submit(cmd);
		context.Present();
	}

	void OnDeviceRemoved()
	{
		ig::PopupMessage("Device removal detected! The app will now quit.", "Error", &context);
		mainloop.Quit();
	}

};

int main()
{
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	std::unique_ptr<App> app = std::make_unique<App>();
	app->Run();
	app = nullptr;
	return 0;
}
