#ifndef AICS_SIMULARIUM_FILE_READER_H
#define AICS_SIMULARIUM_FILE_READER_H

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include "Utilities/json.hpp"

namespace aics {
namespace simularium {
    struct AgentData;

    typedef std::vector<aics::simularium::AgentData> AgentDataFrame;

    struct TrajectoryFrame {
        AgentDataFrame data;
        float time;
        std::size_t frameNumber;
    };

    struct AgentData {
        float vis_type = 0;
        float id = 0;
        float type = 0;
        float x = 0;
        float y = 0;
        float z = 0;
        float xrot = 0;
        float yrot = 0;
        float zrot = 0;
        float collision_radius = 0;
        std::vector<float> subpoints;
    };

    //Json::Value Serialize(AgentDataFrame& adf);
    //std::vector<float> Serialize(AgentData& ad);


    struct CameraPosition {
        CameraPosition()
        {
            position = { 0, 0, 120 };
            lookAtPoint = { 0, 0, 0 };
            upVector = { 0, 1, 0 };
            fovDegrees = 50;
        }

        std::array<float, 3> position;
        std::array<float, 3> lookAtPoint;
        std::array<float, 3> upVector;
        float fovDegrees;
    };

    struct TrajectoryFileProperties {
        std::string fileName = "";
        std::size_t numberOfFrames = 0;
        double timeStepSize = 100;
        float spatialUnitFactorMeters = 1e-9f;
        std::unordered_map<std::size_t, std::string> typeMapping;
        float boxX, boxY, boxZ;
        CameraPosition cameraDefault;

        std::string Str()
        {
            return "TrajectoryFileProperties | File Name " + this->fileName + " | Number of Frames " + std::to_string(this->numberOfFrames) + " | TimeStep Size " + std::to_string(this->timeStepSize) + " | spatialUnitFactor " + std::to_string(this->spatialUnitFactorMeters) + " | Box Size [" + std::to_string(boxX) + "," + std::to_string(boxY) + "," + std::to_string(boxZ) + "]";
        }
    };

    namespace fileio {

        class ISimulariumFile {
            virtual TrajectoryFileProperties getTrajectoryFileInfo() = 0;
            // virtual std::vector<Plot> getPlotData() = 0;
            virtual size_t getNumFrames() = 0;
            //virtual size_t getFrameIndexAtTime(float time) = 0;
            virtual void getFrame(size_t theFrameNumber, TrajectoryFrame* frame) = 0;
        };

		
        class SimulariumFileReaderJson : public ISimulariumFile {
        public:
            SimulariumFileReaderJson(std::string filePath);

            bool DeserializeFrame(
                nlohmann::json& jsonRoot,
                std::size_t frameNumber,
                TrajectoryFrame& outFrame);


            virtual TrajectoryFileProperties getTrajectoryFileInfo();
            // virtual std::vector<Plot> getPlotData() = 0;
            virtual size_t getNumFrames();
            //virtual size_t getFrameIndexAtTime(float time);
            virtual void getFrame(size_t theFrameNumber, TrajectoryFrame* frame);
        private:
            nlohmann::json mJsonRoot;

        };

        class SimulariumFileReaderBinary : public ISimulariumFile {
        public:
            SimulariumFileReaderBinary(std::string filePath);

            virtual TrajectoryFileProperties getTrajectoryFileInfo();
            // virtual std::vector<Plot> getPlotData() = 0;
            virtual size_t getNumFrames();
            //virtual size_t getFrameIndexAtTime(float time);
            virtual void getFrame(size_t theFrameNumber, TrajectoryFrame* frame);
        private:

        };

    } // namespace fileio

} // namespace simularium
} // namespace aics

#endif // AICS_SIMULARIUM_FILE_READER_H