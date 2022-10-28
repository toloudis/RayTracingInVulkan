#include "SimulariumJson.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace aics {
namespace simularium {
    namespace fileio {

        SimulariumFileReaderJson::SimulariumFileReaderJson(std::string filePath) {
            std::ifstream inputstream(filePath);
            inputstream >> this->mJsonRoot;

        }
        TrajectoryFileProperties SimulariumFileReaderJson::getTrajectoryFileInfo() {
            nlohmann::json& fprops = mJsonRoot["trajectoryInfo"];

			
            TrajectoryFileProperties tfp;

            const nlohmann::json& typeMapping = fprops["typeMapping"];

            for (auto it = typeMapping.begin(); it != typeMapping.end(); ++it)
            {
                std::size_t idKey = std::atoi(it.key().c_str());
                const nlohmann::json& entry = it.value();
                tfp.typeMapping[idKey] = entry["name"].get<std::string>();
            }

            const nlohmann::json& size = fprops["size"];
            tfp.boxX = size["x"].get<float>();
            tfp.boxY = size["y"].get<float>();
            tfp.boxZ = size["z"].get<float>();

            tfp.fileName = fprops["fileName"].get<std::string>();
            tfp.numberOfFrames = fprops["totalSteps"].get<int>();
            tfp.timeStepSize = fprops["timeStepSize"].get<float>();
            tfp.spatialUnitFactorMeters = fprops["spatialUnitFactorMeters"].get<float>();

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

        SimulariumFileReaderBinary::SimulariumFileReaderBinary(std::string filePath) {}

        TrajectoryFileProperties SimulariumFileReaderBinary::getTrajectoryFileInfo() { return TrajectoryFileProperties(); }
        // virtual std::vector<Plot> getPlotData() = 0;
        size_t SimulariumFileReaderBinary::getNumFrames() { return 0; }
        //virtual size_t getFrameIndexAtTime(float time);
        void SimulariumFileReaderBinary::getFrame(size_t theFrameNumber, TrajectoryFrame* frame) {}

		
        const enum BlockTypeEnum {
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
        const std::string SIGNATURE = "SIMULARIUMBINARY";

        // each block has a type and a size
        const uint32_t BLOCK_HEADER_SIZE = 8;

        export default class BinaryFileReader implements ISimulariumFile {
        fileContents: ArrayBuffer;
        dataView: DataView;
        header: Header;
        tfi: TrajectoryFileInfo;
        plotData: Plot[];
        nFrames: number;
        frameOffsets: number[];
        frameLengths: number[];
        spatialDataBlock: DataView; // ideally this is really a Float32Array but alignment is not guaranteed yet
            constructor(fileContents: ArrayBuffer) {
                this.nFrames = 0;
                this.frameOffsets = [];
                this.frameLengths = [];
                this.fileContents = fileContents;
                this.dataView = new DataView(fileContents);
                this.header = this.readHeader();
                this.tfi = this.readTrajectoryFileInfo();
                this.plotData = this.readPlotData();
                this.spatialDataBlock = this.readSpatialDataInfo();
            }

            static isBinarySimulariumFile(fileContents: Blob) : Promise<boolean>{
                const first16blob : Blob = fileContents.slice(0, 16);
                return first16blob.text().then((text) = > {
                    return text == = SIGNATURE;
                });
            }

                private readSpatialDataInfo() : DataView{
                // find spatial data block and load frame offsets
                for (const block of this.header.blocks) {
                    if (block.type == = BlockTypeEnum.SPATIAL_DATA_BINARY) {
                        const blockData = this.getBlockContent(block);
                        let byteOffset = 0;
                        // Spatial data version (4-byte int)
                        // const version = blockData.getUint32(byteOffset, true);
                        byteOffset += 4;
                        // Number of frames (4-byte int)
                        this.nFrames = blockData.getUint32(byteOffset, true);
                        byteOffset += 4;

                        // Frame offsets,sizes (Number of frames * two 4-byte ints)
                        for (let i = 0; i < this.nFrames; i++) {
                            this.frameOffsets.push(
                                blockData.getUint32(byteOffset, true)
                            );
                            byteOffset += 4;
                            this.frameLengths.push(
                                blockData.getUint32(byteOffset, true)
                            );
                            byteOffset += 4;
                        }

                        return this.getBlock(block);
                    }
                }
                throw new Error("No spatial data block found");
            }

                private readHeader() : Header{
                // could use DataView here but I know every header field is int32
                // note I set the offset to move past the signature string
                const asints = new Uint32Array(this.fileContents, SIGNATURE.length);
                const headerLength = asints[0];
                const headerVersion = asints[1];
                const nBlocks = asints[2];
                if (nBlocks < 1) {
                    throw new Error("No blocks found in file");
                }
                const blocks : BlockInfo[] = [];
                // the number of 32-bit ints after the SIGNATURE and before the toc
                const OFFSET_TO_TABLE_OF_CONTENTS = 3;
                for (let i = 0; i < nBlocks; i++) {
                    blocks.push({
                        offset: asints[OFFSET_TO_TABLE_OF_CONTENTS + i * 3 + 0],
                        type : asints[OFFSET_TO_TABLE_OF_CONTENTS + i * 3 + 1],
                        size : asints[OFFSET_TO_TABLE_OF_CONTENTS + i * 3 + 2],
                    });
                }
                if (blocks[0].offset != = headerLength) {
                    throw new Error("First block offset does not match header length");
                }
                return {
                    version: headerVersion,
                    blocks,
                };
            }

                private readTrajectoryFileInfo() : TrajectoryFileInfo{
                // find the first block that is a trajectory info block
                for (const block of this.header.blocks) {
                    if (block.type == = BlockTypeEnum.TRAJECTORY_INFO_JSON) {
                        const json = this.parseJsonBlock(block);
                        return json as TrajectoryFileInfo;
                    }
                }
                throw new Error("No trajectory info block found");
            }

                private readPlotData() : Plot[]{
                // find the first block that is a trajectory info block
                for (const block of this.header.blocks) {
                    if (block.type == = BlockTypeEnum.PLOT_DATA_JSON) {
                        const json = this.parseJsonBlock(block);
                        return json as Plot[];
                    }
                }
                return[];
            }

                private getBlock(block: BlockInfo) : DataView{
                // first validate the block with what we expect.

                // TAKE NOTE OF ENDIANNESS.
                // Data transferred via HTTP is generally big endian.
                // Local file should have been written as little endian.
                // All use of DataViews requires explicit endianness but default to big endian.
                // TypedArrays use the underlying system endianness (usually little).
                const blockType = this.dataView.getUint32(block.offset, true);
                const blockSize = this.dataView.getUint32(block.offset + 4, true);

                if (blockType != = block.type) {
                    throw new Error(
                        "Block type mismatch.  Header says " +
                            block.type +
                            " but block says " +
                            blockType
                    );
                }
                if (blockSize != = block.size) {
                    throw new Error(
                        "Block size mismatch.  Header says " +
                            block.size +
                            " but block says " +
                            blockSize
                    );
                }
                // note: NOT a copy.
                // never produce copies internally. let callers make a copy if they need it.
                // also note: return the contents of the block INCLUDING the block header
                return new DataView(this.fileContents, block.offset, block.size);
            }

                private getBlockContent(block: BlockInfo) : DataView{
                // return the block portion after the block header
                // first validate the block with what we expect.

                // TAKE NOTE OF ENDIANNESS.
                // Data transferred via HTTP is generally big endian.
                // Local file should have been written as little endian.
                // All use of DataViews requires explicit endianness but default to big endian.
                // TypedArrays use the underlying system endianness (usually little).
                const blockType = this.dataView.getUint32(block.offset, true);
                const blockSize = this.dataView.getUint32(block.offset + 4, true);

                if (blockType != = block.type) {
                    throw new Error(
                        "Block type mismatch.  Header says " +
                            block.type +
                            " but block says " +
                            blockType
                    );
                }
                if (blockSize != = block.size) {
                    throw new Error(
                        "Block size mismatch.  Header says " +
                            block.size +
                            " but block says " +
                            blockSize
                    );
                }
                // note: NOT a copy.
                // never produce copies internally. let callers make a copy if they need it.
                // also note: return the contents of the block NOT including the block header
                return new DataView(
                    this.fileContents,
                    block.offset + BLOCK_HEADER_SIZE,
                    block.size - BLOCK_HEADER_SIZE
                );
            }

                /* eslint-disable-next-line @typescript-eslint/no-explicit-any */
                private parseJsonBlock(block: BlockInfo) : any{
                    const blockData = this.getBlockContent(block);
                    const enc = new TextDecoder("utf-8");
                    const text = enc.decode(blockData);
                    // trim any trailing null bytes
                    const trimmed = text.replace(/ \0 + $ / , "");
                    const json = JSON.parse(trimmed);
                    return json;
            }

                getTrajectoryFileInfo() : TrajectoryFileInfo{
                    return this.tfi;
            }

                getNumFrames() : number{
                    return this.nFrames;
            }

                getFrameIndexAtTime(time : number) : number{
                    const { timeStepSize } = this.tfi;
            // Find the index of the frame that has the time matching our target time:
            // walk offsets and parse frame times.
            // assumes frames are in ascending order by time!
            for (let i = 0; i < this.nFrames; i++) {
                // get time of frame.
                // const frameNumber = this.spatialDataBlock[frameFloatOffset + 0];
                const frameTime = this.spatialDataBlock.getFloat32(
                    this.frameOffsets[i] + 4,
                    true
                );
                // check time
                if (compareTimes(frameTime, time, timeStepSize) == = 0) {
                    // TODO possible sanity check frameNumber === i ?
                    return i;
                }
            }

            return -1;
            }

                getFrame(theFrameNumber: number) : VisDataFrame | ArrayBuffer{
                    const frameOffset = this.frameOffsets[theFrameNumber];
                    const frameSize = this.frameLengths[theFrameNumber];
                    const totalOffset = this.spatialDataBlock.byteOffset + frameOffset;
                    // TODO possibly this can return a TypedArray or DataView without making a copy
                    // but requires a guarantee on 4-byte alignment. Leaving it as a future optimization.
                    const frameContents = this.fileContents.slice(
                        totalOffset,
                        totalOffset + frameSize
                    );
                    return frameContents;
            }

                getPlotData() : Plot[]{
                    return this.plotData;
            }
        }




















		
    } // namespace fileio
} // namespace simularium
} // namespace aics
