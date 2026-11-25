
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

using namespace std;

int main()
{
    const string filename = "test_database.txt";

    // TEST 1: Write data
    {
        ofstream file(filename);

        assert(file.is_open());

        file << "name Piyush\n";
        file << "project KVStore\n";
        file << "language Cpp\n";

        file.close();
    }

    cout << "TEST 1 PASSED: Data written successfully\n";


    // TEST 2: Read data
    {
        ifstream file(filename);

        assert(file.is_open());

        string key, value;
        int count = 0;

        while (file >> key >> value)
        {
            count++;

            if (key == "name")
                assert(value == "Piyush");

            else if (key == "project")
                assert(value == "KVStore");

            else if (key == "language")
                assert(value == "Cpp");
        }

        assert(count == 3);

        file.close();
    }

    cout << "TEST 2 PASSED: Data read successfully\n";


    // TEST 3: Verify file exists
    {
        ifstream file(filename);

        assert(file.good());

        file.close();
    }

    cout << "TEST 3 PASSED: File verification\n";


    // TEST 4: Missing file
    {
        ifstream file("nonexistent_test_file.txt");

        assert(!file.is_open());
    }

    cout << "TEST 4 PASSED: Missing file handled\n";

    cout << "\nALL PERSISTENCE TESTS PASSED!\n";

    return 0;
}
