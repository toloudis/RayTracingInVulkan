#include "SceneList.hpp"
#include "Assets/Material.hpp"
#include "Assets/Model.hpp"
#include "Assets/SimulariumJson.hpp"
#include "Assets/Sphere.hpp"
#include "Assets/Texture.hpp"
#include "Utilities/Random.hpp"
#include <functional>
#include <fstream>
#include <iostream>
#include <random>

using namespace glm;
using Assets::Material;
using Assets::Model;
using Assets::ModelInstance;
using Assets::Texture;

const std::vector<std::pair<std::string, std::function<SceneAssets (SceneList::CameraInitialSate&)>>> SceneList::AllScenes =
{
	{"Molecules", Molecules},
	{"Simularium", SimulariumTrajectory},
	{"Cube And Spheres", CubeAndSpheres},
	{"Ray Tracing In One Weekend", RayTracingInOneWeekend},
	{"Planets In One Weekend", PlanetsInOneWeekend},
	{"Lucy In One Weekend", LucyInOneWeekend},
	{"Cornell Box", CornellBox},
	{"Cornell Box & Lucy", CornellBoxLucy},
};

SceneAssets SceneList::CubeAndSpheres(CameraInitialSate& camera)
{
	// Basic test scene.
	
	camera.ModelView = translate(mat4(1), vec3(0, 0, -2));
	camera.FieldOfView = 90;
	camera.Aperture = 0.05f;
	camera.FocusDistance = 2.0f;
	camera.ControlSpeed = 2.0f;
	camera.GammaCorrection = false;
	camera.HasSky = true;

	std::vector<Model> models;
	std::vector<Texture> textures;

	models.push_back(Model::LoadModel("../assets/models/cube_multi.obj"));
	models.push_back(Model::CreateSphere(vec3(1, 0, 0), 0.5, Material::Metallic(vec3(0.7f, 0.5f, 0.8f), 0.2f), true));
	models.push_back(Model::CreateSphere(vec3(-1, 0, 0), 0.5, Material::Dielectric(1.5f), true));
	models.push_back(Model::CreateSphere(vec3(0, 1, 0), 0.5, Material::Lambertian(vec3(1.0f), 0), true));

	textures.push_back(Texture::LoadTexture("../assets/textures/land_ocean_ice_cloud_2048.png", Vulkan::SamplerConfig()));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (const Model& m : models) {
		modelInstances.push_back(ModelInstance(&m));
	}

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::move(textures));
}

SceneAssets SceneList::RayTracingInOneWeekend(CameraInitialSate& camera)
{
	// Final scene from Ray Tracing In One Weekend book.
	
	camera.ModelView = lookAt(vec3(13, 2, 3), vec3(0, 0, 0), vec3(0, 1, 0));
	camera.FieldOfView = 20;
	camera.Aperture = 0.1f;
	camera.FocusDistance = 10.0f;
	camera.ControlSpeed = 5.0f;
	camera.GammaCorrection = true;
	camera.HasSky = true;

	const bool isProc = true;

	std::mt19937 engine(42);
	auto random = std::bind(std::uniform_real_distribution<float>(), engine);

	std::vector<Model> models;

	models.push_back(Model::CreateSphere(vec3(0, -1000, 0), 1000, Material::Lambertian(vec3(0.5f, 0.5f, 0.5f)), isProc));

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			const float chooseMat = random();
			const vec3 center(a + 0.9f*random(), 0.2f, b + 0.9f*random());

			if (length(center - vec3(4, 0.2f, 0)) > 0.9)
			{
				if (chooseMat < 0.8f) // Diffuse
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Lambertian(vec3(
						random()*random(),
						random()*random(),
						random()*random())),
						isProc));
				}
				else if (chooseMat < 0.95f) // Metal
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Metallic(
						vec3(0.5f*(1 + random()), 0.5f*(1 + random()), 0.5f*(1 + random())),
						0.5f*random()),
						isProc));
				}
				else // Glass
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Dielectric(1.5f), isProc));
				}
			}
		}
	}

	models.push_back(Model::CreateSphere(vec3(0, 1, 0), 1.0f, Material::Dielectric(1.5f), isProc));
	models.push_back(Model::CreateSphere(vec3(-4, 1, 0), 1.0f, Material::Lambertian(vec3(0.4f, 0.2f, 0.1f)), isProc));
	models.push_back(Model::CreateSphere(vec3(4, 1, 0), 1.0f, Material::Metallic(vec3(0.7f, 0.6f, 0.5f), 0.0f), isProc));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (const Model& m : models) {
		modelInstances.push_back(ModelInstance(&m));
	}

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}


SceneAssets SceneList::PlanetsInOneWeekend(CameraInitialSate& camera)
{
	// Same as RayTracingInOneWeekend but using textures.
	
	camera.ModelView = lookAt(vec3(13, 2, 3), vec3(0, 0, 0), vec3(0, 1, 0));
	camera.FieldOfView = 20;
	camera.Aperture = 0.1f;
	camera.FocusDistance = 10.0f;
	camera.ControlSpeed = 5.0f;
	camera.GammaCorrection = true;
	camera.HasSky = true;

	const bool isProc = true;

	std::mt19937 engine(42);
	auto random = std::bind(std::uniform_real_distribution<float>(), engine);

	std::vector<Model> models;
	std::vector<Texture> textures;

	models.push_back(Model::CreateSphere(vec3(0, -1000, 0), 1000, Material::Lambertian(vec3(0.5f, 0.5f, 0.5f)), isProc));

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			const float chooseMat = random();
			const vec3 center(a + 0.9f * random(), 0.2f, b + 0.9f * random());

			if (length(center - vec3(4, 0.2f, 0)) > 0.9)
			{
				if (chooseMat < 0.8f) // Diffuse
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Lambertian(vec3(
						random() * random(),
						random() * random(),
						random() * random())),
						isProc));
				}
				else if (chooseMat < 0.95f) // Metal
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Metallic(
						vec3(0.5f * (1 + random()), 0.5f * (1 + random()), 0.5f * (1 + random())),
						0.5f * random()),
						isProc));
				}
				else // Glass
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Dielectric(1.5f), isProc));
				}
			}
		}
	}

	models.push_back(Model::CreateSphere(vec3(0, 1, 0), 1.0f, Material::Metallic(vec3(1.0f), 0.1f, 2), isProc));
	models.push_back(Model::CreateSphere(vec3(-4, 1, 0), 1.0f, Material::Lambertian(vec3(1.0f), 0), isProc));
	models.push_back(Model::CreateSphere(vec3(4, 1, 0), 1.0f, Material::Metallic(vec3(1.0f), 0.0f, 1), isProc));

	textures.push_back(Texture::LoadTexture("../assets/textures/2k_mars.jpg", Vulkan::SamplerConfig()));
	textures.push_back(Texture::LoadTexture("../assets/textures/2k_moon.jpg", Vulkan::SamplerConfig()));
	textures.push_back(Texture::LoadTexture("../assets/textures/land_ocean_ice_cloud_2048.png", Vulkan::SamplerConfig()));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (const Model& m : models) {
		modelInstances.push_back(ModelInstance(&m));
	}

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::move(textures));
}

SceneAssets SceneList::LucyInOneWeekend(CameraInitialSate& camera)
{
	// Same as RayTracingInOneWeekend but using the Lucy 3D model.
	
	camera.ModelView = lookAt(vec3(13, 2, 3), vec3(0, 1.0, 0), vec3(0, 1, 0));
	camera.FieldOfView = 20;
	camera.Aperture = 0.05f;
	camera.FocusDistance = 10.0f;
	camera.ControlSpeed = 5.0f;
	camera.GammaCorrection = true;
	camera.HasSky = true;

	const bool isProc = true;

	std::mt19937 engine(42);
	auto random = std::bind(std::uniform_real_distribution<float>(), engine);

	std::vector<Model> models;
	
	models.push_back(Model::CreateSphere(vec3(0, -1000, 0), 1000, Material::Lambertian(vec3(0.5f, 0.5f, 0.5f)), isProc));

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			const float chooseMat = random();
			const vec3 center(a + 0.9f*random(), 0.2f, b + 0.9f*random());

			if (length(center - vec3(4, 0.2f, 0)) > 0.9)
			{
				if (chooseMat < 0.8f) // Diffuse
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Lambertian(vec3(
						random()*random(),
						random()*random(),
						random()*random())),
						isProc));
				}
				else if (chooseMat < 0.95f) // Metal
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Metallic(
						vec3(0.5f*(1 + random()), 0.5f*(1 + random()), 0.5f*(1 + random())),
						0.5f*random()),
						isProc));
				}
				else // Glass
				{
					models.push_back(Model::CreateSphere(center, 0.2f, Material::Dielectric(1.5f), isProc));
				}
			}
		}
	}

	auto lucy0 = Model::LoadModel("../assets/models/lucy.obj");
	auto lucy1 = lucy0;
	auto lucy2 = lucy0;

	const auto i = mat4(1);
	const float scaleFactor = 0.0035f;

	lucy0.Transform(
		rotate(
			scale(
				translate(i, vec3(0, -0.08f, 0)), 
				vec3(scaleFactor)),
			radians(90.0f), vec3(0, 1, 0)));

	lucy1.Transform(
		rotate(
			scale(
				translate(i, vec3(-4, -0.08f, 0)),
				vec3(scaleFactor)),
			radians(90.0f), vec3(0, 1, 0)));

	lucy2.Transform(
		rotate(
			scale(
				translate(i, vec3(4, -0.08f, 0)),
				vec3(scaleFactor)),
			radians(90.0f), vec3(0, 1, 0)));

	lucy0.SetMaterial(Material::Dielectric(1.5f));
	lucy1.SetMaterial(Material::Lambertian(vec3(0.4f, 0.2f, 0.1f)));
	lucy2.SetMaterial(Material::Metallic(vec3(0.7f, 0.6f, 0.5f), 0.05f));

	models.push_back(std::move(lucy0));
	models.push_back(std::move(lucy1));
	models.push_back(std::move(lucy2));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (const Model& m : models) {
		modelInstances.push_back(ModelInstance(&m));
	}

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}

SceneAssets SceneList::CornellBox(CameraInitialSate& camera)
{
	camera.ModelView = lookAt(vec3(278, 278, 800), vec3(278, 278, 0), vec3(0, 1, 0));
	camera.FieldOfView = 40;
	camera.Aperture = 0.0f;
	camera.FocusDistance = 10.0f;
	camera.ControlSpeed = 500.0f;
	camera.GammaCorrection = true;
	camera.HasSky = false;

	const auto i = mat4(1);
	const auto white = Material::Lambertian(vec3(0.73f, 0.73f, 0.73f));

	auto box0 = Model::CreateBox(vec3(0, 0, -165), vec3(165, 165, 0), white);
	auto box1 = Model::CreateBox(vec3(0, 0, -165), vec3(165, 330, 0), white);

	box0.Transform(rotate(translate(i, vec3(555 - 130 - 165, 0, -65)), radians(-18.0f), vec3(0, 1, 0)));
	box1.Transform(rotate(translate(i, vec3(555 - 265 - 165, 0, -295)), radians(15.0f), vec3(0, 1, 0)));

	std::vector<Model> models;
	models.push_back(Model::CreateCornellBox(555));
	models.push_back(box0);
	models.push_back(box1);

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (const Model& m : models) {
		modelInstances.push_back(ModelInstance(&m));
	}

	return std::make_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}

SceneAssets SceneList::CornellBoxLucy(CameraInitialSate& camera)
{
	camera.ModelView = lookAt(vec3(278, 278, 800), vec3(278, 278, 0), vec3(0, 1, 0));
	camera.FieldOfView = 40;
	camera.Aperture = 0.0f;
	camera.FocusDistance = 10.0f;
	camera.ControlSpeed = 500.0f;
	camera.GammaCorrection = true;
	camera.HasSky = false;

	const auto i = mat4(1);
	const auto sphere = Model::CreateSphere(vec3(555 - 130, 165.0f, -165.0f / 2 - 65), 80.0f, Material::Dielectric(1.5f), true);
	auto lucy0 = Model::LoadModel("../assets/models/lucy.obj");

	lucy0.Transform(
		rotate(
			scale(
				translate(i, vec3(555 - 300 - 165/2, -9, -295 - 165/2)),
				vec3(0.6f)),
			radians(75.0f), vec3(0, 1, 0)));

	std::vector<Model> models;
	models.push_back(Model::CreateCornellBox(555));
	models.push_back(sphere);
	models.push_back(lucy0);

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (const Model& m : models) {
		modelInstances.push_back(ModelInstance(&m));
	}

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}

SceneAssets SceneList::SimulariumTrajectory(CameraInitialSate& camera) {
	// read a JSON file
	std::string filePath("C:\\Users\\danielt\\Downloads\\actin.h5.simularium");
	std::ifstream inputstream(filePath);
	nlohmann::json j;
	inputstream >> j;
	aics::simularium::fileio::SimulariumFileReader reader;
	aics::simularium::TrajectoryFrame trajectoryFrame;
	bool ok = reader.DeserializeFrame(
			j,
			0,
		trajectoryFrame);
	
	camera.ModelView = lookAt(vec3(0,0,150), vec3(0,0, 0), vec3(0, 1, 0));
	camera.FieldOfView = 40;
	camera.Aperture = 0.0f;
	camera.FocusDistance = 10.0f;
	camera.ControlSpeed = 500.0f;
	camera.GammaCorrection = true;
	camera.HasSky = false;

	const auto i = mat4(1);

	std::vector<Model> models;
	for (auto agent: trajectoryFrame.data) {
		// TODO:  allow a procedural model to have a transform?
		auto sphere = Model::CreateSphere(vec3(agent.x, agent.y, agent.z), agent.collision_radius, Material::Lambertian(vec3(1.0f,1.0f, 1.0f)), true);
//		sphere.Transform(translate(i, vec3(agent.x, agent.y, agent.z)));
		models.push_back(sphere);
	}

	auto domelight = Model::CreateSphere(vec3(0, 0, 0), 300.0, Material::DiffuseLight(vec3(0.5f, 0.5f, 0.5f)), true);
	models.push_back(domelight);
	
	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (const Model& m : models) {
		modelInstances.push_back(ModelInstance(&m));
	}

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}

SceneAssets SceneList::Molecules(CameraInitialSate& camera) {

	const auto identity = mat4(1);

	std::vector<Model> models;
	for (int ii = 0; ii < 4; ++ii) {
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\3jcl.cif", Material::Lambertian(glm::vec3(0.5, 0, 0))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\6vz8.cif", Material::Lambertian(glm::vec3(0.5, 0.5, 0))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7dzy.cif", Material::Lambertian(glm::vec3(0, 0.5, 0.5))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7kqe.cif", Material::Lambertian(glm::vec3(0.5, 0, 0.5))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7jjj.cif", Material::Lambertian(glm::vec3(0, 0.5, 0))));

		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\3jcl.cif", Material::Lambertian(glm::vec3(0.75, 0, 0))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\6vz8.cif", Material::Lambertian(glm::vec3(0.75, 0.75, 0))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7dzy.cif", Material::Lambertian(glm::vec3(0, 0.75, 0.75))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7kqe.cif", Material::Lambertian(glm::vec3(0.75, 0, 0.75))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7jjj.cif", Material::Lambertian(glm::vec3(0, 0.75, 0))));

		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\3jcl.cif", Material::Lambertian(glm::vec3(0, 0, 0.5))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\6vz8.cif", Material::Lambertian(glm::vec3(0, 0.5, 0.5))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7dzy.cif", Material::Lambertian(glm::vec3(0.5, 0.5, 0.5))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7kqe.cif", Material::Lambertian(glm::vec3(0.25, 0, 0.75))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\7jjj.cif", Material::Lambertian(glm::vec3(0.75, 0.5, 0.25))));
		models.push_back(Model::LoadCIF("C:\\Users\\danielt\\Downloads\\3jcl.cif", Material::Lambertian(glm::vec3(0.33, 0.33, 0))));
	}
#if 0
	const int nModels = 16;
	// randomly grown connected sphere cluster?
	const int nSpheres = 4000;
	for (int j = 0; j < nModels; ++j) {
		const float atomRadius = 2.0f;
		const float atomRadiusMax = 8.0f;
		models.push_back(Model::CreateRandomSphereGroup(nSpheres, 150.0, atomRadius, atomRadiusMax));
	}
#endif
	// now put many instances of each model into the world.
	// 
	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;

//	const int nInstancesPerModel = 2;
//	const float volumeSize = 200.0f;
	const int nInstancesPerModel = 104800;
	const float volumeSize = 50000.0f;
	size_t nSpheres = 0;
	for (const Model& m : models) {
		for (int k = 0; k < nInstancesPerModel; ++k) {
			nSpheres += m.Procedural()->NumBoundingBoxes();
			modelInstances.push_back(ModelInstance(&m, glm::transpose(glm::translate(identity, randomInBox(volumeSize, volumeSize, volumeSize)) * glm::rotate(identity, frand() * 3.14159265f, randomInSphere(1.0)))));
		}
		//break;
	}

	std::cout << "NSPHERES " << nSpheres << std::endl;

	auto domelight = Model::CreateSphere(vec3(0, 0, 0), volumeSize*1.7, Material::DiffuseLight(vec3(0.5f, 0.5f, 0.5f)), true);
	models.push_back(domelight);
	modelInstances.push_back(ModelInstance(&models[models.size()-1]));

	camera.FieldOfView = 40;
	camera.Aperture = 0.0f;
	camera.ControlSpeed = 500.0f;
	camera.GammaCorrection = true;
	camera.FocusDistance = volumeSize/2.0f;
	camera.ModelView = lookAt(vec3(0, 0, volumeSize*1.6), vec3(0, 0, 0), vec3(0, 1, 0));
	camera.HasSky = false;

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}
