//
//  DataStructure.h
//
//  Created by Yoni Arieh on 5/17/21.
//


#ifndef DataStructure_h
#define DataStructure_h

class DataStructure{
public:
    DataStructure() {
    }
    data_struct getStruct(){
        return data_struct;
    }
private:
    struct data_struct{
        std::vector<float[3]> positions;
        std::vector<float[3]> error;
        std::vector<long long> timestamps;
        
        float[3] spin;
        long serialNumber;
        std::string username;
    };
};

#endif /* DataStructure */
