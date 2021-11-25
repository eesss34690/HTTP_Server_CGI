#include <iostream>
#include <vector>  
#include <string>  
#include <utility>
#include <stdio.h>  
#include <stdlib.h>
#include "cgi_parser.hpp"
#include "output_dyn.hpp"

using namespace std;

void output_shell(string session, string content){
    cout << "<script>document.getElementById(\"";
    cout << session << "\").innerHTML += '";
    cout << content << "</br>';</script>\r\n";
}

void output_command(string session, string content){

}

cgi_parser::cgi_parser(const char* query){
    string ans(query);
    env = ans;
    num = 5;
    parser();
}

void cgi_parser::parser()
{
    int start = 0;
    int end = env.find('&');
    int mid;
    while (end != -1) {
        mid = env.find('=', start);
        if (end - mid == 1)
        {
            num = query_big.size() / 3;
            break;
        }
        query_big[env.substr(start, mid - start)] = env.substr(mid + 1, end - mid - 1);
        start = end + 1;
        end = env.find('&', start);
    }
    mid = env.find('=', start);
    if (end - mid == 1)
        query_big[env.substr(start, mid - start)] = env.substr(mid + 1, end - mid - 1);
}

int main ()
{
    cgi_parser query_parse(getenv("QUERY_STRING"));
    cout << "<!DOCTYPE html>\r\n\r\n";
    cout << "<html lang=\"en\">\r\n";
    cout << "<head>\r\n";
    cout << "<meta charset=\"UTF-8\" />\r\n";
    cout << "<title>NP Project 3 Sample Console</title>\r\n";
    cout << "<link\r\n";
    cout << "  rel=\"stylesheet\"\r\n";
    cout << "  href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\r\n";
    cout << "  integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\r\n";
    cout << "  crossorigin=\"anonymous\"\r\n";
    cout << "/>\r\n";
    cout << "<link\r\n";
    cout << "  href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\r\n";
    cout << "  rel=\"stylesheet\"\r\n";
    cout << "/>\r\n";
    cout << "<link\r\n";
    cout << "  rel=\"icon\"\r\n";
    cout << "  type=\"image/png\"\r\n";
    cout << "  href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\r\n";
    cout << "/>\r\n";
    cout << "<style>\r\n";
    cout << "  * {\r\n";
    cout << "    font-family: 'Source Code Pro', monospace;\r\n";
    cout << "    font-size: 1rem !important;\r\n";
    cout << "  }\r\n";
    cout << "  body {\r\n";
    cout << "    background-color: #212529;\r\n";
    cout << "  }\r\n";
    cout << "  pre {\r\n";
    cout << "    color: #cccccc;\r\n";
    cout << "  }\r\n";
    cout << "  b {\r\n";
    cout << "    color: #01b468;\r\n";
    cout << "  }\r\n";
    cout << "</style>\r\n";
    cout << "</head>";
    cout << "  <body>\r\n";
    cout << "<table class=\"table table-dark table-bordered\">\r\n";
    cout << "  <thead>\r\n";
    cout << "    <tr>\r\n";
    for (int i = 0; i< query_parse.get_num(); i++)
    {
        cout << "      <th scope=\"col\">";
        string temp = "h" + to_string(i);
        cout << query_parse.get_attri(temp) << ":";
        temp[0] = 'p';
        cout << query_parse.get_attri(temp) << "</th>\r\n";
    }
    //cout << "      <th scope=\"col\">nplinux1.cs.nctu.edu.tw:1234</th>\r\n";
    cout << "    </tr>\r\n";
    cout << "  </thead>\r\n";
    cout << "  <tbody>\r\n";
    cout << "    <tr>\r\n";
    for (int i = 0; i< query_parse.get_num(); i++)
    {
        cout << "      <td><pre id=\"";
        string temp = "s" + to_string(i);
        cout << temp << "\" class=\"mb-0\"></pre></td>\r\n";
    }
    //cout << "      <td><pre id=\"s0\" class=\"mb-0\"></pre></td>\r\n";
    cout << "    </tr>\r\n";
    cout << "  </tbody>\r\n";
    cout << "</table>\r\n";
    cout << "</body>\r\n";
    cout << "</html>\r\n";

    output_shell("s0", "test");    
    output_shell("s0", "testtest");   
}
