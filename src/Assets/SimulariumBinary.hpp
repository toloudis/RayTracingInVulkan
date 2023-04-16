#ifndef AICS_SIMULARIUM_FILE_READER_BINARY_H
#define AICS_SIMULARIUM_FILE_READER_BINARY_H

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include "Utilities/json.hpp"
#include "Simularium.hpp"

namespace aics {
namespace simularium {

    namespace fileio {

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
            enum BlockTypeEnum : uint32_t {
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
        uint8_t* readSpatialDataInfo();
        AgentDataFrame parseAgentData(uint8_t* ptr, size_t size);

        };

    } // namespace fileio

} // namespace simularium
} // namespace aics

#endif // AICS_SIMULARIUM_FILE_READER_BINARY_H