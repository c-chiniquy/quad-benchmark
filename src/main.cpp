
#include "iglo.h"
#include "igloMainLoop.h"
#include <array>
#include "benchmarks.h"

// iglo callbacks
void Start();
void OnLoopExited();
void OnEvent(ig::Event e);
void Update(double elapsedTime);
void Draw();

ig::IGLOContext context;
ig::CommandList cmd;
ig::MainLoop mainloop;

// Agility SDK path and version
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 715; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

const uint32_t numQuads = 1'000'000;
std::vector<Quad> quads(numQuads);
const float quadSize = 2;

std::vector<std::unique_ptr<Benchmark>> benchmarks;
size_t currentBenchmark = 0;
constexpr double secondsPerBenchmark = 5;
double appStartCooldown = 0.5;
double benchmarkTick = 0;
uint32_t numFrames = 0;
bool benchmarkComplete = false;

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
	ig::Print(ig::ToString(ig::igloVersion, " ", context.GetGraphicsAPIShortName(), "\n"));
	ig::Print(ig::ToString(context.GetGraphicsAPIRenderer(), " (", context.GetGraphicsAPIVendor(), ")\n\n"));

	cmd.Load(context, ig::CommandListType::Graphics);

	// Generate quads
	{
		ig::Print(ig::ToString("Generating ", numQuads, " quads..."));
		ig::Random::SetSeed(1);
		const float max_X = (float)context.GetWidth() - quadSize;
		const float max_Y = (float)context.GetHeight() - quadSize;
		for (uint32_t i = 0; i < numQuads; i++)
		{
			quads[i].x = ig::Random::NextFloat(0, max_X);
			quads[i].y = ig::Random::NextFloat(0, max_Y);
			quads[i].width = quadSize;
			quads[i].height = quadSize;
			quads[i].color = ig::Random::NextUInt32();
		}
		ig::Print(" Done.\n\n");
	}

	// Initialize benchmarks
	{
		benchmarks.clear();
		benchmarks.push_back(std::move(std::make_unique<Benchmark_Nothing>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_1DrawCall>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_BatchedTriangleList>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_DynamicIndexBuffer>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_StaticIndexBuffer>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_RawVertexPulling>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_StructuredVertexPulling>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_Instancing>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUTriangles>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUIndexBuffer>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_GPURaw>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUStructured>()));
		benchmarks.push_back(std::move(std::make_unique<Benchmark_GPUInstancing>()));

		cmd.Begin();
		{
			for (size_t i = 0; i < benchmarks.size(); i++)
			{
				benchmarks.at(i)->Init(context, cmd, quads.data(), numQuads);
			}
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
		cmd.AddTextureBarrier(context.GetBackBuffer(), ig::SimpleBarrier::Common, ig::SimpleBarrier::RenderTarget, false);
		cmd.FlushBarriers();

		cmd.SetRenderTarget(&context.GetBackBuffer());
		cmd.SetViewport((float)context.GetWidth(), (float)context.GetHeight());
		cmd.SetScissorRectangle(context.GetWidth(), context.GetHeight());
		cmd.ClearColor(context.GetBackBuffer(), ig::Colors::Red);

		benchmarks.at(currentBenchmark)->OnRender(context, cmd, quads.data(), numQuads);

		cmd.AddTextureBarrier(context.GetBackBuffer(), ig::SimpleBarrier::RenderTarget, ig::SimpleBarrier::Common, false);
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

int main()
{
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

	if (context.Load(
		ig::WindowSettings("Quad rendering benchmark (D3D12)", 640, 480, false, true, false),
		ig::RenderSettings(ig::PresentMode::ImmediateWithTearing)))
	{
		context.SetOnDeviceRemovedCallback(OnDeviceRemoved);
		mainloop.SetWindowMinimizedBehaviour(ig::MainLoop::WindowMinimizedBehaviour::None);
		mainloop.Run(context, Start, OnLoopExited, Draw, Update, FixedUpdate, OnEvent);
	}
	return 0;
}
