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
 
};
    
    
   
#endif /* DataStructure */
