#include "json.hpp"
#include "D400.h"
#include "DataStructure.h"
#include "V2.h"

json DataStructure::jsonify(){
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

void DataStructure::setError(){
    for(int i = 0; i < uncenteredPositions.size(); i++){
        std::vector<float> errorSet;
        errorSet.push_back(uncenteredPositions[i][0]*0.02);
        //std::cout << uncenteredPositions[i][0]*0.02 << std::endl;
        errorSet.push_back(uncenteredPositions[i][1]*0.02);
        errorSet.push_back(uncenteredPositions[i][2]*0.02);
        uncenteredError.push_back(errorSet);
    }

    //std::cout << "\n\n\n\n\n\n\n";
}

void DataStructure::centerPositions(){
    for(int i=0;i<uncenteredPositions.size();i++){
        untransformedPositions.push_back(geometricCenterAdjuster(uncenteredPositions[i]));
    }
}

void DataStructure::transformPositions(D400 &camera){
    for(int i=0;i<untransformedPositions.size();i++){
        positions.push_back(camera.transformPoint(untransformedPositions[i]));
    }
}

void DataStructure::transformError(D400 &camera){
    for(int i=0;i<uncenteredError.size();i++){
        error.push_back(camera.transformError(uncenteredError[i]));
        /*
        for (int k = 0; k < 3; k++){
            std::cout << error[i][k] << " ";
        }
        std::cout << "\n\n\n\n\n";*/
    }
}

std::vector<float> DataStructure::geometricCenterAdjuster(std::vector<float> point3D){
    
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
    