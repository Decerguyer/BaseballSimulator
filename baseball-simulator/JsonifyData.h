//
//  JsonifyData.h
//
//
//  Created by Yoni Arieh on 5/18/21.
//

#include "DataStructure.h"
#include "json.hpp"

using json = nlohmann::json;

#ifndef JsonifyData_h
#define JsonifyData_h

class JsonifyData{
public:
    //Constructor passes a specialized data struct to jsonify function
    JsonifyData(DataStructure data) {
        jsonify(data);
    }
    //getJson() returns the resultant json
    json getJson(){
        return j;
    }
private:
    //Private variable that stores the json
    json j;
    
    //jsonify function uses the data structure to create a json
    //Saves resultant in private variable 'j'
    jsonify(DataStructure data){
        
        //************************Juan Write Code here***********************//
        
    }
};



#endif /* JsonifyData */
