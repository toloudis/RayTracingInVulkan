#ifdef _WIN32
#define NOMINMAX
#endif // _WIN32
#include "SceneList.hpp"
#include "Assets/AssetDownloader.hpp"
#include "Assets/LoadCifModel.hpp"
#include "Assets/Material.hpp"
#include "Assets/Model.hpp"
#include "Assets/SimulariumBinary.hpp"
#include "Assets/SimulariumJson.hpp"
#include "Assets/Sphere.hpp"
#include "Assets/Texture.hpp"
#include "Assets/threading.hpp"
#include "Utilities/Random.hpp"

#include <glm/gtx/euler_angles.hpp>

#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>


using namespace glm;
using Assets::Material;
using Assets::Model;
using Assets::ModelInstance;
using Assets::Texture;

namespace
{

	void AddRayTracingInOneWeekendCommonScene(std::vector<std::unique_ptr<Model>>& models, const bool& isProc, std::function<float ()>& random)
	{
		// Common models from the final scene from Ray Tracing In One Weekend book. Only the three central spheres are missing.
		// Calls to random() are always explicit and non-inlined to avoid C++ undefined evaluation order of function arguments,
		// this guarantees consistent and reproducible behaviour across different platforms and compilers.

		models.push_back(
			std::unique_ptr<Model>(
				Model::CreateSphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, Material::Lambertian(vec3(0.5f, 0.5f, 0.5f)), isProc)
			)
		);

		for (int i = -11; i < 11; ++i)
		{
			for (int j = -11; j < 11; ++j)
			{
				const float chooseMat = random();
				const float center_y = static_cast<float>(j) + 0.9f * random();
				const float center_x = static_cast<float>(i) + 0.9f * random();
				const vec3 center(center_x, 0.2f, center_y);

				if (length(center - vec3(4, 0.2f, 0)) > 0.9f)
				{
					if (chooseMat < 0.8f) // Diffuse
					{
						const float b = random() * random();
						const float g = random() * random();
						const float r = random() * random();

						models.push_back(std::unique_ptr<Model>(Model::CreateSphere(center, 0.2f, Material::Lambertian(vec3(r, g, b)), isProc)));
					}
					else if (chooseMat < 0.95f) // Metal
					{
						const float fuzziness = 0.5f * random();
						const float b = 0.5f * (1 + random());
						const float g = 0.5f * (1 + random());
						const float r = 0.5f * (1 + random());

						models.push_back(std::unique_ptr<Model>(Model::CreateSphere(center, 0.2f, Material::Metallic(vec3(r, g, b), fuzziness), isProc)));
					}
					else // Glass
					{
						models.push_back(std::unique_ptr<Model>(Model::CreateSphere(center, 0.2f, Material::Dielectric(1.5f), isProc)));
					}
				}
			}
		}
	}

}

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

	std::vector<std::unique_ptr<Model>> models;
	std::vector<Texture> textures;

	models.push_back(std::unique_ptr<Model>(Model::LoadModel("../assets/models/cube_multi.obj")));
	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(1, 0, 0), 0.5, Material::Metallic(vec3(0.7f, 0.5f, 0.8f), 0.2f), true)));
	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(-1, 0, 0), 0.5, Material::Dielectric(1.5f), true)));
	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(0, 1, 0), 0.5, Material::Lambertian(vec3(1.0f), 0), true)));

	textures.push_back(Texture::LoadTexture("../assets/textures/land_ocean_ice_cloud_2048.png", Vulkan::SamplerConfig()));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (auto& m : models) {
		modelInstances.push_back(ModelInstance(m.get()));
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
	std::function<float ()> random = std::bind(std::uniform_real_distribution<float>(), engine);

	std::vector<std::unique_ptr<Model>> models;

	AddRayTracingInOneWeekendCommonScene(models, isProc, random);

	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(0, 1, 0), 1.0f, Material::Dielectric(1.5f), isProc)));
	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(-4, 1, 0), 1.0f, Material::Lambertian(vec3(0.4f, 0.2f, 0.1f)), isProc)));
	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(4, 1, 0), 1.0f, Material::Metallic(vec3(0.7f, 0.6f, 0.5f), 0.0f), isProc)));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (auto& m : models) {
		modelInstances.push_back(ModelInstance(m.get()));
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
	std::function<float()> random = std::bind(std::uniform_real_distribution<float>(), engine);

	std::vector<std::unique_ptr<Model>> models;
	std::vector<Texture> textures;

	AddRayTracingInOneWeekendCommonScene(models, isProc, random);

	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(0, 1, 0), 1.0f, Material::Metallic(vec3(1.0f), 0.1f, 2), isProc)));
	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(-4, 1, 0), 1.0f, Material::Lambertian(vec3(1.0f), 0), isProc)));
	models.push_back(std::unique_ptr<Model>(Model::CreateSphere(vec3(4, 1, 0), 1.0f, Material::Metallic(vec3(1.0f), 0.0f, 1), isProc)));

	textures.push_back(Texture::LoadTexture("../assets/textures/2k_mars.jpg", Vulkan::SamplerConfig()));
	textures.push_back(Texture::LoadTexture("../assets/textures/2k_moon.jpg", Vulkan::SamplerConfig()));
	textures.push_back(Texture::LoadTexture("../assets/textures/land_ocean_ice_cloud_2048.png", Vulkan::SamplerConfig()));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (auto& m : models) {
		modelInstances.push_back(ModelInstance(m.get()));
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
	std::function<float()> random = std::bind(std::uniform_real_distribution<float>(), engine);

	std::vector<std::unique_ptr<Model>> models;

	AddRayTracingInOneWeekendCommonScene(models, isProc, random);

	auto lucy0 = Model::LoadModel("../assets/models/lucy.obj");
	// TODO use ModelInstances instead.
	auto lucy1 = new Model(*lucy0);
	auto lucy2 = new Model(*lucy0);

	const auto i = mat4(1);
	const float scaleFactor = 0.0035f;

	lucy0->Transform(
		rotate(
			scale(
				translate(i, vec3(0, -0.08f, 0)),
				vec3(scaleFactor)),
			radians(90.0f), vec3(0, 1, 0)));

	lucy1->Transform(
		rotate(
			scale(
				translate(i, vec3(-4, -0.08f, 0)),
				vec3(scaleFactor)),
			radians(90.0f), vec3(0, 1, 0)));

	lucy2->Transform(
		rotate(
			scale(
				translate(i, vec3(4, -0.08f, 0)),
				vec3(scaleFactor)),
			radians(90.0f), vec3(0, 1, 0)));

	lucy0->SetMaterial(Material::Dielectric(1.5f));
	lucy1->SetMaterial(Material::Lambertian(vec3(0.4f, 0.2f, 0.1f)));
	lucy2->SetMaterial(Material::Metallic(vec3(0.7f, 0.6f, 0.5f), 0.05f));

	models.push_back(std::unique_ptr<Model>(lucy0));
	models.push_back(std::unique_ptr<Model>(lucy1));
	models.push_back(std::unique_ptr<Model>(lucy2));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (auto& m : models) {
		modelInstances.push_back(ModelInstance(m.get()));
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

	box0->Transform(rotate(translate(i, vec3(555 - 130 - 165, 0, -65)), radians(-18.0f), vec3(0, 1, 0)));
	box1->Transform(rotate(translate(i, vec3(555 - 265 - 165, 0, -295)), radians(15.0f), vec3(0, 1, 0)));

	std::vector<std::unique_ptr<Model>> models;
	models.push_back(std::unique_ptr<Model>(Model::CreateCornellBox(555)));
	models.push_back(std::unique_ptr<Model>(box0));
	models.push_back(std::unique_ptr<Model>(box1));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (auto& m : models) {
		modelInstances.push_back(ModelInstance(m.get()));
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

	lucy0->Transform(
		rotate(
			scale(
				translate(i, vec3(555 - 300 - 165/2, -9, -295 - 165/2)),
				vec3(0.6f)),
			radians(75.0f), vec3(0, 1, 0)));

	std::vector<std::unique_ptr<Model>> models;
	models.push_back(std::unique_ptr<Model>(Model::CreateCornellBox(555)));
	models.push_back(std::unique_ptr<Model>(sphere));
	models.push_back(std::unique_ptr<Model>(lucy0));

	// create an instance for each model:
	std::vector<ModelInstance> modelInstances;
	for (auto& m : models) {
		modelInstances.push_back(ModelInstance(m.get()));
	}

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}

static std::vector<std::string> SplitWithCharacters(const std::string& str, int splitLength) {
	size_t numSubstrings = str.length() / splitLength;
	std::vector<std::string> ret;

	for (int i = 0; i < numSubstrings; i++) {
		ret.push_back(str.substr(i * splitLength, splitLength));
	}

	// If there are leftover characters, create a shorter item at the end.
	if (str.length() % splitLength != 0) {
		ret.push_back(str.substr(splitLength * numSubstrings));
	}

	return ret;
}

static void hex2rgb(std::string hex, std::array<float, 3>& out) {
	if (hex.at(0) == '#') {
		hex.erase(0, 1);
	}

	while (hex.length() < 6) {
		hex += "0";
	}

	std::vector<std::string> colori = SplitWithCharacters(hex, 2);

	out[0] = stoi(colori[0], nullptr, 16)/255.0f;
	out[1] = stoi(colori[1], nullptr, 16)/255.0f;
	out[2] = stoi(colori[2], nullptr, 16)/255.0f;
}

static aics::simularium::fileio::ISimulariumFile* GetReader(std::string path) {
	bool isBinary = aics::simularium::fileio::SimulariumFileReaderBinary::isBinarySimulariumFile(path);
	if (isBinary)
		return new aics::simularium::fileio::SimulariumFileReaderBinary(path);
	else
		return new aics::simularium::fileio::SimulariumFileReaderJson(path);
}

SceneAssets SceneList::SimulariumTrajectory(CameraInitialSate& camera) {
	// read a JSON file
	std::string fp2__("E:\\data\\readdy-new-self-ass.simularium");
	std::string fp2_("C:\\Users\\danielt\\Downloads\\actin.h5.simularium");
	// https://aics-simularium-data.s3.us-east-2.amazonaws.com/trajectory/json_v3/bloood-plasma-1.0.simularium
	std::string fp2("C:\\Users\\dmt\\Downloads\\bloood-plasma-1.0.simularium");
	aics::simularium::fileio::ISimulariumFile* reader = GetReader(fp2);
	
	aics::simularium::TrajectoryFileProperties tfp = reader->getTrajectoryFileInfo();

	camera.ModelView = lookAt(vec3(tfp.cameraDefault.position[0], tfp.cameraDefault.position[1], tfp.cameraDefault.position[2]),
		vec3(tfp.cameraDefault.lookAtPosition[0], tfp.cameraDefault.lookAtPosition[1], tfp.cameraDefault.lookAtPosition[2]),
		vec3(tfp.cameraDefault.upVector[0], tfp.cameraDefault.upVector[1], tfp.cameraDefault.upVector[2]));
	camera.FieldOfView = tfp.cameraDefault.fovDegrees;
	camera.Aperture = 0.0f;
	camera.FocusDistance = 10.0f;
	camera.ControlSpeed = 5.0f;
	camera.GammaCorrection = true;
	camera.HasSky = false;


	// get all the scene's geometries
	std::unordered_map<std::size_t, std::unique_ptr<Model>> modelLookup;
#if 0
	for (auto agentType : tfp.typeMapping) {
		aics::simularium::AgentType at = agentType.second;
		Assets::Model* m = nullptr;
		// parse color
		std::string color = at.geometry.color;
		std::array<float, 3> rgb;
		hex2rgb(color, rgb);
		if (at.geometry.displayType == "SPHERE") {
			m = Model::CreateSphere(
				vec3(0, 0, 0), 1.0, 
				Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])), 
				true, std::to_string(agentType.first));
		}
		else if (at.geometry.displayType == "OBJ") {
			// TODO download first
			m = Model::LoadModel(at.geometry.url);
		}
		else if (at.geometry.displayType == "PDB") {
			// TODO download first
			m = Assets::LoadCIF(at.geometry.url, Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])));
		}
		else {
			m = Model::CreateSphere(
				vec3(0, 0, 0), 1.0, 
				Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])), 
				true, std::to_string(agentType.first));
		}
		modelLookup[agentType.first] = std::unique_ptr<Model>(m);
	}
#endif

	// usage example:
	unsigned int min_cores = 1; // for the case when hardware_concurency fails and returns 0
	unsigned int number_of_cores = std::max(min_cores, std::min(6u, std::thread::hardware_concurrency() - 2u));
	std::mutex mutex;
	{
		std::vector<std::future<bool>> jobs;
		Tasks tasks;
		for (auto agentType : tfp.typeMapping) {
			jobs.push_back(tasks.queue([agentType, &modelLookup, &mutex]()->bool {
				aics::simularium::AgentType at = agentType.second;
				Assets::Model* m = nullptr;
				// parse color
				std::string color = at.geometry.color;
				std::array<float, 3> rgb;
				hex2rgb(color, rgb);
				AssetDownloader ad;
				if (at.geometry.displayType == "SPHERE") {
					m = Model::CreateSphere(
						vec3(0, 0, 0), 1.0,
						Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])),
						true, std::to_string(agentType.first));
				}
				else if (at.geometry.displayType == "OBJ") {
					// download first
					std::string tmpname = "temp_" + std::to_string(agentType.first) + ".obj";
					if (ad.download(at.geometry.url, tmpname, nullptr)) {
						m = Model::LoadModel(tmpname);
					}
					else {
						m = Model::CreateSphere(
							vec3(0, 0, 0), 1.0,
							Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])),
							true, std::to_string(agentType.first));
					}
				}
				else if (at.geometry.displayType == "PDB") {
					// download first
					std::string tmpname = "temp_" + std::to_string(agentType.first) + ".cif";

					std::string actualUrl = at.geometry.url;
					if (actualUrl.rfind("http", 0) == 0) {
						// assume this is a PDB ID to be loaded from the actual PDB
						// if not a valid ID, then download will fail.
						std::string pdbID = actualUrl;
						// prefer mmCIF first. If this fails, we will try .pdb.
						// TODO:
						// Can we confirm that the rcsb.org servers have every id as a cif file?
						// If so, then we don't need to do this second try and we can always use .cif.
						actualUrl = "https://files.rcsb.org/download/" + pdbID + "-assembly1.cif";
					}
					
					if (ad.download(actualUrl, tmpname, nullptr)) {
						std::cout << "Downloaded " << actualUrl << " to " << tmpname << std::endl;
						m = Assets::LoadCIF(tmpname, Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])));
					}
					else {
						m = Model::CreateSphere(
							vec3(0, 0, 0), 1.0,
							Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])),
							true, std::to_string(agentType.first));
					}
				}
				else {
					m = Model::CreateSphere(
						vec3(0, 0, 0), 1.0,
						Material::Lambertian(vec3(rgb[0], rgb[1], rgb[2])),
						true, std::to_string(agentType.first));
				}
				{
					std::lock_guard<std::mutex> lk(mutex);
					modelLookup[agentType.first] = std::unique_ptr<Model>(m);
				}
				return true;
			}));
		}
		tasks.start(number_of_cores);
		for_each(jobs.begin(), jobs.end(), [](auto& x) { x.get(); });
	}




	
	aics::simularium::TrajectoryFrame trajectoryFrame;
	reader->getFrame(tfp.totalSteps / 2 /*0*/, &trajectoryFrame);
//	bool ok = reader.DeserializeFrame(
//			j,
//			0,
//		trajectoryFrame);


	const auto i = mat4(1);

	std::vector<ModelInstance> modelInstances;
	for (auto agent: trajectoryFrame.data) {
		auto& m = modelLookup[(std::size_t)agent.type];

		auto identity = glm::mat4(1.0f); // construct identity matrix
		// scale geom by radius!
		auto scale = identity;// glm::scale(identity, glm::vec3(agent.collision_radius, agent.collision_radius, agent.collision_radius));
		// apply the matrix transformation to rotate
		auto rot = identity;// glm::eulerAngleXYZ(agent.xrot, agent.yrot, agent.zrot)* scale;
		// apply the matrix transformation to translate
		auto trans = glm::translate(rot, glm::vec3(agent.x, agent.y, agent.z));
		
		// create a mat4 with the transform data xrot, yrot, zrot, x,y,z
		//modelInstances.push_back(ModelInstance(m.get(), trans));
		modelInstances.push_back(ModelInstance(m.get(), glm::transpose(glm::translate(identity, glm::vec3(agent.x, agent.y, agent.z)) * glm::rotate(identity, frand() * 3.14159265f, randomInSphere(1.0)))));

	}

	std::vector<std::unique_ptr<Model>> models;
	// add one for the dome light we are about to make.
	models.reserve(1 + modelLookup.size());
	// done with modelLookup
	for (auto& m : modelLookup) {
		models.push_back(std::move(m.second));
	}
	modelLookup.clear();
	
	auto domelight = Model::CreateSphere(vec3(0, 0, 0), 300.0, Material::DiffuseLight(vec3(0.5f, 0.5f, 0.5f)), true);
	models.push_back(std::unique_ptr<Model>(domelight));

	modelInstances.push_back(ModelInstance(domelight));

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}

SceneAssets SceneList::Molecules(CameraInitialSate& camera) {
	const auto identity = mat4(1);

	std::vector<std::unique_ptr<Model>> models;
	//
// create an instance for each model:
	std::vector<ModelInstance> modelInstances;

	Assets::LoadCIFAsScene(
		//"C:\\Users\\dmt\\Downloads\\2plv.cif",
		"C:\\Users\\danielt\\Downloads\\cellpack_atom_instances_1189_curated\\cellpack_atom_instances_1189_curated.cif",
		models, modelInstances);

	//models.push_back(std::unique_ptr<Model>(Assets::LoadCIF("C:\\Users\\dmt\\Downloads\\cellpack_atom_instances_1189_curated\\cellpack_atom_instances_1189_curated.cif", Material::Lambertian(glm::vec3(0.5, 0, 0)))));

	for (int ii = 0; ii < 4; ++ii) {
		//models.push_back(std::unique_ptr<Model>(Assets::LoadCIF("C:\\Users\\dmt\\Downloads\\5wj1.cif", Material::Lambertian(glm::vec3(0.5, 0, 0)))));
		//models.push_back(std::unique_ptr<Model>(Assets::LoadCIF("C:\\Users\\dmt\\Downloads\\6vz8.cif", Material::Lambertian(glm::vec3(0.5, 0.5, 0)))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7dzy.cif", Material::Lambertian(glm::vec3(0, 0.5, 0.5))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7kqe.cif", Material::Lambertian(glm::vec3(0.5, 0, 0.5))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7jjj.cif", Material::Lambertian(glm::vec3(0, 0.5, 0))));

		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\3jcl.cif", Material::Lambertian(glm::vec3(0.75, 0, 0))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\6vz8.cif", Material::Lambertian(glm::vec3(0.75, 0.75, 0))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7dzy.cif", Material::Lambertian(glm::vec3(0, 0.75, 0.75))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7kqe.cif", Material::Lambertian(glm::vec3(0.75, 0, 0.75))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7jjj.cif", Material::Lambertian(glm::vec3(0, 0.75, 0))));

		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\3jcl.cif", Material::Lambertian(glm::vec3(0, 0, 0.5))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\6vz8.cif", Material::Lambertian(glm::vec3(0, 0.5, 0.5))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7dzy.cif", Material::Lambertian(glm::vec3(0.5, 0.5, 0.5))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7kqe.cif", Material::Lambertian(glm::vec3(0.25, 0, 0.75))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\7jjj.cif", Material::Lambertian(glm::vec3(0.75, 0.5, 0.25))));
		//models.push_back(Assets::LoadCIF("C:\\Users\\danielt\\Downloads\\3jcl.cif", Material::Lambertian(glm::vec3(0.33, 0.33, 0))));
	}
#if 0
	const int nModels = 16;
	// randomly grown connected sphere cluster?
	const int nSpheres = 4000;
	for (int j = 0; j < nModels; ++j) {
		const float atomRadius = 2.0f;
		const float atomRadiusMax = 8.0f;
		models.push_back(std::unique_ptr<Model>(Model::CreateRandomSphereGroup(nSpheres, 150.0, atomRadius, atomRadiusMax)));
	}
#endif
	// now put many instances of each model into the world.

//	const int nInstancesPerModel = 2;
//	const float volumeSize = 200.0f;
	const int nInstancesPerModel = 0;
	const float volumeSize = 5000.0f;
	for (auto& m : models) {
		for (int k = 0; k < nInstancesPerModel; ++k) {
			modelInstances.push_back(ModelInstance(m.get(), glm::transpose(glm::translate(identity, randomInBox(volumeSize, volumeSize, volumeSize)) * glm::rotate(identity, frand() * 3.14159265f, randomInSphere(1.0)))));
		}
		//break;
	}


	size_t nSpheres = 0;
	for (auto& inst : modelInstances) {
		nSpheres += inst.model_->Procedural()->NumBoundingBoxes();
	}
	std::cout << "NSPHERES " << nSpheres << std::endl;

	auto domelight = Model::CreateSphere(vec3(0, 0, 0), volumeSize*10, Material::DiffuseLight(vec3(0.5f, 0.5f, 0.5f)), true);
	models.push_back(std::unique_ptr<Model>(domelight));
	modelInstances.push_back(ModelInstance(domelight));

	camera.FieldOfView = 40;
	camera.Aperture = 0.0f;
	camera.ControlSpeed = 500.0f;
	camera.GammaCorrection = true;
	camera.FocusDistance = volumeSize/2.0f;
	camera.ModelView = lookAt(vec3(0, 0, volumeSize), vec3(0, 0, 0), vec3(0, 1, 0));
	camera.HasSky = false;

	return std::forward_as_tuple(std::move(modelInstances), std::move(models), std::vector<Texture>());
}
