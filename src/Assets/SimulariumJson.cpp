#include "SimulariumJson.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace aics {
namespace simularium {
    void TrajectoryFileProperties::fromJson(const nlohmann::json& fprops) {
        const nlohmann::json& typeMapping = fprops["typeMapping"];

        for (auto it = typeMapping.begin(); it != typeMapping.end(); ++it)
        {
            std::size_t idKey = std::atoi(it.key().c_str());
            const nlohmann::json& entry = it.value();
            this->typeMapping[idKey] = entry["name"].get<std::string>();
        }

        const nlohmann::json& size = fprops["size"];
        boxX = size["x"].get<float>();
        boxY = size["y"].get<float>();
        boxZ = size["z"].get<float>();

        fileName = fprops["fileName"].get<std::string>();
        numberOfFrames = fprops["totalSteps"].get<int>();
        timeStepSize = fprops["timeStepSize"].get<float>();
        spatialUnitFactorMeters = fprops["spatialUnitFactorMeters"].get<float>();

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
        size_t SimulariumFileReaderJson::getNumFrames() { return this->getTrajectoryFileInfo().numberOfFrames; }
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
