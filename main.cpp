//I read that we musn't use libraries
//We can use the standard libraries like <iostream> and <vector>, also these ones 

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

struct entry{
    int month, day, hour, min, sec;
    long long totalTime; 
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
    while (pos < n && line[pos] == ' ') 
        ++pos;
        if(pos >=n)
            return "";
        size_t start = pos;
        while (pos < n && line[pos] != ' ')
            ++pos;
        return line.substr(start, pos - start);
}

void splitIp(string &i, int &a, int &b, int &c, int &d, int &p){
    p = stoi(i.substr(i.find(":")+1)); 
    int pos = 0;
    a = stoi(i.substr(pos, i.find(".")));
    pos = i.find(".") +1;
    b = stoi(i.substr(pos, i.find(".")));
    pos = i.find(".", pos) +1;
    c = stoi(i.substr(pos, i.find(".")));
    pos = i.find(".", pos) +1;
    d = stoi(i.substr(pos, i.find(".")));
}

long long total_time(int month, int day, int hour, int minute, int second){
    return (((((month * 31LL + day) * 24 + hour) * 60 + minute) * 60) + second);
} 

void swap(entry& a, entry& b) {
    entry temp = a;
    a = b;
    b = temp;
}

int particion(vector<entry>& a, int low, int high) {
    long long pivot = a[high].totalTime;    
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (a[j].totalTime <= pivot) {      
            ++i;
            swap(a[i], a[j]);               
        }
    }
    swap(a[i + 1], a[high]);                
    return i + 1;
}

void quickSort(vector<entry>& a, int low, int high) {
    if (low < high) {
        int p = particion(a, low, high);
        quickSort(a, low, p - 1);
        quickSort(a, p + 1, high);
    }
}

int main(){
    ifstream theFile("bitacora.txt");
    vector<entry> logs;
    string line;
    while(getline(theFile,line)){

        entry TO; //This is a temporaly object to store the data
        size_t pos = 0; //I use this data type because the function find return this type 
        string month_str = tokenizer(line, pos);
        string day_str = tokenizer(line, pos);
        string time_str = tokenizer(line, pos);
        string ipPort = tokenizer(line, pos);
        string reason = line.substr(pos);
        
        
        TO.month  = months_int(month_str);
        TO.day = stoi(day_str);
        TO.hour = stoi(time_str.substr(0,2));
        TO.min = stoi(time_str.substr(3,2));
        TO.sec = stoi(time_str.substr(6,2));

        
        TO.totalTime = total_time(TO.month, TO.day, TO.hour,  TO.min, TO.sec);
        splitIp(ipPort, TO.ip1, TO.ip2, TO.ip3, TO.ip4, TO.port);
        TO.originLine = line;
        logs.push_back(TO);
        
    }
    theFile.close();
    quickSort(logs, 0, (int)logs.size() -1);
    cout << logs.size() << "\n";
    ofstream outFile("sorted.txt");
    for (auto &e : logs) 
        outFile << e.originLine << "\n";
    outFile.close();

    int month1, day1, month2, day2;
    cin >> month1 >> day1 >> month2 >> day2; //We only need to implement this part to the search Luis

    return 0;
}
