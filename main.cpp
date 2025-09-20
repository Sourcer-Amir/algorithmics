//I read that we musn't use libraries
//We can use the standard libraries like <iostream> and <vector>, also these ones 
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

struct entry{
    int month; //It's an integer between 1 and 12
    int day;
    int hour;
    int minute;
    int second;
    int ip1, ip2, ip3, ip4; //Cant be a string because we need to compare them
    int port;
    string reason;
    string originLine; //I think this will be useful for printing the querie
}

//I re-use the merge sort of the last code
void merge(int a[], int l, int m, int r, long long &comp ){
    int n1 = m - l + 1;
    int n2 = r - m;

    int *L = new int[n1];
    int *R = new int[n2];

    for (int i = 0; i < n1; ++i) L[i] = a[l + i];
    for (int j = 0; j < n2; ++j) R[j] = a[m + 1 + j];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        comp++;                    
        if (L[i] <= R[j]) a[k++] = L[i++];
        else               a[k++] = R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];

    delete[] L; delete[] R;
}

void mergeSort(int a[], int l, int r, long long &comp){
    if (l >= r) return;
    int m = l + (r - l) / 2;
    mergeSort(a, l, m, comp);
    mergeSort(a, m + 1, r, comp);
    merge(a, l, m, r, comp);
}

void ordenaMerge(int a[], int n, long long &comp) {
    comp = 0; 
    if (n > 0) mergeSort(a, 0, n - 1, comp);
}

int main(){
    
    return 0;
}


//Implement the quick sort algorithm in C++.