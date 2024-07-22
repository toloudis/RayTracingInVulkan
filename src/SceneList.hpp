#pragma once
#include "Utilities/Glm.hpp"
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace Assets
{
	class Model;
	class ModelInstance;
	class Texture;
	class VolumeTexture;
}

typedef std::tuple<std::vector<Assets::ModelInstance>, std::vector<std::unique_ptr<Assets::Model>>, std::vector<Assets::Texture>, std::vector<Assets::VolumeTexture>> SceneAssets;

class SceneList final
{
public:

	struct CameraInitialSate
	{
		glm::mat4 ModelView;
		float FieldOfView;
		float Aperture;
		float FocusDistance;
		float ControlSpeed;
		bool GammaCorrection;
		bool HasSky;
	};

	static SceneAssets CubeAndSpheres(CameraInitialSate& camera);
	static SceneAssets RayTracingInOneWeekend(CameraInitialSate& camera);
	static SceneAssets PlanetsInOneWeekend(CameraInitialSate& camera);
	static SceneAssets LucyInOneWeekend(CameraInitialSate& camera);
	static SceneAssets CornellBox(CameraInitialSate& camera);
	static SceneAssets CornellBoxLucy(CameraInitialSate& camera);
	static SceneAssets SimulariumTrajectory(CameraInitialSate& camera);
	static SceneAssets Molecules(CameraInitialSate& camera);


	static const std::vector<std::pair<std::string, std::function<SceneAssets (CameraInitialSate&)>>> AllScenes;
};
