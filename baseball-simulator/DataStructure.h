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
    
    void setError(){
        for(int i = 0; i < uncenteredPositions.size(); i++){
            std::vector<float> errorSet;
            errorSet.push_back(uncenteredPositions[i][0]*0.02);
            std::cout << uncenteredPositions[i][0]*0.02 << std::endl;
            errorSet.push_back(uncenteredPositions[i][1]*0.02);
            errorSet.push_back(uncenteredPositions[i][2]*0.02);
            uncenteredError.push_back(errorSet);
        }

        std::cout << "\n\n\n\n\n\n\n";
    }
    void centerPositions(CameraCalibration calib){
        for(int i=0;i<uncenteredPositions.size();i++){
            untransformedPositions.push_back(calib.convertPosMatVec(calib.centerPointAdjust(calib.convertPosVecMat(uncenteredPositions[i]))));
        }
    }
    /*
    void centerError(CameraCalibration calib){
        for(int i=0;i<uncenteredError.size();i++){
            untransformedError.push_back(calib.convertPosMatVec(calib.centerPointAdjust(calib.convertPosVecMat(uncenteredError[i]))));
            for (int k = 0; k < 3; k++){
                std::cout << untransformedError[i][k] << " ";
            }
            std::cout << "\n\n\n\n\n";
        }
    }
    */
    void transformPositions(CameraCalibration calib){
        for(int i=0;i<untransformedPositions.size();i++){
            positions.push_back(calib.convertPosMatVec(calib.transformPoint(calib.convertPosVecMat(untransformedPositions[i]))));
        }
    }
    void transformError(CameraCalibration calib){
        //for(int i=0;i<untransformedError.size();i++){
        for(int i=0;i<uncenteredError.size();i++){
            //error.push_back(calib.convertPosMatVec(calib.transformPoint(calib.convertPosVecMat(untransformedError[i]))));
            error.push_back(calib.convertPosMatVec(calib.transformPoint(calib.convertPosVecMat(uncenteredError[i]))));
            for (int k = 0; k < 3; k++){
                std::cout << error[i][k] << " ";
            }
            std::cout << "\n\n\n\n\n";
        }
    }
 
};
    
    
   
#endif /* DataStructure */
