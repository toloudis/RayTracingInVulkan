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

        SimulariumFileReaderBinary::SimulariumFileReaderBinary(std::string filePath) {
            mFilePath = filePath;

            nFrames = 0;
            //frameOffsets = [];
            //frameLengths = [];
            header = readHeader();
            //tfi = readTrajectoryFileInfo();
            //plotData = readPlotData();
            spatialDataBlock = readSpatialDataInfo();
        }

        TrajectoryFileProperties SimulariumFileReaderBinary::getTrajectoryFileInfo() { 
            // find the first block that is a trajectory info block
            for (auto block: this->header.blocks) {
                if (block.type == BlockTypeEnum::TRAJECTORY_INFO_JSON) {
                    nlohmann::json json = this->parseJsonBlock(block);
                    TrajectoryFileProperties tfp;
                    tfp.fromJson(json);
                    return tfp;
                }
            }
            throw ("No trajectory info block found");
        }
		
        // virtual std::vector<Plot> getPlotData() = 0;
        size_t SimulariumFileReaderBinary::getNumFrames() { return 0; }
        //virtual size_t getFrameIndexAtTime(float time);
        void SimulariumFileReaderBinary::getFrame(size_t theFrameNumber, TrajectoryFrame* frame) {
                auto frameOffset = this->frameOffsets[theFrameNumber];
                auto frameSize = this->frameLengths[theFrameNumber];
				
				// beginning of frame is here:
                uint8_t* frameData = this->spatialDataBlock + frameOffset;
                // fill in TrajectoryFrame from this data:
                frame->time = ((float*)(frameData))[1];
				frame->frameNumber =(size_t) ((float*)(frameData))[0];
				frame->data = this->parseAgentData(frameData + 8, frameSize - 8);
        }
		
        AgentDataFrame SimulariumFileReaderBinary::parseAgentData(uint8_t* ptr, size_t size) {
            AgentDataFrame af;
            size_t offset = 0;
            for (int i = 0; i < size; ++i) {
                AgentData ad;
                ad.vis_type = ((float*)(ptr))[offset++];
                ad.id = ((float*)(ptr))[offset++];
                ad.type = ((float*)(ptr))[offset++];
                ad.x = ((float*)(ptr))[offset++];
                ad.y = ((float*)(ptr))[offset++];
                ad.z = ((float*)(ptr))[offset++];
                ad.xrot = ((float*)(ptr))[offset++];
                ad.yrot = ((float*)(ptr))[offset++];
                ad.zrot = ((float*)(ptr))[offset++];
                ad.collision_radius = ((float*)(ptr))[offset++];
                uint32_t num_subpoints = ((uint32_t*)(ptr))[offset++];
                if (num_subpoints > 0) {
                    ad.subpoints = std::vector<float>(num_subpoints);
                    for (uint32_t i = 0; i < num_subpoints; i++) {
                        ad.subpoints[i] = ((float*)(ptr))[offset++];
                    }
                }
                af.push_back(ad);
            }
			return af;

        }

        const std::string SIGNATURE = "SIMULARIUMBINARY";
        // each block has a type and a size
        const uint32_t BLOCK_HEADER_SIZE = 8;

        bool SimulariumFileReaderBinary::isBinarySimulariumFile(std::string filePath) {
            std::fstream f1;
            uint8_t signature[16];

            f1.open(filePath, std::ios::in | std::ios::binary);
            f1.read((char*)signature, 16);
			std::string s((char*)signature, 16);
			f1.close();

			return s == SIGNATURE;
		}

        uint8_t* SimulariumFileReaderBinary::readSpatialDataInfo() {
            std::fstream f1;
            f1.open(mFilePath, std::ios::in | std::ios::binary);
            // find spatial data block and load frame offsets
            for (auto block : this->header.blocks) {
                if (block.type == BlockTypeEnum::SPATIAL_DATA_BINARY) {
                    uint8_t* blockData = this->getBlock(block);
                    size_t byteOffset = 8; // advance past block header.
                    // Spatial data version (4-byte int)
                    // const version = blockData.getUint32(byteOffset, true);
                    byteOffset += 4;
                    // Number of frames (4-byte int)
                    this->nFrames = ((uint32_t*)(blockData))[byteOffset/4];// blockData.getUint32(byteOffset, true);
                    byteOffset += 4;

                    // Frame offsets,sizes (Number of frames * two 4-byte ints)
                    for (uint32_t i = 0; i < this->nFrames; i++) {
                        this->frameOffsets.push_back(
                           ((uint32_t*)(blockData))[byteOffset / 4]
                        );
                        byteOffset += 4;
                        this->frameLengths.push_back(
                            ((uint32_t*)(blockData))[byteOffset / 4]
                        );
                        byteOffset += 4;
                    }
                    return blockData;
                }
            }
            throw ("No spatial data block found");
        }

		
        SimulariumFileReaderBinary::Header SimulariumFileReaderBinary::readHeader() {
            std::fstream is(mFilePath, std::ios::in | std::ios::binary);
            // get length of file:
            is.seekg(0, is.end);
            size_t length = is.tellg();
			// start after signature
            is.seekg(SIGNATURE.size(), is.beg);
            uint32_t headerLength = 0;
            is.read((char*)&headerLength, 4);
            uint32_t headerVersion = 0;
            is.read((char*)&headerVersion, 4);
            uint32_t nBlocks = 0;
            is.read((char*)&nBlocks, 4);
            if (nBlocks < 1) {
                throw ("No blocks found in file");
            }

            std::vector<BlockInfo> blocks(nBlocks);
            // the number of 32-bit ints after the SIGNATURE and before the toc
            const int OFFSET_TO_TABLE_OF_CONTENTS = 3;
            for (uint32_t i = 0; i < nBlocks; i++) {
                BlockInfo bi;
				is.read((char*)&bi.offset, 4);
				is.read((char*)&bi.type, 4);
				is.read((char*)&bi.size, 4);
                blocks.push_back(bi);
            }
            if (blocks[0].offset != headerLength) {
                throw ("First block offset does not match header length");
            }
            return {
                headerVersion,
                blocks,
            };
        }

        uint8_t* SimulariumFileReaderBinary::getBlockContent(const SimulariumFileReaderBinary::BlockInfo& block) {
            std::fstream is(mFilePath, std::ios::in | std::ios::binary);

            // return the block portion after the block header
            // first validate the block with what we expect.

            // TAKE NOTE OF ENDIANNESS.
            // Data transferred via HTTP is generally big endian.
            // Local file should have been written as little endian.
            // All use of DataViews requires explicit endianness but default to big endian.
            // TypedArrays use the underlying system endianness (usually little).
            is.seekg(block.offset, is.beg);
            const uint32_t blockType = 0;
            is.read((char*)&blockType, 4);
            const uint32_t blockSize = 0;
			is.read((char*)&blockSize, 4);

            if (blockType != block.type) {
                std::ostringstream oss;
                oss << "Block type mismatch.  Header says " <<
                    block.type <<
                    " but block says " <<
                    blockType;

                throw (oss.str());
            }
            if (blockSize != block.size) {
                std::ostringstream oss;
                oss << "Block size mismatch.  Header says " <<
                    block.size <<
                    " but block says " <<
                    blockSize;

                throw (oss.str());
            }
            // note: NOT a copy.
            // never produce copies internally. let callers make a copy if they need it.
            // also note: return the contents of the block NOT including the block header
            size_t nbytesToRead = block.size - BLOCK_HEADER_SIZE;
			uint8_t* data = new uint8_t[nbytesToRead];
            is.seekg(block.offset+BLOCK_HEADER_SIZE, is.beg);
			is.read((char*)data, nbytesToRead);
            return data;
        }

        uint8_t* SimulariumFileReaderBinary::getBlock(const SimulariumFileReaderBinary::BlockInfo& block) {
            std::fstream is(mFilePath, std::ios::in | std::ios::binary);

            // return the block portion after the block header
            // first validate the block with what we expect.

            // TAKE NOTE OF ENDIANNESS.
            // Data transferred via HTTP is generally big endian.
            // Local file should have been written as little endian.
            // All use of DataViews requires explicit endianness but default to big endian.
            // TypedArrays use the underlying system endianness (usually little).
            is.seekg(block.offset, is.beg);
            const uint32_t blockType = 0;
            is.read((char*)&blockType, 4);
            const uint32_t blockSize = 0;
            is.read((char*)&blockSize, 4);

            if (blockType != block.type) {
                std::ostringstream oss;
                oss << "Block type mismatch.  Header says " <<
                    block.type <<
                    " but block says " <<
                    blockType;

                throw (oss.str());
            }
            if (blockSize != block.size) {
                std::ostringstream oss;
                oss << "Block size mismatch.  Header says " <<
                    block.size <<
                    " but block says " <<
                    blockSize;

                throw (oss.str());
            }
            // note: NOT a copy.
            // never produce copies internally. let callers make a copy if they need it.
            // also note: return the contents of the block NOT including the block header
            size_t nbytesToRead = block.size;
            uint8_t* data = new uint8_t[nbytesToRead];
            is.seekg(block.offset, is.beg);
            is.read((char*)data, nbytesToRead);
            return data;
        }

        nlohmann::json SimulariumFileReaderBinary::parseJsonBlock(const SimulariumFileReaderBinary::BlockInfo& block){
            auto blockData = this->getBlockContent(block);
			std::string text((char*)blockData, block.size-BLOCK_HEADER_SIZE);
            nlohmann::json json = nlohmann::json::parse(text);
            return json;
        }

			
    } // namespace fileio
} // namespace simularium
} // namespace aics
