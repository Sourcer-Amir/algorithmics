//I read that we musn't use libraries
//We can use the standard libraries like <iostream> and <vector>, also these ones 
//Change the primary key to "timeSum" or something like that
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

struct entry{
    int month, day, hour, minute, second;
    long long totalTime; //Is not a real primary key, but it's similar Dan 
    int ip1, ip2, ip3, ip4; //Cant be a string because we need to compare them
    int port;
    string reason; //The last part of the log entry
    string originLine; //I think this will be useful for printing the querie
};

int months_int(string& month){
    string months [12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    for(int i = 0; i < 12; i++)
        if(months[i] == month) 
            return i + 1;
    return -1;
}

string tokenizer(string& line, size_t &pos){
    size_t n = line.size();
    while (pos < n && line[pos] == ' ') ++pos;
}


long long primaryKey(int month, int day, int hour, int minute, int second){
    return (((((month * 31LL + day) * 24 + hour) * 60 + minute) * 60) + second);
} // I already know that is not a primary key, not unique, but it's pretty similar



int main(){
    ifstream theFile("bitacora.txt");
    vector<entry> logs;
    string line;
    while(getline(theFile,line)){
        entry TO; //This is a temporaly object to store the data
        size_t pos = 0; //I use this data type because the function find return this type 

        //TO.originLine = line;
        logs.push_back(TO);
    }
    cout << logs.size() << "\n";

    return 0;
}
