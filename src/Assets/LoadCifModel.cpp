#include "LoadCifModel.hpp"
#include "readcif.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>

using namespace glm;

namespace Assets {
#define MAX_CHAR_ATOM_NAME 4
#define MAX_CHAR_RES_NAME 4
#define MAX_CHAR_CHAIN_ID 4
	struct Atom
	{
		Atom() {
			clear();
		}
		void clear() {
			memset(atom_name, 0, MAX_CHAR_ATOM_NAME);
			memset(residue_name, 0, MAX_CHAR_RES_NAME);
			memset(chain_id, 0, MAX_CHAR_CHAIN_ID);
		}
		char element;
		char atom_name[MAX_CHAR_ATOM_NAME];
		char residue_name[MAX_CHAR_RES_NAME];
		char chain_id[MAX_CHAR_CHAIN_ID];
		int residue_num;
		float x, y, z;
	};

	static const bool Required = true;

	struct ExtractCIF : readcif::CIFFile {
		ExtractCIF()
		{
			register_heuristic_stylized_detection();
			// Personal preference, I like lambda functions better.
			// The lambda functions are needed because parse_XXXX
			// are member functions.
			register_category("atom_site",
				[this]() {
					parse_atom_site();
				});
		}
		void parse_atom_site()
		{
			readcif::CIFFile::ParseValues pv;
			pv.reserve(10);
			Atom atom;
			pv.emplace_back(get_column("type_symbol", Required),
				[&atom](const char* start) {
					atom.element = *start;
				});
			pv.emplace_back(get_column("label_atom_id", Required),
				[&atom](const char* start, const char* end) {
					size_t count = end - start;
					if (count > MAX_CHAR_ATOM_NAME)
						count = MAX_CHAR_ATOM_NAME;
					strncpy(atom.atom_name, start, count);
				});
			pv.emplace_back(get_column("label_comp_id", Required),
				[&atom](const char* start, const char* end) {
					size_t count = end - start;
					if (count > MAX_CHAR_RES_NAME)
						count = MAX_CHAR_RES_NAME;
					strncpy(atom.residue_name, start, count);
				});
			pv.emplace_back(get_column("label_asym_id"),
				[&atom](const char* start, const char* end) {
					size_t count = end - start;
					if (count > MAX_CHAR_CHAIN_ID)
						count = MAX_CHAR_CHAIN_ID;
					strncpy(atom.chain_id, start, count);
				});
			pv.emplace_back(get_column("label_seq_id", Required),
				[&atom](const char* start) {
					atom.residue_num = readcif::str_to_int(start);
				});
			// x, y, z are not required by mmCIF, but are by us
			pv.emplace_back(get_column("Cartn_x", Required),
				[&atom](const char* start) {
					atom.x = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("Cartn_y", Required),
				[&atom](const char* start) {
					atom.y = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("Cartn_z", Required),
				[&atom](const char* start) {
					atom.z = (float)readcif::str_to_float(start);
				});
			while (parse_row(pv)) {
				atoms.push_back(atom);
				atom.clear();
			}
		}

		std::vector<Atom> atoms;
	};

	void LoadCIFAsScene(const std::string& filename, std::vector<Model>& models, std::vector<ModelInstance>& modelInstances)
	{
		std::cout << "- loading '" << filename << "'... " << std::flush;
		const auto timer = std::chrono::high_resolution_clock::now();
		const std::string materialPath = std::filesystem::path(filename).parent_path().string();

		std::vector<glm::vec3> vertices;
		std::vector<float> radii;

		ExtractCIF extract;

		try {
			extract.parse_file(filename.c_str());
		}
		catch (std::exception& e) {
			std::cerr << e.what() << '\n';
		}

		size_t n = extract.atoms.size();
		std::cout << n << " atoms\n";
		for (size_t i = 0; i < n; ++i) {
			Atom& a = extract.atoms[i];
			vertices.push_back(glm::vec3(a.x, a.y, a.z));
			radii.push_back(1.0f);
		}

		const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - timer).count();

		std::cout << elapsed << "s" << std::endl;

		models.push_back(Model::CreateSphereGroup(std::move(vertices), std::move(radii), Material::Lambertian(glm::vec3(0.75, 0, 0.75)), true, filename));
		modelInstances.push_back(ModelInstance(&models[models.size()-1]));
	}

	Model LoadCIF(const std::string& filename, const Material& material) {
		std::cout << "- loading '" << filename << "'... " << std::flush;
		const auto timer = std::chrono::high_resolution_clock::now();
		const std::string materialPath = std::filesystem::path(filename).parent_path().string();

		std::vector<glm::vec3> vertices;
		std::vector<float> radii;

		ExtractCIF extract;

		try {
			extract.parse_file(filename.c_str());
		}
		catch (std::exception& e) {
			std::cerr << e.what() << '\n';
		}

		size_t n = extract.atoms.size();
		std::cout << n << " atoms\n";
		for (size_t i = 0; i < n; ++i) {
			Atom& a = extract.atoms[i];
			vertices.push_back(glm::vec3(a.x, a.y, a.z));
			radii.push_back(1.0f);
		}

		const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - timer).count();

		std::cout << elapsed << "s" << std::endl;

		return Model::CreateSphereGroup(std::move(vertices), std::move(radii), material, true, filename);

	}
}
