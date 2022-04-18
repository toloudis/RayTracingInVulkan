#pragma once

enum struct RendererType: uint32_t {
	Rasterizer,
	ProgressivePathTracer,
	SinglePassRayTracer,
};

struct UserSettings final
{
	// Application
	bool Benchmark;

	// Benchmark
	bool BenchmarkNextScenes{};
	uint32_t BenchmarkMaxTime{};

	// Scene
	int SceneIndex;

	// Renderer
	RendererType Renderer;
	bool AccumulateRays;
	uint32_t NumberOfSamples;
	uint32_t NumberOfBounces;
	uint32_t MaxNumberOfSamples;

	// Camera
	float FieldOfView;
	float Aperture;
	float FocusDistance;

	// Profiler
	bool ShowHeatmap;
	float HeatmapScale;

	// UI
	bool ShowSettings;
	bool ShowOverlay;

	inline const static float FieldOfViewMinValue = 10.0f;
	inline const static float FieldOfViewMaxValue = 90.0f;

	bool RequiresAccumulationReset(const UserSettings& prev) const
	{
		return
			Renderer != prev.Renderer ||
			AccumulateRays != prev.AccumulateRays ||
			NumberOfBounces != prev.NumberOfBounces ||
			FieldOfView != prev.FieldOfView ||
			Aperture != prev.Aperture ||
			FocusDistance != prev.FocusDistance;
	}
	RendererType CycleIncrementRenderer() {
		switch (Renderer)
		{
		case RendererType::Rasterizer:
			return RendererType::ProgressivePathTracer;
		case RendererType::ProgressivePathTracer:
			return RendererType::SinglePassRayTracer;
		case RendererType::SinglePassRayTracer:
			return RendererType::Rasterizer;
		default:
			return RendererType::Rasterizer;
		}
	}
};
