//
//  DataStructure.h
//
//  Created by Yoni Arieh on 5/17/21.
//


#ifndef DataStructure_h
#define DataStructure_h

#include "json.hpp"
using json = nlohmann::json;
using namespace std;

struct DataStructure{
    
    vector<vector<float>> uncenteredPositions;
    vector<vector<float>> uncenteredError;
    
    vector<vector<float>> untransformedPositions;
    vector<vector<float>> untransformedError;
    
    //These fields will be used for the JSON
    vector<vector<float>> positions;
    vector<vector<float>> error;
    vector<long long> timestamps;
    vector <float> spin;

    long serialNumber;
    string username;

    json jsonify(){
        if (!error.empty() && !positions.empty() && !timestamps.empty()){
            json out;
            out["serial_number"] = serialNumber;
            out["pitcher_id"] = username;
            out["spin"] = spin;
            out["error"] = error;
            out["positions"] = positions;
            out["timestamps"] = timestamps;
            return out;
        } else {
            json j = {};
            return j;
        }
    }
    
    void centerPositions(CameraCalibration calib){
        for(int i=0;i<uncenteredPositions.size();i++){
            untransformedPositions.push_back(calib.convertPosMatVec(calib.centerPointAdjust(calib.convertPosVecMat(uncenteredPositions[i]))));
        }
    }
    void transformPositions(CameraCalibration calib){
        for(int i=0;i<untransformedPositions.size();i++){
            positions.push_back(calib.convertPosMatVec(calib.transformPoint(calib.convertPosVecMat(untransformedPositions[i]))));
        }
    }
    void setError(){
        for(int i = 0; i < positions.size(); i++){
            std::vector<float> errorSet;
            errorSet.push_back(0.01);
            errorSet.push_back(0.01);
            errorSet.push_back(0.01);
            error.push_back(errorSet);
        }
    }
 
};
    
    
   
#endif /* DataStructure */
