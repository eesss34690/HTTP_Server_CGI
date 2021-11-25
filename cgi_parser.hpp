#include <iostream>
#include <vector>  
#include <string> 
#include <utility>

using namespace std;
class cgi_parser{
private:
    string env;
    int num;
    vector<pair<string, string> > query_big;
public:
    cgi_parser(const char* query);
    void parser();
};