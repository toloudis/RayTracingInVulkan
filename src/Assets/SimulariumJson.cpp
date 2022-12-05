#include "SimulariumJson.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#if 0
{
    "version": 3,
        "timeUnits" : { "magnitude": 1.0, "name" : "ns" },
        "timeStepSize" : 0.1,
        "totalSteps" : 4001,
        "spatialUnits" : { "magnitude": 1.0, "name" : "nm" },
        "size" : { "x": 25.0, "y" : 25.0, "z" : 25.0 },
        "cameraDefault" : {
        "position": { "x": 0.0, "y" : 0.0, "z" : 120.0 },
            "lookAtPosition" : { "x": 0.0, "y" : 0.0, "z" : 0.0 },
            "upVector" : { "x": 0.0, "y" : 1.0, "z" : 0.0 },
            "fovDegrees" : 75.0
    },
        "typeMapping": {
            "0": {
                "name": "Head",
                    "geometry" : { "displayType": "SPHERE", "color" : "#94a7fc" }
            },
                "1" : {
                "name": "Tail",
                    "geometry" : { "displayType": "SPHERE", "color" : "#bf5736" }
            }
        }
}

#endif

namespace aics {
namespace simularium {
    static void getFloat3XYZ(const nlohmann::json& parent, std::string name, std::array<float, 3>& out) {
        if (parent.contains(name)) {
            const nlohmann::json& jvec = parent[name];
            out[0] = jvec["x"].get<float>();
            out[1] = jvec["y"].get<float>();
            out[2] = jvec["z"].get<float>();
        }
    }
	
    void TrajectoryFileProperties::fromJson(const nlohmann::json& fprops) {
        if (fprops.contains("typeMapping")) {
            const nlohmann::json& typeMapping = fprops["typeMapping"];

            for (auto it = typeMapping.begin(); it != typeMapping.end(); ++it)
            {
                std::size_t idKey = std::atoi(it.key().c_str());
                const nlohmann::json& entry = it.value();
                AgentType at;
                at.name = entry["name"].get<std::string>();
                if (entry.contains("geometry")) {
                    const nlohmann::json& geometry = entry["geometry"];
                    if (geometry.contains("displayType"))
                        at.geometry.displayType = geometry["displayType"].get<std::string>();
                    if (geometry.contains("url"))
                        at.geometry.url = geometry["url"].get<std::string>();
                    if (geometry.contains("color"))
                        at.geometry.color = geometry["color"].get<std::string>();
                }

                this->typeMapping[idKey] = at;
            }

        }

        if (fprops.contains("size")) {
            const nlohmann::json& jsize = fprops["size"];
            size[0] = jsize["x"].get<float>();
            size[1] = jsize["y"].get<float>();
            size[2] = jsize["z"].get<float>();
        }
        if (fprops.contains("cameraDefault")) {
			auto jCameraDefault = fprops["cameraDefault"];
            getFloat3XYZ(jCameraDefault, "position", cameraDefault.position);
            getFloat3XYZ(jCameraDefault, "lookAtPosition", cameraDefault.lookAtPosition);
            getFloat3XYZ(jCameraDefault, "upVector", cameraDefault.upVector);
            if (jCameraDefault.contains("fovDegrees")) {
				cameraDefault.fovDegrees = jCameraDefault["fovDegrees"].get<float>();
            }
        }

        //fileName = fprops["fileName"].get<std::string>();
        totalSteps = fprops["totalSteps"].get<int32_t>();
        timeStepSize = fprops["timeStepSize"].get<float>();
        if (fprops.contains("timeUnits")) {
            const nlohmann::json& jtimeUnits = fprops["timeUnits"];
            timeUnits.magnitude = jtimeUnits["magnitude"].get<float>();
            timeUnits.name = jtimeUnits["name"].get<std::string>();
        }
        if (fprops.contains("spatialUnits")) {
            const nlohmann::json& jspatialUnits = fprops["spatialUnits"];
            spatialUnits.magnitude = jspatialUnits["magnitude"].get<float>();
            spatialUnits.name = jspatialUnits["name"].get<std::string>();
        }

    }

    namespace fileio {

        SimulariumFileReaderJson::SimulariumFileReaderJson(std::string filePath) {
            std::ifstream inputstream(filePath);
            inputstream >> this->mJsonRoot;

        }
        TrajectoryFileProperties SimulariumFileReaderJson::getTrajectoryFileInfo() {
            nlohmann::json& fprops = mJsonRoot["trajectoryInfo"];
			
            TrajectoryFileProperties tfp;
            tfp.fromJson(fprops);

            return tfp;
        }
        // virtual std::vector<Plot> getPlotData() = 0;
        size_t SimulariumFileReaderJson::getNumFrames() { return this->getTrajectoryFileInfo().totalSteps; }
        void SimulariumFileReaderJson::getFrame(size_t theFrameNumber, TrajectoryFrame* frame) {
			DeserializeFrame(this->mJsonRoot, theFrameNumber, *frame);
        }

        bool SimulariumFileReaderJson::DeserializeFrame(
            nlohmann::json& jsonRoot,
            std::size_t frameNumber,
            TrajectoryFrame& outFrame)
        {
            AgentDataFrame adf;
            nlohmann::json& spatialData = jsonRoot["spatialData"];
            std::size_t start = spatialData["bundleStart"].get<std::size_t>();
            std::size_t size = spatialData["bundleSize"].get<std::size_t>();
            std::size_t frame = std::max(std::min(start + size, frameNumber), start);
            std::size_t index = (frame - start);

            nlohmann::json& bundleData = spatialData["bundleData"];
            nlohmann::json& frameJson = bundleData[index];
            nlohmann::json& frameData = frameJson["data"];

            for (std::size_t i = 0; i != frameData.size();) {
                AgentData ad;
                ad.vis_type = frameData[i++].get<float>();
                ad.id = frameData[i++].get<float>();
                ad.type = frameData[i++].get<float>();
                ad.x = frameData[i++].get<float>();
                ad.y = frameData[i++].get<float>();
                ad.z = frameData[i++].get<float>();
                ad.xrot = frameData[i++].get<float>();
                ad.yrot = frameData[i++].get<float>();
                ad.zrot = frameData[i++].get<float>();
                ad.collision_radius = frameData[i++].get<float>();

                std::size_t num_sp = frameData[i++].get<std::size_t>();
                if (num_sp + i > frameData.size()) {
                    //LOG_F(WARNING, "At index %i, %zu subpoints, %i entries in data buffer", i, num_sp, frameData.size());
                    //LOG_F(ERROR, "Parsing error, incorrect data packing or misalignment in simularium JSON");
                    return false;
                }

                for (std::size_t j = 0; j < num_sp; j++) {
                    ad.subpoints.push_back(frameData[i++].get<float>());
                }

                adf.push_back(ad);
            }

            outFrame.data = adf;
            outFrame.time = frameJson["time"].get<float>();
            outFrame.frameNumber = frameNumber;

            return true;
        }


			
    } // namespace fileio
} // namespace simularium
} // namespace aics
