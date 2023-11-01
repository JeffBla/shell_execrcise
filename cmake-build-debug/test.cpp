#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
int main(){
        std::string arg1("cat");
        std::string arg2("test.txt");
	std::string arg3("&");
        char* arg[4] = {strdup(arg1.c_str()),strdup(arg2.c_str()), strdup(arg3.c_str()),NULL};
        execvp(arg[0],arg);  //works
}
