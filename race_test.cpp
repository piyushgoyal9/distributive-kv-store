
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cassert>

using namespace std;

int counter = 0;
mutex counterMutex;

void increment()
{
    for (int i = 0; i < 1000000; i++)
    {
        lock_guard<mutex> lock(counterMutex);
        counter++;
    }
}

int main()
{
    cout << "Starting thread safety test...\n";

    thread t1(increment);
    thread t2(increment);

    t1.join();
    t2.join();

    cout << "Expected counter: 2000000\n";
    cout << "Actual counter:   " << counter << endl;

    assert(counter == 2000000);

    cout << "\nTHREAD SAFETY TEST PASSED!\n";

    return 0;
}
