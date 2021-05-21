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
    
    json jsonify(data_struct data){
        if (!error.empty() && !positions.empty() && !timestamps.empty()){
            json out;
            out["serial_number"] = data.serialNumber;
            out["pitcher_id"] = data.username;
            out["spin"] = data.spin;
            out["error"] = data.error;
            out["positions"] = data.positions;
            out["timestamps"] = data.timestamps;
            return out;
        } else {
            json j = {};
            return j;
        }
    }
    
private:
  
};

#endif /* DataStructure */
