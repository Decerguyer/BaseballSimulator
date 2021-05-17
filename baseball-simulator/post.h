
#ifndef Post
#define Post

#include <iostream>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

/**
 * class used to send post requests
 */
class Post{
public:
    json body;
    explicit Post(json b){
        body = b;
    }

    /**
     * sends post request with header content-type: application/json, and a json body.
     */
    void sendRequest() const {
        std::string const str_json = body.dump();
        const char *tmp = str_json.c_str();

        curlpp::initialize(CURL_GLOBAL_ALL);
        char url[] = "https://2342hbqxca.execute-api.us-east-1.amazonaws.com/dev/pitch";
        try{
            curlpp::Cleanup cleaner;
            curlpp::Easy request;
            std::list<std::string> header;
            header.emplace_back("Content-Type: application/json");
            request.setOpt(new curlpp::options::Url(url));
            request.setOpt(new curlpp::options::HttpHeader(header));
            request.setOpt(new curlpp::options::Verbose(false));
            request.setOpt(new curlpp::options::PostFields(tmp));
            request.setOpt(new curlpp::options::PostFieldSize((long) str_json.size()));
            request.perform();
        }
        catch ( curlpp::LogicError & e ) {
            std::cout << e.what() << std::endl;
        }
        catch ( curlpp::RuntimeError & e ) {
            std::cout << e.what() << std::endl;
        }
        cURLpp::terminate();
    }



};

/**
int main (){
    json j;
    j={
            {"positions",{"1", "2", "5"}},
            {"spin", "3"},
            {"pitcher", "owen gay"}
    };
    Post post(j);
    post.sendRequest();
}
*/
#endif