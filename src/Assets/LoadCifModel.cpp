#include "LoadCifModel.hpp"
#include "readcif.h"
#include "Utilities/Random.hpp"

#include <dsrpdb/PDB.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/hash.hpp>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>

using namespace glm;

namespace Assets {
#define MAX_CHAR_ATOM_NAME 4
#define MAX_CHAR_RES_NAME 4
#define MAX_CHAR_CHAIN_ID 4
#define MAX_CHAR_ENTITY_ID 4
	struct Atom
	{
		Atom() {
			clear();
		}
		void clear() {
			memset(atom_name, 0, MAX_CHAR_ATOM_NAME);
			memset(residue_name, 0, MAX_CHAR_RES_NAME);
			memset(chain_id, 0, MAX_CHAR_CHAIN_ID);
			memset(entity_id, 0, MAX_CHAR_ENTITY_ID);
		}
		char element;
		char atom_name[MAX_CHAR_ATOM_NAME];
		char residue_name[MAX_CHAR_RES_NAME];
		// this is the label_asym_id
		char chain_id[MAX_CHAR_CHAIN_ID];
		char entity_id[MAX_CHAR_ENTITY_ID];
		int residue_num;
		float x, y, z;
	};

	static const bool Required = true;


	std::vector<std::string> split(const std::string& s, char delimiter)
	{
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.push_back(token);
		}
		return tokens;
	}
	std::vector<std::string> parseAsymIdList(const std::string& value) {
		// comma separated list of ids.
		std::vector<std::string> splitted = split(value, ',');
		return splitted;
	}

	std::vector<std::vector<std::string>> parseOperatorList(const std::string& value) {
		// (a)
		// (1-5)
		// (1-5)(1-5)
		//
		// '(X0)(1-5)' becomes [['X0'], ['1', '2', '3', '4', '5']]
		// kudos to Glen van Ginkel.
		std::vector<std::string> groups;
		std::vector<std::vector<std::string>> ret;

		//std::regex reg("\\(? ([^ \\(\\)] + )\\) ? ] *", std::regex_constants::ECMAScript);
		//const oeRegex = / \(? ([^ \(\)] + )\) ? ] * / g;
		std::regex oeRegex("\\(?([^\\(\\)]+)\\)?",
			std::regex_constants::ECMAScript);
		std::smatch match;
		std::regex_match(value, match, oeRegex);
		if (!match.empty()) {
			for (unsigned i = 1; i < match.size(); ++i) {
				groups.push_back(match.str(i));
			}
		}

		for (auto g : groups) {
			std::vector<std::string> group;
			std::vector<std::string> splitted = split(g, ',');
			for (auto e : splitted) {
				std::string::size_type dashIndex = e.find("-", 0);
				if (dashIndex != std::string::npos) {
					// dash means we have integer ranges?
					std::string sfrom = e.substr(0, dashIndex);
					std::string sto = e.substr(dashIndex + 1);
					int from = std::stoi(sfrom);
					int to = std::stoi(sto);
					for (int i = from; i <= to; ++i) {
						group.push_back(std::to_string(i));
					}
				}
				else {
					group.push_back(e);
				}

			}
			ret.push_back(group);
		}

		return ret;
	}

#if 0
		let g : any;
		while (g = oeRegex.exec(value)) groups[groups.length] = g[1];

		groups.forEach(g = > {
			const group : string[] = [];
			g.split(',').forEach(e = > {
				const dashIndex = e.indexOf('-');
				if (dashIndex > 0) {
					const from = parseInt(e.substring(0, dashIndex)), to = parseInt(e.substr(dashIndex + 1));
					for (let i = from; i <= to; i++) group[group.length] = i.toString();
				}
	 else {
	  group[group.length] = e.trim();
  }
});
ret[ret.length] = group;
});

return ret;
	}

		function expandOperators(operatorList: string[][]) {
		const ops : string[][] = [];
		const currentOp : string[] = [];
		for (let i = 0; i < operatorList.length; i++) currentOp[i] = '';
		expandOperators1(operatorList, ops, operatorList.length - 1, currentOp);
		return ops;
	}

	function expandOperators1(operatorNames: string[][], list : string[][], i : number, current : string[]) {
		if (i < 0) {
			list[list.length] = current.slice(0);
			return;
		}

		let ops = operatorNames[i], len = ops.length;
		for (let j = 0; j < len; j++) {
			current[i] = ops[j];
			expandOperators1(operatorNames, list, i - 1, current);
		}
	}

	//[In]
	console.log(expandOperators(parseOperatorList("(X0)(1-4)")));
	//[Out]
	[[ "X0", "1" ], ["X0", "2"], ["X0", "3"], ["X0", "4"]]
#endif
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
			register_category("pdbx_struct_oper_list",
				[this]() {
					parse_transform();
				});
			register_category("pdbx_struct_assembly_gen",
				[this]() {
					parse_assemblies();
				});
		}

		void parse_assemblies() {
//			_pdbx_struct_assembly_gen.assembly_id
	//			_pdbx_struct_assembly_gen.oper_expression
		//		_pdbx_struct_assembly_gen.asym_id_list
			readcif::CIFFile::ParseValues pv;
			pv.reserve(3);
			std::string operExpr;
			std::string assemblyId;
			std::string asymIdList;
			pv.emplace_back(get_column("assembly_id", Required),
				[&assemblyId](const char* start, const char* end) {
					size_t count = end - start;
					assemblyId = std::string(start, count);
				});
			pv.emplace_back(get_column("oper_expression", Required),
				[&operExpr](const char* start, const char* end) {
					size_t count = end - start;
					operExpr = std::string(start, count);
				});
			pv.emplace_back(get_column("asym_id_list", Required),
				[&asymIdList](const char* start, const char* end) {
					size_t count = end - start;
					asymIdList = std::string(start, count);
				});

			while (parse_row(pv)) {
				std::vector<std::vector<std::string>> operators = parseOperatorList(operExpr);
				std::vector<std::string> asymIds = parseAsymIdList(asymIdList);

				if (!operators.empty()) {
					if (assemblies.find(assemblyId) != assemblies.end()) {
						// add to existing
						for (size_t i = 0; i < operators.size(); ++i) {
							for (auto asymId : asymIds) {
								assemblies[assemblyId].push_back({ {asymId, operators[i]} });
							}
						}
					}
					else {
						// first one
						assemblies[assemblyId] = { {{asymIds[0], operators[0]}}};
						for (size_t i = 1; i < asymIds.size(); ++i) {
							assemblies[assemblyId].push_back({ {asymIds[i], operators[0]}});
						}
						// add to existing
						for (size_t i = 1; i < operators.size(); ++i) {
							for (auto asymId : asymIds) {
								assemblies[assemblyId].push_back({ {asymId, operators[i]} });
							}
						}
					}

				}
				else {
					// ERROR???
				}
			}


		}
		void parse_transform()
		{
			readcif::CIFFile::ParseValues pv;
			pv.reserve(16);
			glm::mat4 m(1.0f);
			std::string id;
			pv.emplace_back(get_column("id", Required),
				[&id](const char* start, const char* end) {
					size_t count = end - start;
					id = std::string(start, count);
				});
			pv.emplace_back(get_column("matrix[1][1]", Required),
				[&m](const char* start) {
					m[0][0] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[1][2]", Required),
				[&m](const char* start) {
					m[1][0] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[1][3]", Required),
				[&m](const char* start) {
					m[2][0] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("vector[1]", Required),
				[&m](const char* start) {
					m[0][3] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[2][1]", Required),
				[&m](const char* start) {
					m[0][1] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[2][2]", Required),
				[&m](const char* start) {
					m[1][1] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[2][3]", Required),
				[&m](const char* start) {
					m[2][1] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("vector[2]", Required),
				[&m](const char* start) {
					m[1][3] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[3][1]", Required),
				[&m](const char* start) {
					m[0][2] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[3][2]", Required),
				[&m](const char* start) {
					m[1][2] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("matrix[3][3]", Required),
				[&m](const char* start) {
					m[2][2] = (float)readcif::str_to_float(start);
				});
			pv.emplace_back(get_column("vector[3]", Required),
				[&m](const char* start) {
					m[2][3] = (float)readcif::str_to_float(start);
				});

			while (parse_row(pv)) {
				transforms[id] = m;
				m = glm::mat4(1.0f);
			}

		}
		void parse_atom_site()
		{
			readcif::CIFFile::ParseValues pv;
			pv.reserve(11);
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
			pv.emplace_back(get_column("label_entity_id"),
				[&atom](const char* start, const char* end) {
					size_t count = end - start;
					if (count > MAX_CHAR_ENTITY_ID)
						count = MAX_CHAR_ENTITY_ID;
					strncpy(atom.entity_id, start, count);
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

		void build_entities() {
			for (const Atom& atom : atoms) {
				//danger this string conversion is super inefficient here.
				std::string entity_id = atom.entity_id;
				if (entities.find(entity_id) != entities.end()) {
					// add to existing
					entities[entity_id].push_back(&atom);
				}
				else {
					// first one
					entities[entity_id] = { &atom };
				}

				std::string chain_id = atom.chain_id;
				if (chains.find(chain_id) != chains.end()) {
					// add to existing
					chains[chain_id].push_back(&atom);
				}
				else {
					// first one
					chains[chain_id] = { &atom };
				}
			}
		}

		std::vector<Atom> atoms;
		std::unordered_map<std::string, std::vector<const Atom*>> entities;
		std::unordered_map<std::string, std::vector<const Atom*>> chains;
		std::unordered_map<std::string, glm::mat4> transforms;
		// each assembly has a list of asym units
		// and each asym unit has a list of ops
		using AsymUnit = std::unordered_map<std::string, std::vector<std::string>>;
		std::unordered_map<std::string, std::vector<AsymUnit>> assemblies;
	};

	void LoadCIFAsScene(const std::string& filename, std::vector<std::unique_ptr<Model>>& models, std::vector<ModelInstance>& modelInstances)
	{
		std::cout << "- loading '" << filename << "'... " << std::endl << std::flush;
		const auto timer = std::chrono::high_resolution_clock::now();
		const std::string materialPath = std::filesystem::path(filename).parent_path().string();

		ExtractCIF extract;

		try {
			extract.parse_file(filename.c_str());
			extract.build_entities();
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}

		std::vector<std::unique_ptr<Model>> newModels;
		size_t nEntities = extract.chains.size();
		if (nEntities > 0) {
			std::unordered_map<std::string, Model*> modelMap;
			for (auto e : extract.chains) {
				size_t n = e.second.size();
				std::cout << "Entity " << e.first << " has " << n << " atoms" << std::endl;
				std::vector<glm::vec3> vertices;
				std::vector<float> radii;
				for (size_t i = 0; i < n; ++i) {
					const Atom* a = e.second[i];
					vertices.push_back(glm::vec3(a->x, a->y, a->z));
					radii.push_back(1.0f);
				}
				auto model = Model::CreateSphereGroup(std::move(vertices), std::move(radii), Material::Lambertian(randomInBox(1.0, 1.0, 1.0) + glm::vec3(0.5, 0.5, 0.5)), true, filename + " :: " + e.first);
				modelMap[e.first] = model;
				newModels.push_back(std::unique_ptr<Model>(model));
			}

			// now look for instances as assembly.
			size_t nAssemblies = extract.assemblies.size();
			if (nAssemblies > 0) {
				// let's just use the first assembly
				auto assy = (extract.assemblies.begin()->second);
				for (auto& asymUnit: assy) {
					for (auto& opsEntry : asymUnit) {
						auto entityId = opsEntry.first;
						// first is entity id: read from modelmap
						//if not found then skip?
						auto iter = modelMap.find(entityId);
						if (iter != modelMap.end()) {
							Model* model = iter->second;
							// second is vector of operations
							// now look up transforms.
							auto opsList = opsEntry.second;
							for (auto& op : opsList) {
								// look up in list of ops
								auto xformiter = extract.transforms.find(op);
								if (xformiter == extract.transforms.end()) {
									// ERROR
								}
								else {
									glm::mat4 xform = xformiter->second;
									// now we can omit by clipping!!
									// todo: implement clipping post-load time for more dynamic updates elsewhere in code
									//if (xform[2][3] > 0) {
										modelInstances.push_back(ModelInstance(model, xformiter->second));
									//}
								}
							}
						}
					}
				}
			}
			else {
				// one instance per model entity
				for (auto& m : newModels) {
					modelInstances.push_back(ModelInstance(m.get()));
				}

			}

			// move contents of newmodels into models.
			// make sure models has room:
			//models.reserve(models.size() + newModels.size());
			std::move(newModels.begin(), newModels.end(), std::back_inserter(models));
		}
		else { // nEntities <= 0
			// one sphere per atom in the atoms list
			std::vector<glm::vec3> vertices;
			std::vector<float> radii;
			size_t n = extract.atoms.size();
			std::cout << n << " atoms\n";
			for (size_t i = 0; i < n; ++i) {
				Atom& a = extract.atoms[i];
				vertices.push_back(glm::vec3(a.x, a.y, a.z));
				radii.push_back(1.0f);
			}
			models.push_back(std::unique_ptr<Model>(Model::CreateSphereGroup(std::move(vertices), std::move(radii), Material::Lambertian(randomInBox(1.0,1.0,1.0) + glm::vec3(0.5, 0.5, 0.5)), true, filename)));
		}

		const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - timer).count();

		std::cout << elapsed << "s" << std::endl;

		//modelInstances.push_back(ModelInstance(models[models.size()-1].get()));
	}

	Model* LoadCIF(const std::string& filename, const Material& material, float scale) {
		std::cout << "- loading '" << filename << "'... " << std::flush;
		const auto timer = std::chrono::high_resolution_clock::now();
		const std::string materialPath = std::filesystem::path(filename).parent_path().string();

		std::vector<glm::vec3> vertices;
		std::vector<float> radii;

		ExtractCIF extract;

		try {
			extract.parse_file(filename.c_str());
			extract.build_entities();

		}
		catch (std::exception& e) {
			std::cerr << e.what() << '\n';
		}

		size_t n = extract.atoms.size();
		std::cout << n << " atoms\n";
		for (size_t i = 0; i < n; ++i) {
			Atom& a = extract.atoms[i];
			vertices.push_back(glm::vec3(a.x*scale, a.y*scale, a.z*scale));
			radii.push_back(0.25f);
		}

		const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - timer).count();

		std::cout << elapsed << "s" << std::endl;

		return Model::CreateSphereGroup(std::move(vertices), std::move(radii), material, true, filename);

	}

	Model* LoadPDB(const std::string& filename, const Material& material, float scale) {
		std::cout << "- loading '" << filename << "'... " << std::flush;
		const auto timer = std::chrono::high_resolution_clock::now();
		const std::string materialPath = std::filesystem::path(filename).parent_path().string();

		std::vector<glm::vec3> vertices;
		std::vector<float> radii;

		try{
		// create istream from filename
		std::ifstream ifs(filename);
		dsrpdb::PDB pdb(ifs);
		for (auto i = pdb.models_begin(); i != pdb.models_end(); ++i) {
			auto model = *i;
			for (auto j = 0; j != model.number_of_chains(); ++j) {
				auto chain = model.chain(j);
				
				for (auto k = chain.atoms_begin(); k != chain.atoms_end(); ++k) {
					auto atom = (*k).second;
					auto p = atom.cartesian_coords();
					vertices.push_back(glm::vec3(p.x() * scale, p.y() * scale, p.z() * scale));
					radii.push_back(0.25f);
				}
#if 0
				for (auto k = chain.residues_begin(); k != chain.residues_end(); ++k) {
					auto residue = *k;
					for (auto l = residue.atoms_begin(); l != residue.atoms_end(); ++l) {
						auto atom = *l;
						//std::cout << atom.name() << " " << atom.x() << " " << atom.y() << " " << atom.z() << std::endl;
						vertices.push_back(glm::vec3(atom.x() * scale, atom.y() * scale, atom.z() * scale));
						radii.push_back(0.25f);
					}
				}
#endif
			}
		}

		}
		catch (std::exception& e) {
			std::cerr << e.what() << '\n';
		}

		const auto elapsed = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - timer).count();

		std::cout << elapsed << "s" << std::endl;

		return Model::CreateSphereGroup(std::move(vertices), std::move(radii), material, true, filename);

	}
}
