#include <iostream>
#include <vector>
#include <fstream>
#include <string>
using namespace std;

// -------------------------------------------------------------
// Estructura que representa una entrada (registro) de la bitácora
// -------------------------------------------------------------
struct entry{
    int month, day, hour, min, sec;   // Fecha y hora desglosada
    long long totalTime;              // Clave numérica para ordenar por fecha/hora
    int ip1, ip2, ip3, ip4;           // Octetos de la IP (para comparar punto por punto)
    int port;                         // Puerto
    string reason;                    // Resto del mensaje (motivo / descripción)
    string originLine;                // Línea original tal cual (útil para imprimir)
};

// -------------------------------------------------------------
// Convierte el nombre del mes (ej. "Jan") a número (1..12)
// -------------------------------------------------------------
int months_int(string& month){
    string months [12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    for(int i = 0; i < 12; i++)
        if(months[i] == month) 
            return i + 1;
    return -1; // devuelve -1 si el mes no es válido (no debería ocurrir si el archivo está bien formado)
}

// -------------------------------------------------------------
// Tokenizador sencillo: extrae el siguiente token separado por espacios
// pos es la posición de inicio y se actualiza para la siguiente llamada
// -------------------------------------------------------------
string tokenizer(string& line, size_t &pos){
    size_t n = line.size();
    while (pos < n && line[pos] == ' ') 
        ++pos;
    if(pos >= n)
        return "";
    size_t start = pos;
    while (pos < n && line[pos] != ' ')
        ++pos;
    return line.substr(start, pos - start);
}

// -------------------------------------------------------------
// Divide una cadena "ip:port" en sus componentes numéricos
// Recibe el string i (ej. "192.168.0.1:8080") y llena a,b,c,d,p
// -------------------------------------------------------------
void splitIp(string &i, int &a, int &b, int &c, int &d, int &p){
    // obtener puerto (todo lo que va después de ':')
    p = stoi(i.substr(i.find(":")+1)); 

    // extraer cada octeto usando posiciones relativas
    int pos = 0;
    a = stoi(i.substr(pos, i.find(".")));             // primer octeto desde inicio
    pos = i.find(".") + 1;
    b = stoi(i.substr(pos, i.find(".")));             // segundo octeto (busca primer '.' desde pos)
    pos = i.find(".", pos) + 1;
    c = stoi(i.substr(pos, i.find(".")));             // tercer octeto
    pos = i.find(".", pos) + 1;
    d = stoi(i.substr(pos, i.find(".")));             // cuarto octeto (termina antes de ':' en la práctica)
}

// -------------------------------------------------------------
// Calcula una clave numérica a partir de fecha y hora
// (usa 31 días por mes, conforme a la suposición del enunciado)
// -------------------------------------------------------------
long long total_time(int month, int day, int hour, int minute, int second){
    return (((((month * 31LL + day) * 24 + hour) * 60 + minute) * 60) + second);
} 

// -------------------------------------------------------------
// Comparador que aplica el orden requerido:
// 1) totalTime (fecha/hora)
// 2) ip1, ip2, ip3, ip4 (octeto por octeto)
// 3) port
// 4) reason (cadena) como desempate final
// -------------------------------------------------------------
bool lessEntry(const entry &A, const entry &B) {
    if (A.totalTime != B.totalTime) 
        return A.totalTime < B.totalTime;
    if (A.ip1 != B.ip1) 
        return A.ip1 < B.ip1;
    if (A.ip2 != B.ip2) 
        return A.ip2 < B.ip2;
    if (A.ip3 != B.ip3) 
        return A.ip3 < B.ip3;
    if (A.ip4 != B.ip4) 
        return A.ip4 < B.ip4;
    if (A.port != B.port) 
        return A.port < B.port;
    return A.reason < B.reason;
} // With this function we will compare the Ip plus the totaltime

// -------------------------------------------------------------
// Swap simple para intercambiar dos entries (utilizado por quicksort)
// -------------------------------------------------------------
void swap(entry& a, entry& b) {
    entry temp = a;
    a = b;
    b = temp;
}

// -------------------------------------------------------------
// Partición estilo Lomuto para quicksort sobre vector<entry>
// Se usa lessEntry para comparar registros según la prioridad definida
// -------------------------------------------------------------
int particion(vector<entry>& a, int low, int high) {
    entry pivot = a[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (lessEntry(a[j], pivot)) {
            ++i;
            swap(a[i], a[j]);
        }
    }
    swap(a[i + 1], a[high]);
    return i + 1;
}

// -------------------------------------------------------------
// Quicksort recursivo usando particion de Lomuto
// -------------------------------------------------------------
void quickSort(vector<entry>& a, int low, int high) {
    if (low < high) {
        int p = particion(a, low, high);
        quickSort(a, low, p - 1);
        quickSort(a, p + 1, high);
    }
}

// -------------------------------------------------------------
// Búsquedas binarias para encontrar límites por totalTime
// lowerBoundSum -> primera posición con totalTime >= thetime
// upperBoundSum -> primera posición con totalTime > thetime
// -------------------------------------------------------------
int lowerBoundSum(const vector<entry> &v, long long thetime) { 
    int l = 0, r = (int)v.size();
    while (l < r) {
        int m = l + (r - l) / 2;
        if (v[m].totalTime < thetime) l = m + 1; 
        else r = m;
    } 
    return l;
} //Binary search to find the lower bound 

int upperBoundSum(const vector<entry> &v, long long thetime) {
    int l = 0, r = (int)v.size();
    while (l < r) {
        int m = l + (r - l) / 2;
        if (v[m].totalTime <= thetime) l = m + 1; 
        else r = m;
    }
    return l;
} //Binary search to find the upper bound 

// -------------------------------------------------------------
// Función principal
// 1) Lee bitacora.txt línea por línea
// 2) Parsea tokens (mes, día, hora, ip:port, razón)
// 3) Calcula totalTime y divide la IP en octetos
// 4) Inserta registros en logs
// 5) Ordena con quickSort usando la comparación definida
// 6) Escribe sorted.txt con las líneas ordenadas
// 7) Lee rango de fechas desde stdin y muestra registros en ese rango
// -------------------------------------------------------------
int main(){
    ifstream theFile("bitacora.txt");
    vector<entry> logs;
    string line;

    // Lectura y parsing: asumimos que bitacora.txt está bien formado
    while(getline(theFile,line)){
        entry TO; // temporal para cada línea
        size_t pos = 0; // posición para tokenizer
        string month_str = tokenizer(line, pos);   // token mes (ej. "Feb")
        string day_str = tokenizer(line, pos);     // token día (ej. "01", "30")
        string time_str = tokenizer(line, pos);    // token hora (HH:MM:SS)
        string ipPort = tokenizer(line, pos);      // token ip:port
        string reason = line.substr(pos);          // resto de la línea -> reason

        TO.reason = reason;        
        TO.month  = months_int(month_str);
        TO.day = stoi(day_str);
        TO.hour = stoi(time_str.substr(0,2));
        TO.min = stoi(time_str.substr(3,2));
        TO.sec = stoi(time_str.substr(6,2));

        // clave/tiempo total para ordenar (segundos relativos)
        TO.totalTime = total_time(TO.month, TO.day, TO.hour,  TO.min, TO.sec);

        // dividir IP:PORT en sus componentes numéricos
        splitIp(ipPort, TO.ip1, TO.ip2, TO.ip3, TO.ip4, TO.port);

        TO.originLine = line;   // almacenamos la línea original tal cual
        logs.push_back(TO);     // agregamos al vector
    }
    theFile.close();

    // Ordenar los registros según la comparación definida
    quickSort(logs, 0, (int)logs.size() -1);

    // Escribir todos los registros ordenados en sorted.txt (misma estructura que la entrada)
    ofstream outFile("sorted.txt");
    for (auto &e : logs) 
        outFile << e.originLine << "\n";
    outFile.close();

    // Lectura de rango de fechas desde stdin (para pruebas automáticas)
    int sm, sd, em, ed;
    if (!(cin >> sm >> sd)) return 0;
    if (!(cin >> em >> ed)) return 0;

    // Convertir rango a totalTime (incluir desde 00:00:00 hasta 23:59:59)
    long long sk = total_time(sm, sd, 0, 0, 0);
    long long ek = total_time(em, ed, 23, 59, 59);
    if (sk > ek) { long long t = sk; sk = ek; ek = t; }

    // Encontrar índices con búsqueda binaria y mostrar los registros del rango
    int start = lowerBoundSum(logs, sk);
    int end = upperBoundSum(logs, ek) - 1;
    if (start < 0) 
        start = 0;
    if (end >= (int)logs.size()) 
        end = (int)logs.size() - 1;
    for (int i = start; i <= end; ++i) 
        cout << logs[i].originLine << '\n';

    return 0;
}
