#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;

int marker_amount;
int array_size;
int *array;

HANDLE *cant_continue;
HANDLE *stop_thread;
HANDLE *continue_thread;
HANDLE start;
CRITICAL_SECTION cs;

void WINAPI marker(int num) {
    WaitForSingleObject(start, INFINITE);
    srand(num);
    vector<int> marked;

    while (true) {
        int i = rand() % array_size;
        if (array[i] == 0) {
            marked.push_back(i);
            Sleep(5);
            array[i] = num;
            Sleep(5);
        } else {
            EnterCriticalSection(&cs);
            cout << "Thread number " << num << endl << "Elements marked: " << i << endl << "Not marked: "
            << i << endl << endl;
            LeaveCriticalSection(&cs);
            SetEvent(cant_continue[num]);

            HANDLE _events[2];

            _events[0] = stop_thread[num];
            _events[1] = continue_thread[num];
            WaitForMultipleObjects(2, _events, FALSE, INFINITE);
            if (WaitForSingleObject(stop_thread[num], 0) == WAIT_OBJECT_0) {
                for (int j = 0; j < marked.size(); j++) {
                    array[marked[j]] = 0;
                }
                return;
            }
            ResetEvent(continue_thread[num]);
            ResetEvent(cant_continue[num]);
        }
    }
}

int main() {
    InitializeCriticalSection(&cs);
    srand(time(0));
    cout << "Enter array size: " << endl;
    cin >> array_size;
    array = new int[array_size];
    for (int i = 0; i < array_size; i++) {
        array[i] = 0;
    }
    cout << "Enter amount of threads: " << endl;
    cin >> marker_amount;

    HANDLE *_threads = new HANDLE[marker_amount];
    DWORD *IDs = new DWORD[marker_amount];
    cant_continue = new HANDLE[marker_amount];
    stop_thread = new HANDLE[marker_amount];
    continue_thread = new HANDLE[marker_amount];
    start = CreateEvent(NULL, TRUE, FALSE, NULL);
    for (int i = 0; i < marker_amount; i++) {
        cant_continue[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        stop_thread[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        continue_thread[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        _threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) marker, (LPVOID) i, 0, &IDs[i]);
    }

    SetEvent(start);

    while (WaitForMultipleObjects(marker_amount, _threads, TRUE, 0) != WAIT_OBJECT_0) {
        WaitForMultipleObjects(marker_amount, cant_continue, TRUE, INFINITE);
        EnterCriticalSection(&cs);
        for (int i = 0; i < array_size; i++) {
            cout << array[i] << " ";
        } cout << endl;
        int finishT;
        cout << "Choose thread which should be finished:" << endl;
        cin >> finishT;
        if (finishT < 0 || finishT > marker_amount - 1) {
            cout << "No such stream exists!" << endl;
            system("pause");
        }
        LeaveCriticalSection(&cs);
        SetEvent(stop_thread[finishT]);
        WaitForSingleObject(_threads[finishT], INFINITE);
        for (int i = 0; i < array_size; i++) {
            if (i != finishT) {
                SetEvent(continue_thread[i]);
            }
        }
        EnterCriticalSection(&cs);
        for (int i = 0; i < array_size; i++) {
            cout << array[i] << " ";
        } cout << endl;
        LeaveCriticalSection(&cs);
    }

    system("pause");

    delete[] array;

    return 0;
}