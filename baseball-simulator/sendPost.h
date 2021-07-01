
#ifndef sendPost_hpp
#define sendPost_hpp

#include <iostream>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include "json.hpp"
#include "V2.h"

using namespace std;
using json = nlohmann::json;

/**
 * class used to send post requests
 */
class sendPost{
public:
	sendPost();
    void sendRequest(json body) const;

};

#endif

