#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <iomanip>
#include <cmath>


using namespace std;

struct Net 
{
    int num_cells;  // Number of connected cells
    vector<int> connected_nets;  // IDs of connected nets
};

struct position {
    int x; 
    int y;
};

vector<vector<int>> parse(string filename , vector<Net>& nets, int& rows, int& columns, int& cellnum, int& netnum )
{
    ifstream myfile(filename);
    string line;
    string num;

    getline(myfile, line);
    stringstream lineStream(line);
    for (int i = 0; i < 4; i++)
    {
        getline(lineStream, num, ' ');
        if (i == 0)
        {
            cellnum = stoi(num);
        }
        else if (i == 1)
        {
            netnum = stoi(num);
        }
        else if (i == 2)
        {
            rows = stoi(num);
        }
        else if (i == 3)
        {
            columns = stoi(num);
        }
    }

    vector<vector<int>> grid(rows, vector<int>(columns));
    srand(time(0));
    
    for (int i = 1; i <= cellnum; i++)
    {
        int a, b;
        do {
            a = rand() % rows;
            b = rand() % columns;
        } while (grid[a][b] != 0);

        grid[a][b] = i;
    }

    while (getline(myfile, line))
    {
        stringstream lineStream(line);
        getline(lineStream, num, ' ');
        Net mynet = { stoi(num)};
        while (getline(lineStream, num, ' '))
        {
            mynet.connected_nets.push_back(stoi(num));
        }
        nets.push_back(mynet);
    }

    return grid;
}

int wireLength(const vector<vector<int>>& grid, const vector<Net>& nets) {
    int totalwireLength = 0;
    unordered_map<int, position> placement;
    position pos;
    int minX, maxX, minY, maxY;

    for (int i = 0; i < grid.size(); i++) {
        for (int j = 0; j < grid[i].size(); j++) {
            if (grid[i][j] != 0) {
                pos.x = i;
                pos.y = j;
                placement[grid[i][j]] = pos;
            }
        }
    }

    for (const auto& net : nets) {
        minX = INT_MAX; maxX = INT_MIN; minY = INT_MAX; maxY = INT_MIN;
        for (int cellID : net.connected_nets) {
            minX = min(minX, placement[cellID].x);
            minY = min(minY, placement[cellID].y);
            maxX = max(maxX, placement[cellID].x);
            maxY = max(maxY, placement[cellID].y);
        }
        totalwireLength += maxX - minX + maxY - minY;
    }

    return totalwireLength;
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
            cout << setw(4) << grid[i][j];
        }
        cout << endl;
    }
}

void simulatedAnnealing(vector<vector<int>>& grid, vector<Net>& nets, int& totalwireLength) {
    ofstream outFile("annealing_data.csv");
    outFile << "Temperature,WireLength\n"; // CSV header

    totalwireLength = wireLength(grid, nets); // Calculate initial wire length
    cout << "Initial Wire Length: " << totalwireLength << endl;
    cout << "Initial Grid Configuration:" << endl;
    printGrid(grid);

    const double initialTemperature = 500 * totalwireLength;
    const double finalTemperature = 5e-6 * (initialTemperature / nets.size());
    const double coolingRate = 0.95;
    const int movesPerTemperature = 20 * grid.size() * grid[0].size();

    double temperature = initialTemperature;
    srand(time(0)); // Seed random number generator

    while (temperature > finalTemperature) {
        outFile << temperature << "," << totalwireLength << "\n"; // Log temperature and wire length
        
        for (int move = 0; move < movesPerTemperature; ++move) {
            int row1 = rand() % grid.size();
            int col1 = rand() % grid[0].size();
            int row2 = rand() % grid.size();
            int col2 = rand() % grid[0].size();

            swap(grid[row1][col1], grid[row2][col2]);

            int newWireLength = wireLength(grid, nets);
            int deltaL = newWireLength - totalwireLength;

            if (deltaL < 0) {
                totalwireLength = newWireLength; // Update wire length
            } else if (exp(-deltaL / temperature) > (double)rand() / RAND_MAX) {
                totalwireLength = newWireLength; // Update wire length
            } else {
                swap(grid[row1][col1], grid[row2][col2]); // Revert the swap
            }
        }

        temperature *= coolingRate;
    }

    outFile.close();
}


int main()
{
  
    // File containing input data
    string filename = "C:\\Users\\AUC\\Desktop\\d0.txt"; 

    // Variables to store grid dimensions and counts
    int rows = 0, columns = 0, cellnum = 0, netnum = 0;

    // Variable to store total wire length
    int totalwireLength = 0;

    // Vector to store the grid configuration
    vector<vector<int>> grid;

    // Vector to store net information
    vector<Net> nets;

    // Parse the input file and generate initial grid and net configurations
    grid = parse(filename, nets, rows, columns, cellnum, netnum);
    //totalwireLength= wireLength(grid, nets, totalwireLength);
    // Perform simulated annealing to minimize wire length


    simulatedAnnealing(grid, nets, totalwireLength);

    // Print the final wire length and grid configuration
    cout << "Final Wire Length: " << totalwireLength << endl;
    cout << "Final Grid Configuration:" << endl;
    printGrid(grid);

    return 0;
}