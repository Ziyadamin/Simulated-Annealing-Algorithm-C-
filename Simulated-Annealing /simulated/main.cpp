#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <random>
#include <cmath>
#include <climits>
#include <chrono>

using namespace std;

struct Net
{
    int num_cells;  // Number of connected cells
    vector<int> connected_nets;
};

struct position {
    int x;
    int y;
};

vector<vector<int>> parse(const string& filename, vector<Net>& nets, int& rows, int& columns, int& cellnum, int& netnum, vector<position>& placement) {
    ifstream myfile(filename);
    string line;
    string num;

    getline(myfile, line);
    stringstream lineStream(line);
    for (int i = 0; i < 4; i++) {
        getline(lineStream, num, ' ');
        if (i == 0) {
            cellnum = stoi(num);
        } else if (i == 1) {
            netnum = stoi(num);
        } else if (i == 2) {
            rows = stoi(num);
        } else if (i == 3) {
            columns = stoi(num);
        }
    }
    vector<vector<int>> grid(rows, vector<int>(columns, 0));
    placement.resize(cellnum + 1);
    srand(time(0));

    vector<int> vec(rows * columns);
    random_device rd;
    mt19937 generator(rd());
    for (int i = 0; i < rows * columns; ++i) {
        vec[i] = i;
    }

    shuffle(vec.begin(), vec.end(), generator);

    for (int i = 1; i <= cellnum; i++) {
        int row = vec[i] / columns;
        int col = vec[i] % columns;
        grid[row][col] = i;
        position pos{row, col};
        placement[i] = pos;
    }

    while (getline(myfile, line)) {
        stringstream lineStream(line);
        getline(lineStream, num, ' ');
        Net mynet = {stoi(num)};
        while (getline(lineStream, num, ' ')) {
            mynet.connected_nets.push_back(stoi(num));
        }
        nets.push_back(mynet);
    }

    return grid;
}

int wireLength(const vector<Net>& nets, const vector<position>& placement) {
    int totalWireLength = 0;
    for (const auto& net : nets) {
        int minX = INT_MAX, maxX = INT_MIN;
        int minY = INT_MAX, maxY = INT_MIN;
        for (int cellID : net.connected_nets) {
            const auto& pos = placement[cellID];
            minX = min(minX, pos.x);
            maxX = max(maxX, pos.x);
            minY = min(minY, pos.y);
            maxY = max(maxY, pos.y);
        }
        totalWireLength += (maxX - minX) + (maxY - minY);
    }
    return totalWireLength;
}

void printGridB(const vector<vector<int>>& grid) {
    int rows = grid.size();
    if (rows == 0) return;
    int columns = grid[0].size();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            cout << (grid[i][j] == 0 ? " 1" : " 0");
        }
        cout << endl;
    }
}


void printGrid(const vector<vector<int>>& grid) {
    int rows = grid.size();
    if (rows == 0) {
        cout << "Grid is empty." << endl;
        return;
    }
    int columns = grid[0].size();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (grid[i][j] == 0)
                cout << " --";
            else
                cout << setw(4) << grid[i][j];
        }
        cout << endl;
    }
}

void simulatedAnnealing(vector<vector<int>>& grid, vector<Net>& nets, int& totalwireLength, vector<position>& placement) {
    totalwireLength = wireLength(nets, placement); // Calculate initial wire length
    int initialWireLength = totalwireLength;       // Store initial wire length

    const double initialTemperature = 500 * totalwireLength;
    const double finalTemperature = 5e-6 * (initialTemperature / nets.size());
    const double coolingRate = 0.95;
    const int movesPerTemperature = 20 * grid.size() * grid[0].size();

    double temperature = initialTemperature;
    srand(time(nullptr)); // Seed random number generator

    auto start = chrono::high_resolution_clock::now(); // Start time

    while (temperature > finalTemperature) {
        for (int move = 0; move < movesPerTemperature; ++move) {
            int row1 = rand() % grid.size();
            int col1 = rand() % grid[0].size();
            int row2 = rand() % grid.size();
            int col2 = rand() % grid[0].size();

            swap(grid[row1][col1], grid[row2][col2]);
            if (grid[row1][col1] != 0) placement[grid[row1][col1]] = {row1, col1};
            if (grid[row2][col2] != 0) placement[grid[row2][col2]] = {row2, col2};

            int newWireLength = wireLength(nets, placement);
            int deltaL = newWireLength - totalwireLength;

            if (deltaL < 0) {
                totalwireLength = newWireLength; // Update wire length
            } else if (exp(-deltaL / temperature) > static_cast<double>(rand()) / RAND_MAX) {
                totalwireLength = newWireLength; // Update wire length
            } else {
                swap(grid[row1][col1], grid[row2][col2]); // Revert the swap
                if (grid[row1][col1] != 0) placement[grid[row1][col1]] = {row1, col1};
                if (grid[row2][col2] != 0) placement[grid[row2][col2]] = {row2, col2};
            }
        }
        temperature *= coolingRate;
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Initial Wire Length: " << initialWireLength << endl;
    cout << "Time taken for Simulated Annealing: " << duration.count() << " milliseconds" << endl;
}

int main() {
    string txtfile;
    cout << "Input file: ";
    cin >> txtfile;
    string filename = "/Users/amanywaleed/Desktop/simulated/" + txtfile;
    int rows = 0, columns = 0, cellnum = 0, netnum = 0;

    int totalWireLength = 0;

    vector<vector<int>> grid;
    vector<Net> nets;
    vector<position> placement;
    grid = parse(filename, nets, rows, columns, cellnum, netnum, placement);

    cout << "Initial Grid Configuration:" << endl;
    printGrid(grid);
    cout << "Initial Grid Configuration:" << endl;
    printGridB(grid);
    cout << endl;
    simulatedAnnealing(grid, nets, totalWireLength, placement);
    cout << "Final Wire Length: " << totalWireLength << endl;
    cout << "Final Grid Configuration:" << endl;
    printGrid(grid);
    cout << "Final Grid Configuration:" << endl;
    printGridB(grid);

    return 0;
}


//
////TEMP VS TWL
//#include <iostream>
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <vector>
//#include <ctime>
//#include <cstdlib>
//#include <algorithm>
//#include <iomanip>
//#include <random>
//#include <cmath>
//#include <climits>
//#include <chrono>
//
//using namespace std;
//
//struct Net {
//    int num_cells;  // Number of connected cells
//    vector<int> connected_nets;
//};
//
//struct position {
//    int x;
//    int y;
//};
//
//vector<vector<int>> parse(const string& filename, vector<Net>& nets, int& rows, int& columns, int& cellnum, int& netnum, vector<position>& placement) {
//    ifstream myfile(filename);
//    string line;
//    string num;
//
//    getline(myfile, line);
//    stringstream lineStream(line);
//    for (int i = 0; i < 4; i++) {
//        getline(lineStream, num, ' ');
//        if (i == 0) {
//            cellnum = stoi(num);
//        } else if (i == 1) {
//            netnum = stoi(num);
//        } else if (i == 2) {
//            rows = stoi(num);
//        } else if (i == 3) {
//            columns = stoi(num);
//        }
//    }
//    vector<vector<int>> grid(rows, vector<int>(columns, 0));
//    placement.resize(cellnum + 1);
//    srand(time(0));
//
//    vector<int> vec(rows * columns);
//    random_device rd;
//    mt19937 generator(rd());
//    for (int i = 0; i < rows * columns; ++i) {
//        vec[i] = i;
//    }
//
//    shuffle(vec.begin(), vec.end(), generator);
//
//    for (int i = 1; i <= cellnum; i++) {
//        int row = vec[i] / columns;
//        int col = vec[i] % columns;
//        grid[row][col] = i;
//        position pos{row, col};
//        placement[i] = pos;
//    }
//
//    while (getline(myfile, line)) {
//        stringstream lineStream(line);
//        getline(lineStream, num, ' ');
//        Net mynet = {stoi(num)};
//        while (getline(lineStream, num, ' ')) {
//            mynet.connected_nets.push_back(stoi(num));
//        }
//        nets.push_back(mynet);
//    }
//
//    return grid;
//}
//
//int wireLength(const vector<Net>& nets, const vector<position>& placement) {
//    int totalWireLength = 0;
//    for (const auto& net : nets) {
//        int minX = INT_MAX, maxX = INT_MIN;
//        int minY = INT_MAX, maxY = INT_MIN;
//        for (int cellID : net.connected_nets) {
//            const auto& pos = placement[cellID];
//            minX = min(minX, pos.x);
//            maxX = max(maxX, pos.x);
//            minY = min(minY, pos.y);
//            maxY = max(maxY, pos.y);
//        }
//        totalWireLength += (maxX - minX) + (maxY - minY);
//    }
//    return totalWireLength;
//}
//
//void printGridB(const vector<vector<int>>& grid) {
//    int rows = grid.size();
//    if (rows == 0) return;
//    int columns = grid[0].size();
//    for (int i = 0; i < rows; ++i) {
//        for (int j = 0; j < columns; ++j) {
//            cout << (grid[i][j] == 0 ? " 1" : " 0");
//        }
//        cout << endl;
//    }
//}
//
//void printGrid(const vector<vector<int>>& grid) {
//    int rows = grid.size();
//    if (rows == 0) {
//        cout << "Grid is empty." << endl;
//        return;
//    }
//    int columns = grid[0].size();
//
//    for (int i = 0; i < rows; ++i) {
//        for (int j = 0; j < columns; ++j) {
//            if (grid[i][j] == 0)
//                cout << " --";
//            else
//                cout << setw(4) << grid[i][j];
//        }
//        cout << endl;
//    }
//}
//
//void simulatedAnnealing(vector<vector<int>>& grid, vector<Net>& nets, int& totalwireLength, vector<position>& placement, double coolingRate, ofstream& outFile) {
//    totalwireLength = wireLength(nets, placement); // Calculate initial wire length
//    int initialWireLength = totalwireLength;       // Store initial wire length
//
//    const double initialTemperature = 500 * totalwireLength;
//    const double finalTemperature = 5e-6 * (initialTemperature / nets.size());
//    const int movesPerTemperature = 20 * grid.size() * grid[0].size();
//
//    double temperature = initialTemperature;
//    srand(time(nullptr)); // Seed random number generator
//
//    auto start = chrono::high_resolution_clock::now(); // Start time
//
//    outFile << "Temperature,TWL\n"; // CSV header
//
//    while (temperature > finalTemperature) {
//        for (int move = 0; move < movesPerTemperature; ++move) {
//            int row1 = rand() % grid.size();
//            int col1 = rand() % grid[0].size();
//            int row2 = rand() % grid.size();
//            int col2 = rand() % grid[0].size();
//
//            swap(grid[row1][col1], grid[row2][col2]);
//            if (grid[row1][col1] != 0) placement[grid[row1][col1]] = {row1, col1};
//            if (grid[row2][col2] != 0) placement[grid[row2][col2]] = {row2, col2};
//
//            int newWireLength = wireLength(nets, placement);
//            int deltaL = newWireLength - totalwireLength;
//
//            if (deltaL < 0) {
//                totalwireLength = newWireLength; // Update wire length
//            } else if (exp(-deltaL / temperature) > static_cast<double>(rand()) / RAND_MAX) {
//                totalwireLength = newWireLength; // Update wire length
//            } else {
//                swap(grid[row1][col1], grid[row2][col2]); // Revert the swap
//                if (grid[row1][col1] != 0) placement[grid[row1][col1]] = {row1, col1};
//                if (grid[row2][col2] != 0) placement[grid[row2][col2]] = {row2, col2};
//            }
//        }
//        outFile << temperature << "," << totalwireLength << "\n"; // Log temperature and wire length
//        temperature *= coolingRate;
//    }
//
//    auto end = chrono::high_resolution_clock::now();
//    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
//
//    cout << "Initial Wire Length: " << initialWireLength << endl;
//    cout << "Time taken for Simulated Annealing: " << duration.count() << " milliseconds" << endl;
//}
//
//int main() {
//    string txtfile;
//    cout << "Input file: ";
//    cin >> txtfile;
//    string filename = "/Users/amanywaleed/Desktop/simulated/" + txtfile;
//            int rows = 0, columns = 0, cellnum = 0, netnum = 0;
//            int totalWireLength = 0;
//            vector<vector<int>> grid;
//            vector<Net> nets;
//            vector<position> placement;
//            grid = parse(filename, nets, rows, columns, cellnum, netnum, placement);
//            string tempOutFilename = filename + "_temp_output.csv";
//            ofstream tempCsvfile(tempOutFilename);
//            double rate=0.95;
//            simulatedAnnealing(grid, nets, totalWireLength, placement, rate, tempCsvfile);
//
//            tempCsvfile.close();
//    cout<<"done";
//    return 0;
//}

//
////COOLING RATE VS TWL
//#include <iostream>
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <vector>
//#include <ctime>
//#include <cstdlib>
//#include <algorithm>
//#include <iomanip>
//#include <random>
//#include <cmath>
//#include <climits>
//#include <chrono>
//
//using namespace std;
//
//struct Net
//{
//    int num_cells;  // Number of connected cells
//    vector<int> connected_nets;
//};
//
//struct position {
//    int x;
//    int y;
//};
//
//vector<vector<int>> parse(const string& filename, vector<Net>& nets, int& rows, int& columns, int& cellnum, int& netnum, vector<position>& placement) {
//    ifstream myfile(filename);
//    string line;
//    string num;
//
//    getline(myfile, line);
//    stringstream lineStream(line);
//    for (int i = 0; i < 4; i++) {
//        getline(lineStream, num, ' ');
//        if (i == 0) {
//            cellnum = stoi(num);
//        } else if (i == 1) {
//            netnum = stoi(num);
//        } else if (i == 2) {
//            rows = stoi(num);
//        } else if (i == 3) {
//            columns = stoi(num);
//        }
//    }
//    vector<vector<int>> grid(rows, vector<int>(columns, 0));
//    placement.resize(cellnum + 1);
//    srand(time(0));
//
//    vector<int> vec(rows * columns);
//    random_device rd;
//    mt19937 generator(rd());
//    for (int i = 0; i < rows * columns; ++i) {
//        vec[i] = i;
//    }
//
//    shuffle(vec.begin(), vec.end(), generator);
//
//    for (int i = 1; i <= cellnum; i++) {
//        int row = vec[i] / columns;
//        int col = vec[i] % columns;
//        grid[row][col] = i;
//        position pos{row, col};
//        placement[i] = pos;
//    }
//
//    while (getline(myfile, line)) {
//        stringstream lineStream(line);
//        getline(lineStream, num, ' ');
//        Net mynet = {stoi(num)};
//        while (getline(lineStream, num, ' ')) {
//            mynet.connected_nets.push_back(stoi(num));
//        }
//        nets.push_back(mynet);
//    }
//
//    return grid;
//}
//
//int wireLength(const vector<Net>& nets, const vector<position>& placement) {
//    int totalWireLength = 0;
//    for (const auto& net : nets) {
//        int minX = INT_MAX, maxX = INT_MIN;
//        int minY = INT_MAX, maxY = INT_MIN;
//        for (int cellID : net.connected_nets) {
//            const auto& pos = placement[cellID];
//            minX = min(minX, pos.x);
//            maxX = max(maxX, pos.x);
//            minY = min(minY, pos.y);
//            maxY = max(maxY, pos.y);
//        }
//        totalWireLength += (maxX - minX) + (maxY - minY);
//    }
//    return totalWireLength;
//}
//
//void printGridB(const vector<vector<int>>& grid) {
//    int rows = grid.size();
//    if (rows == 0) return;
//    int columns = grid[0].size();
//    for (int i = 0; i < rows; ++i) {
//        for (int j = 0; j < columns; ++j) {
//            cout << (grid[i][j] == 0 ? " 1" : " 0");
//        }
//        cout << endl;
//    }
//}
//
//
//void printGrid(const vector<vector<int>>& grid) {
//    int rows = grid.size();
//    if (rows == 0) {
//        cout << "Grid is empty." << endl;
//        return;
//    }
//    int columns = grid[0].size();
//
//    for (int i = 0; i < rows; ++i) {
//        for (int j = 0; j < columns; ++j) {
//            if (grid[i][j] == 0)
//                cout << " --";
//            else
//                cout << setw(4) << grid[i][j];
//        }
//        cout << endl;
//    }
//}
//
//void simulatedAnnealing(vector<vector<int>>& grid, vector<Net>& nets, int& totalwireLength, vector<position>& placement,double coolingRate) {
//    totalwireLength = wireLength(nets, placement); // Calculate initial wire length
//    int initialWireLength = totalwireLength;       // Store initial wire length
//
//    const double initialTemperature = 500 * totalwireLength;
//    const double finalTemperature = 5e-6 * (initialTemperature / nets.size());
//  //  const double coolingRate = 0.95;
//    const int movesPerTemperature = 20 * grid.size() * grid[0].size();
//
//    double temperature = initialTemperature;
//    srand(time(nullptr)); // Seed random number generator
//
//    auto start = chrono::high_resolution_clock::now(); // Start time
//
//    while (temperature > finalTemperature) {
//        for (int move = 0; move < movesPerTemperature; ++move) {
//            int row1 = rand() % grid.size();
//            int col1 = rand() % grid[0].size();
//            int row2 = rand() % grid.size();
//            int col2 = rand() % grid[0].size();
//
//            swap(grid[row1][col1], grid[row2][col2]);
//            if (grid[row1][col1] != 0) placement[grid[row1][col1]] = {row1, col1};
//            if (grid[row2][col2] != 0) placement[grid[row2][col2]] = {row2, col2};
//
//            int newWireLength = wireLength(nets, placement);
//            int deltaL = newWireLength - totalwireLength;
//
//            if (deltaL < 0) {
//                totalwireLength = newWireLength; // Update wire length
//            } else if (exp(-deltaL / temperature) > static_cast<double>(rand()) / RAND_MAX) {
//                totalwireLength = newWireLength; // Update wire length
//            } else {
//                swap(grid[row1][col1], grid[row2][col2]); // Revert the swap
//                if (grid[row1][col1] != 0) placement[grid[row1][col1]] = {row1, col1};
//                if (grid[row2][col2] != 0) placement[grid[row2][col2]] = {row2, col2};
//            }
//        }
//        temperature *= coolingRate;
//    }
//
//    auto end = chrono::high_resolution_clock::now();
//    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
//
//    cout << "Initial Wire Length: " << initialWireLength << endl;
//    cout << "Time taken for Simulated Annealing: " << duration.count() << " milliseconds" << endl;
//}
//
//int main() {
//    string txtfile;
//    cout << "Input file: ";
//    cin >> txtfile;
//    string filename = "/Users/amanywaleed/Desktop/simulated/" + txtfile;
//    vector<double> coolingRates = {0.75, 0.8, 0.85, 0.9, 0.95};
//    const int numRuns = 5;  // Run each cooling rate only once
//
//    string outFilename = filename + "_output.csv";
//    ofstream csvfile(outFilename);
//
//    csvfile << "CoolingRate,TWL" << endl;
//
//    for (const auto& rate : coolingRates) {
//        for (int run = 0; run < numRuns; ++run) {
//            int rows = 0, columns = 0, cellnum = 0, netnum = 0;
//            int totalWireLength = 0;
//            vector<vector<int>> grid;
//            vector<Net> nets;
//            vector<position> placement;
//            grid = parse(filename, nets, rows, columns, cellnum, netnum, placement);
//            simulatedAnnealing(grid, nets, totalWireLength, placement, rate);
//            csvfile << rate << "," << totalWireLength << endl;
//        }
//    }
//
//    csvfile.close();
//    return 0;
//}
