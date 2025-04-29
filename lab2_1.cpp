#include <iostream>
#include <vector>
#include <chrono>
#include <windows.h>
#include <climits>  // Для INT_MAX
#include <cstdlib>  // Для функції rand()
#include <ctime>    // Для ініціалізації rand()

using namespace std;  // Оголошення простору імен

int main() {
    SetConsoleOutputCP(1251);
    srand(time(0));
    int n;
    cout << "Введіть розмір масиву: ";
    cin >> n;

    vector<int> arr(n);
    for (int i = 0; i < n; ++i)
    {
        arr[i] = rand() % 100;
    }

    int sum = 0;
    int min = INT_MAX;

    auto start = chrono::high_resolution_clock::now();

    for (int num : arr)
    {
        if (num % 2 != 0)
        {
            sum += num;
            if (num < min)
            {
                min = num;
            }
        }
    }
    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Час виконання: " << elapsed.count() << " мілісекунд." << endl;

    return 0;
}
