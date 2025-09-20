// main.cpp
// Merge Sort recursivo + búsqueda binaria propia + ordenamiento por IP y puerto.
// Compilar: g++ main.cpp -o bitacora
// Ejecutar ejemplo: printf "2 1 4 29\n" | ./bitacora

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

struct LogEntry {
    int month;   // 1-12
    int day;     // 1-31
    int hour;    // 0-23
    int minute;  // 0-59
    int second;  // 0-59
    long long key; // segundos desde inicio de año
    int ip1, ip2, ip3, ip4; // octetos de la IP
    int port; // puerto
    string original; // línea completa
    string lastStr;  // resto de línea después de IP:port (para desempate final)
};

// helpers
int monthToInt(const string& m) {
    const string months[12] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    for (int i = 0; i < 12; ++i)
        if (months[i] == m) return i + 1;
    return -1;
}
bool validDate(int month, int day) {
    if (month < 1 || month > 12) return false;
    const int daysInMonth[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (day < 1 || day > daysInMonth[month]) return false;
    return true;
}
long long toKey(int month, int day, int hour, int minute, int second) {
    const int daysInMonth[13] =
        {0,31,28,31,30,31,30,31,31,30,31,30,31};
    long long totalDays = 0;
    for (int m = 1; m < month; ++m) totalDays += daysInMonth[m];
    totalDays += (day - 1);
    return (((totalDays * 24LL + hour) * 60LL + minute) * 60LL + second);
}

// Parse IP:port like "235.99.27.158:6526"
// returns true if parsed ok
bool parseIPPort(const string& token, int &a, int &b, int &c, int &d, int &port) {
    // find colon
    size_t colon = token.find(':');
    if (colon == string::npos) return false;
    string ip = token.substr(0, colon);
    string portStr = token.substr(colon + 1);
    // split ip by '.'
    int vals[4] = {0,0,0,0};
    size_t p = 0, start = 0;
    int idx = 0;
    while (idx < 4) {
        size_t dot = ip.find('.', start);
        string part;
        if (dot == string::npos) {
            part = ip.substr(start);
            // must be last
            start = ip.size();
        } else {
            part = ip.substr(start, dot - start);
            start = dot + 1;
        }
        if (part.empty()) return false;
        // convert
        char *endptr = nullptr;
        long val = strtol(part.c_str(), &endptr, 10);
        if (*endptr != '\0' || val < 0 || val > 255) return false;
        vals[idx++] = (int)val;
        if (dot == string::npos) break;
    }
    if (idx != 4) return false;
    // port
    char *endptr = nullptr;
    long vp = strtol(portStr.c_str(), &endptr, 10);
    if (*endptr != '\0' || vp < 0 || vp > 65535) return false;
    a = vals[0]; b = vals[1]; c = vals[2]; d = vals[3];
    port = (int)vp;
    return true;
}

// Parsing de línea completa
bool parseLine(const string& line, LogEntry &e) {
    if (line.empty()) return false;
    istringstream iss(line);
    string mon;
    int day;
    string timeStr;
    if (!(iss >> mon >> day >> timeStr)) return false;

    int hh = 0, mm = 0, ss = 0;
    if (sscanf(timeStr.c_str(), "%d:%d:%d", &hh, &mm, &ss) != 3) return false;

    int monthNum = monthToInt(mon);
    if (monthNum == -1) return false;
    if (!validDate(monthNum, day)) return false;
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59 || ss < 0 || ss > 59) return false;

    string ipport;
    if (!(iss >> ipport)) return false;

    int a,b,c,d,p;
    if (!parseIPPort(ipport, a, b, c, d, p)) return false;

    string rest;
    getline(iss, rest); // incluye el espacio inicial, si existe
    if (!rest.empty() && rest[0] == ' ') rest.erase(0,1); // quitar espacio inicial

    e.month = monthNum;
    e.day = day;
    e.hour = hh;
    e.minute = mm;
    e.second = ss;
    e.key = toKey(monthNum, day, hh, mm, ss);
    e.ip1 = a; e.ip2 = b; e.ip3 = c; e.ip4 = d; e.port = p;
    e.original = line;
    e.lastStr = rest;
    return true;
}

// Comparador: (a < b) según tu orden: key, ip octet1..4, port, lastStr
bool entryLess(const LogEntry &a, const LogEntry &b) {
    if (a.key != b.key) return a.key < b.key;
    if (a.ip1 != b.ip1) return a.ip1 < b.ip1;
    if (a.ip2 != b.ip2) return a.ip2 < b.ip2;
    if (a.ip3 != b.ip3) return a.ip3 < b.ip3;
    if (a.ip4 != b.ip4) return a.ip4 < b.ip4;
    if (a.port != b.port) return a.port < b.port;
    return a.lastStr < b.lastStr;
}

// Merge Sort (recursivo) para vector<LogEntry>
void merge(vector<LogEntry>& a, int l, int m, int r, vector<LogEntry>& tmp) {
    int i = l;
    int j = m + 1;
    int k = l;
    while (i <= m && j <= r) {
        if (!entryLess(a[j], a[i])) tmp[k++] = a[i++]; // a[i] <= a[j]
        else tmp[k++] = a[j++];
    }
    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (int x = l; x <= r; ++x) a[x] = tmp[x];
}
void mergeSortRec(vector<LogEntry>& a, int l, int r, vector<LogEntry>& tmp) {
    if (l >= r) return;
    int m = l + (r - l) / 2;
    mergeSortRec(a, l, m, tmp);
    mergeSortRec(a, m + 1, r, tmp);
    merge(a, l, m, r, tmp);
}
void mergeSort(vector<LogEntry>& a) {
    int n = (int)a.size();
    if (n <= 1) return;
    vector<LogEntry> tmp(n);
    mergeSortRec(a, 0, n - 1, tmp);
}

// Búsqueda binaria propia (lower_bound / upper_bound)
// lowerBound: primera posición i tal que logs[i].key >= targetKey
int lowerBoundByKey(const vector<LogEntry>& logs, long long targetKey) {
    int l = 0, r = (int)logs.size(); // intervalo [l, r)
    while (l < r) {
        int m = l + (r - l) / 2;
        if (logs[m].key < targetKey) l = m + 1;
        else r = m;
    }
    return l;
}
// upperBound: primera posición i tal que logs[i].key > targetKey
int upperBoundByKey(const vector<LogEntry>& logs, long long targetKey) {
    int l = 0, r = (int)logs.size();
    while (l < r) {
        int m = l + (r - l) / 2;
        if (logs[m].key <= targetKey) l = m + 1;
        else r = m;
    }
    return l;
}

int main() {
    // Leer bitacora.txt
    ifstream fin("bitacora.txt");
    if (!fin.is_open()) {
        cerr << "Error: no se pudo abrir bitacora.txt\n";
        return 1;
    }
    vector<LogEntry> logs;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        LogEntry e;
        if (parseLine(line, e)) logs.push_back(e);
        // si hay líneas mal formadas, las omitimos
    }
    fin.close();

    // Ordenar
    mergeSort(logs);

    // Escribir sorted.txt
    ofstream fout("sorted.txt");
    if (!fout.is_open()) {
        cerr << "Error: no se pudo crear sorted.txt\n";
        return 1;
    }
    for (const auto &e : logs) fout << e.original << '\n';
    fout.close();

    // Leer fechas desde stdin (para pruebas automáticas)
    int sm, sd, em, ed;
    if (!(cin >> sm >> sd)) return 0;
    if (!(cin >> em >> ed)) return 0;
    if (!validDate(sm, sd) || !validDate(em, ed)) return 0;

    long long startKey = toKey(sm, sd, 0, 0, 0);
    long long endKey = toKey(em, ed, 23, 59, 59);
    if (startKey > endKey) { long long t = startKey; startKey = endKey; endKey = t; }

    // Encontrar índices con búsqueda binaria propia
    int startIdx = lowerBoundByKey(logs, startKey);
    int endIdx = upperBoundByKey(logs, endKey) - 1;

    if (startIdx < 0) startIdx = 0;
    if (endIdx >= (int)logs.size()) endIdx = (int)logs.size() - 1;
    if (startIdx <= endIdx) {
        for (int i = startIdx; i <= endIdx; ++i) {
            cout << logs[i].original << '\n';
        }
    }
    return 0;
}
