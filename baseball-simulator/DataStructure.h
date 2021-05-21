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

class DataStructure{
public:
    DataStructure() {
    }
    /*data_struct getStruct(){
        return data_struct;
    }*/
    
    struct data_struct{
        vector<vector<float>> positions;
        vector<vector<float>> error;
        vector<long long> timestamps;
        float spin[3];

        long serialNumber;
        string username;
    };
    
    json jsonObj;
    
    json jsonify(){
        if (!error.empty() && !positions.empty() && !timestamps.empty()){
            json out;
            out["serial_number"] = data_struct.serialNumber;
            out["pitcher_id"] = data_struct.username;
            out["spin"] = data_struct.spin;
            out["error"] = data_struct.error;
            out["positions"] = data_struct.positions;
            out["timestamps"] = data_struct.timestamps;
            return out;
        } else {
            json j = {};
            return j;
        }
    }
    
private:
  
};

#endif /* DataStructure */
