#ifndef AICS_SIMULARIUM_FILE_READER_H
#define AICS_SIMULARIUM_FILE_READER_H

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "Utilities/json.hpp"

namespace aics {
namespace simularium {
struct AgentData;

typedef std::vector<aics::simularium::AgentData> AgentDataFrame;

struct TrajectoryFrame {
  AgentDataFrame data;
  float time;
  std::size_t frameNumber;
  TrajectoryFrame() : data({}), time(0), frameNumber(0) {}
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

// Json::Value Serialize(AgentDataFrame& adf);
// std::vector<float> Serialize(AgentData& ad);

struct CameraPosition {
  CameraPosition() {
    position = {0, 0, 120};
    lookAtPosition = {0, 0, 0};
    upVector = {0, 1, 0};
    fovDegrees = 50;
  }

  std::array<float, 3> position;
  std::array<float, 3> lookAtPosition;
  std::array<float, 3> upVector;
  float fovDegrees;
};

struct AgentType {
  std::string name;
  struct Geometry {
    std::string displayType;
    std::string color;
    std::string url;
  } geometry;

  AgentType() : name(""), geometry({"SPHERE", "#FFFFFF", ""}) {}
};
struct Unit {
  std::string name;
  float magnitude;
};
struct TrajectoryFileProperties {
  // std::string fileName = "";
  Unit timeUnits = {"s", 1.0f};
  Unit spatialUnits = {"nm", 1.0f};
  double timeStepSize = 100;
  int32_t totalSteps = 1;
  std::array<float, 3> size;
  CameraPosition cameraDefault;
  std::unordered_map<std::size_t, AgentType> typeMapping;

  std::string Str() {
    return "TrajectoryFileProperties | Number of Frames " +
           std::to_string(this->totalSteps) + " | TimeStep Size " +
           std::to_string(this->timeStepSize) + " | Box Size [" +
           std::to_string(size[0]) + "," + std::to_string(size[1]) + "," +
           std::to_string(size[2]) + "]";
  }
  void fromJson(const nlohmann::json &fprops);
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
};

namespace fileio {

class ISimulariumFile {
public:
  virtual TrajectoryFileProperties getTrajectoryFileInfo() = 0;
  // virtual std::vector<Plot> getPlotData() = 0;
  virtual size_t getNumFrames() = 0;
  // virtual size_t getFrameIndexAtTime(float time) = 0;
  virtual void getFrame(size_t theFrameNumber, TrajectoryFrame *frame) = 0;
};

} // namespace fileio

} // namespace simularium
} // namespace aics

#endif // AICS_SIMULARIUM_FILE_READER_H