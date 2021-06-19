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
    void centerPositions(){
        for(int i=0;i<uncenteredPositions.size();i++){
            untransformedPositions.push_back(geometricCenterAdjuster(uncenteredPositions[i]));
        }
    }
    void transformPositions(D400 camera){
        for(int i=0;i<untransformedPositions.size();i++){
            positions.push_back(camera.transformPoint(untransformedPositions[i]));
        }
    }
    void transformError(D400 camera){
        //for(int i=0;i<untransformedError.size();i++){
        for(int i=0;i<uncenteredError.size();i++){
            error.push_back(camera.transformError(uncenteredError[i]));
            /*
            for (int k = 0; k < 3; k++){
                std::cout << error[i][k] << " ";
            }
            std::cout << "\n\n\n\n\n";*/
        }
    }
    
    std::vector<float> geometricCenterAdjuster(std::vector<float> point3D){
        
        double x = point3D[0];
        double y = point3D[1];
        double z = point3D[2];

        double r = BASEBALL_RADIUS; //Baseball radius in meters approximation

        double _x, _y, _z;

        _x = (x*std::sqrt(x*x + y*y + z*z + r))/std::sqrt(x*x + y*y + z*z); //Similar Triangles
        _y = (y*(x*x + y*y + z*z + r))/(x*x + y*y + z*z);
        _z = (z*(x*x + y*y + z*z + r))/(x*x + y*y + z*z);

        std::vector<float> adjustedPoint;
        adjustedPoint.push_back(_x);
        adjustedPoint.push_back(_y);
        adjustedPoint.push_back(_z);
        return adjustedPoint;
    }
 
};
    
    
   
#endif /* DataStructure */
