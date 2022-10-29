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
        void fromJson(const nlohmann::json& fprops);

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

            static bool isBinarySimulariumFile(std::string filePath);
        private:
            const enum BlockTypeEnum : uint32_t {
                // type = 0 : spatial data block in JSON
                SPATIAL_DATA_JSON = 0,
                // type = 1 : trajectory info block in JSON
                TRAJECTORY_INFO_JSON = 1,
                // type = 2 : plot data block in JSON
                PLOT_DATA_JSON = 2,
                // type = 3 : spatial data block in binary
                SPATIAL_DATA_BINARY = 3,
            };

            struct BlockInfo {
                BlockTypeEnum type;
                uint32_t offset;
                uint32_t size;
            };
            struct Header {
                uint32_t version;
                std::vector<BlockInfo> blocks;
            };
            
            std::string mFilePath;
            Header header;
            TrajectoryFileProperties tfi;
        uint32_t nFrames;
        std::vector<uint32_t> frameOffsets;
        std::vector<uint32_t> frameLengths;
        uint8_t* spatialDataBlock; // ideally this is really a Float32Array but alignment is not guaranteed yet

        Header readHeader();
        nlohmann::json parseJsonBlock(const BlockInfo& block);
        uint8_t* getBlockContent(const BlockInfo& block);
        uint8_t* getBlock(const BlockInfo& block);

        };

    } // namespace fileio

} // namespace simularium
} // namespace aics

#endif // AICS_SIMULARIUM_FILE_READER_H