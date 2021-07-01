//
//  DataStructure.h
//
//  Created by Yoni Arieh on 5/17/21.
//


#ifndef DataStructure_h
#define DataStructure_h

#include "json.hpp"
#include "V2.h"
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

    json jsonify();
    void setError();
    void centerPositions();
    void transformPositions(D400 &camera);
    void transformError(D400 &camera);
    std::vector<float> geometricCenterAdjuster(std::vector<float> point3D);
 
};
    
    
   
#endif /* DataStructure */
