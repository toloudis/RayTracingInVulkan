#ifndef AICS_SIMULARIUM_FILE_READER_JSON_H
#define AICS_SIMULARIUM_FILE_READER_JSON_H

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include "Utilities/json.hpp"
#include "Simularium.hpp"

namespace aics {
namespace simularium {

    namespace fileio {

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


    } // namespace fileio

} // namespace simularium
} // namespace aics

#endif // AICS_SIMULARIUM_FILE_READER_JSON_H