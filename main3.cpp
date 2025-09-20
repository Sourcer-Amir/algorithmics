// main.cpp
// Merge Sort recursivo + búsqueda binaria propia
// Sin <sstream> ni <cstdlib>
// Compilar: g++ main.cpp -o bitacora
// Ejecutar (ejemplo): printf "2 1 4 29\n" | ./bitacora

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

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
    string lastStr;  // resto de la línea después de IP:port (desempate final)
};

// ---------- utilidades sin <sstream> ni <cstdlib> ----------
int monthToInt(const string& m) {
    const string months[12] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    for (int i = 0; i < 12; ++i)
        if (months[i] == m) return i + 1;
    return -1;
}

int toInt(const string &s) {
    if (s.empty()) return -1;
    int val = 0;
    int i = 0;
    // permitir signo? no necesario aquí
    for (; i < (int)s.size(); ++i) {
        char c = s[i];
        if (c < '0' || c > '9') return -1;
        val = val * 10 + (c - '0');
    }
    return val;
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

// parse "HH:MM:SS" -> hh,mm,ss ; devuelve true si bien
bool parseTime(const string &s, int &hh, int &mm, int &ss) {
    size_t p1 = s.find(':');
    if (p1 == string::npos) return false;
    size_t p2 = s.find(':', p1 + 1);
    if (p2 == string::npos) return false;
    string sh = s.substr(0, p1);
    string sm = s.substr(p1 + 1, p2 - (p1 + 1));
    string ss_s = s.substr(p2 + 1);
    hh = toInt(sh); mm = toInt(sm); ss = toInt(ss_s);
    if (hh < 0 || hh > 23) return false;
    if (mm < 0 || mm > 59) return false;
    if (ss < 0 || ss > 59) return false;
    return true;
}

// parse "a.b.c.d:port"
bool parseIPPort(const string &token, int &a, int &b, int &c, int &d, int &port) {
    size_t colon = token.find(':');
    if (colon == string::npos) return false;
    string ip = token.substr(0, colon);
    string portStr = token.substr(colon + 1);
    // parse port
    int p = toInt(portStr);
    if (p < 0 || p > 65535) return false;
    // parse 4 octets
    size_t p0 = 0;
    size_t pDot;
    int vals[4];
    for (int i = 0; i < 4; ++i) {
        if (p0 >= ip.size()) return false;
        pDot = ip.find('.', p0);
        string part;
        if (pDot == string::npos) part = ip.substr(p0);
        else part = ip.substr(p0, pDot - p0);
        int v = toInt(part);
        if (v < 0 || v > 255) return false;
        vals[i] = v;
        if (pDot == string::npos) {
            if (i != 3) return false;
            break;
        }
        p0 = pDot + 1;
    }
    a = vals[0]; b = vals[1]; c = vals[2]; d = vals[3];
    port = p;
    return true;
}

// parse line sin istringstream
// formato esperado: "Mon DD HH:MM:SS IP:PORT <resto...>"
bool parseLine(const string &line, LogEntry &e) {
    if (line.empty()) return false;
    // encontrar primer espacio -> month
    size_t p1 = line.find(' ');
    if (p1 == string::npos) return false;
    string mon = line.substr(0, p1);
    // segundo token day
    size_t p2 = line.find(' ', p1 + 1);
    if (p2 == string::npos) return false;
    string dayStr = line.substr(p1 + 1, p2 - (p1 + 1));
    // tercer token time
    size_t p3 = line.find(' ', p2 + 1);
    if (p3 == string::npos) return false;
    string timeStr = line.substr(p2 + 1, p3 - (p2 + 1));
    // cuarto token ip:port
    size_t p4 = line.find(' ', p3 + 1);
    if (p4 == string::npos) return false; // requerimos al menos resta después
    string ipport = line.substr(p3 + 1, p4 - (p3 + 1));
    string rest = line.substr(p4 + 1); // resto de la linea (puede incluir espacios)
    int monthNum = monthToInt(mon);
    if (monthNum == -1) return false;
    int day = toInt(dayStr);
    if (!validDate(monthNum, day)) return false;
    int hh, mm, ss;
    if (!parseTime(timeStr, hh, mm, ss)) return false;
    int a,b,c,d,port;
    if (!parseIPPort(ipport, a,b,c,d,port)) return false;
    e.month = monthNum;
    e.day = day;
    e.hour = hh;
    e.minute = mm;
    e.second = ss;
    e.key = toKey(monthNum, day, hh, mm, ss);
    e.ip1 = a; e.ip2 = b; e.ip3 = c; e.ip4 = d; e.port = port;
    e.original = line;
    e.lastStr = rest;
    return true;
}

// comparador completo: por key, luego octetos, puerto, lastStr
bool entryLess(const LogEntry &A, const LogEntry &B) {
    if (A.key != B.key) return A.key < B.key;
    if (A.ip1 != B.ip1) return A.ip1 < B.ip1;
    if (A.ip2 != B.ip2) return A.ip2 < B.ip2;
    if (A.ip3 != B.ip3) return A.ip3 < B.ip3;
    if (A.ip4 != B.ip4) return A.ip4 < B.ip4;
    if (A.port != B.port) return A.port < B.port;
    return A.lastStr < B.lastStr;
}

// Merge Sort recursivo (vector) - implementado desde cero
void mergeVec(vector<LogEntry> &a, int l, int m, int r, vector<LogEntry> &tmp) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        if (!entryLess(a[j], a[i])) tmp[k++] = a[i++]; // a[i] <= a[j]
        else tmp[k++] = a[j++];
    }
    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (int x = l; x <= r; ++x) a[x] = tmp[x];
}

void mergeSortRec(vector<LogEntry> &a, int l, int r, vector<LogEntry> &tmp) {
    if (l >= r) return;
    int m = l + (r - l)/2;
    mergeSortRec(a, l, m, tmp);
    mergeSortRec(a, m + 1, r, tmp);
    mergeVec(a, l, m, r, tmp);
}

void mergeSort(vector<LogEntry> &a) {
    int n = (int)a.size();
    if (n <= 1) return;
    vector<LogEntry> tmp(n);
    mergeSortRec(a, 0, n - 1, tmp);
}

// Búsqueda binaria propia: lower_bound y upper_bound por key
int lowerBoundByKey(const vector<LogEntry> &logs, long long targetKey) {
    int l = 0, r = (int)logs.size(); // [l, r)
    while (l < r) {
        int m = l + (r - l)/2;
        if (logs[m].key < targetKey) l = m + 1;
        else r = m;
    }
    return l;
}

int upperBoundByKey(const vector<LogEntry> &logs, long long targetKey) {
    int l = 0, r = (int)logs.size();
    while (l < r) {
        int m = l + (r - l)/2;
        if (logs[m].key <= targetKey) l = m + 1;
        else r = m;
    }
    return l;
}

int main() {
    // Leer bitacora.txt en la misma carpeta
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
        // Si una línea es inválida la omitimos silenciosamente
    }
    fin.close();

    // Ordenar con Merge Sort recursivo
    mergeSort(logs);

    // Escribir sorted.txt
    ofstream fout("sorted.txt");
    if (!fout.is_open()) {
        cerr << "Error: no se pudo crear sorted.txt\n";
        return 1;
    }
    for (size_t i = 0; i < logs.size(); ++i) fout << logs[i].original << '\n';
    fout.close();

    // Leer fechas desde stdin para pruebas automáticas: sm sd em ed
    int sm, sd, em, ed;
    if (!(cin >> sm >> sd)) return 0;
    if (!(cin >> em >> ed)) return 0;
    if (!validDate(sm, sd) || !validDate(em, ed)) return 0;

    long long startKey = toKey(sm, sd, 0, 0, 0);
    long long endKey = toKey(em, ed, 23, 59, 59);
    if (startKey > endKey) {
        long long t = startKey; startKey = endKey; endKey = t;
    }

    int startIdx = lowerBoundByKey(logs, startKey);
    int endIdx = upperBoundByKey(logs, endKey) - 1;

    if (startIdx < 0) startIdx = 0;
    if (endIdx >= (int)logs.size()) endIdx = (int)logs.size() - 1;
    if (startIdx <= endIdx) {
        for (int i = startIdx; i <= endIdx; ++i) cout << logs[i].original << '\n';
    }
    return 0;
}
