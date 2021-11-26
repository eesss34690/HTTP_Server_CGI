#include <iostream>
#include <string> 
using namespace std;

void output_shell(string session, const char* content){
    cout << "<script>document.getElementById(\"";
    cout << session << "\").innerHTML += '";
    cout << content << "</br>';</script>\r\n";
}

void output_command(string session, const char* content){
    cout << "<script>document.getElementById(\"";
    cout << session << "\").innerHTML += '<b>";
    cout << content << "</br></b>';</script>\r\n";
}
