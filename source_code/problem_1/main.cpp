#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <algorithm>

using namespace std;

struct Cell
{
    int r;
    int c;
    Cell(int r = 0, int c = 0) : r(r), c(c) {}
    
    bool operator<(const Cell& other) const
    {
        if (r == other.r) return c < other.c;
        return r < other.r;
    }
    
    bool operator==(const Cell& other) const
    {
        return r == other.r && c == other.c;
    }
};

class WaterOptimization
{
private:
    int H, W, R;
    int Pp, Pt, B;
    Cell backboneStart;
    vector<string> grid;
    vector<vector<bool>> connected;
    vector<vector<bool>> covered;
    vector<Cell> backboneCells;
    vector<Cell> tankCells;
    
    int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    int totalTargetCells;
    int coveredTargetCells;
    int totalCost;
    int remainingBudget;
    int score;
    double coveragePercentage;
    double budgetUtilization;

public:
    WaterOptimization()
    {
        H = 0; W = 0; R = 0;
        Pp = 0; Pt = 0; B = 0;
        totalTargetCells = 0;
        coveredTargetCells = 0;
        totalCost = 0;
        remainingBudget = 0;
        score = 0;
        coveragePercentage = 0.0;
        budgetUtilization = 0.0;
    }

    void readInput(string filename)
    {
        ifstream fin(filename);
        if (!fin.is_open())
        {
            cout << "Error opening file: " << filename << endl;
            return;
        }

        fin >> H >> W >> R;
        fin >> Pp >> Pt >> B;
        fin >> backboneStart.r >> backboneStart.c;
        
        grid.resize(H);
        string line;
        getline(fin, line);
        
        for (int i = 0; i < H; i++)
        {
            getline(fin, grid[i]);
        }
        
        fin.close();
        
        connected.resize(H, vector<bool>(W, false));
        covered.resize(H, vector<bool>(W, false));
        
        countTargetCells();
    }

    void countTargetCells()
    {
        totalTargetCells = 0;
        for (int r = 0; r < H; r++)
        {
            for (int c = 0; c < W; c++)
            {
                if (grid[r][c] == '.')
                {
                    totalTargetCells++;
                }
            }
        }
    }

    bool isValid(int r, int c)
    {
        return r >= 0 && r < H && c >= 0 && c < W;
    }

    bool isObstacle(int r, int c)
    {
        return grid[r][c] == '#';
    }

    bool isTarget(int r, int c)
    {
        return grid[r][c] == '.';
    }

    bool hasLineOfSight(int r1, int c1, int r2, int c2)
    {
        int min_r = min(r1, r2);
        int max_r = max(r1, r2);
        int min_c = min(c1, c2);
        int max_c = max(c1, c2);
        
        for (int r = min_r; r <= max_r; r++)
        {
            for (int c = min_c; c <= max_c; c++)
            {
                if (isObstacle(r, c))
                {
                    return false;
                }
            }
        }
        return true;
    }

    void expandBackbone()
    {
        queue<Cell> q;
        connected[backboneStart.r][backboneStart.c] = true;
        q.push(backboneStart);
        
        while (!q.empty())
        {
            Cell current = q.front();
            q.pop();
            
            for (int i = 0; i < 8; i++)
            {
                int nr = current.r + dr[i];
                int nc = current.c + dc[i];
                
                if (isValid(nr, nc) && !isObstacle(nr, nc) && !connected[nr][nc])
                {
                    connected[nr][nc] = true;
                    q.push(Cell(nr, nc));
                    backboneCells.push_back(Cell(nr, nc));
                }
            }
        }
    }

    int calculateCoverageScore(int r, int c)
    {
        int score = 0;
        for (int dr = -R; dr <= R; dr++)
        {
            for (int dc = -R; dc <= R; dc++)
            {
                int nr = r + dr;
                int nc = c + dc;
                if (isValid(nr, nc) && isTarget(nr, nc) && hasLineOfSight(r, c, nr, nc))
                {
                    score++;
                }
            }
        }
        return score;
    }

    void optimizePlacement()
    {
        expandBackbone();
        
        vector<pair<int, Cell>> candidates;
        
        for (int r = 0; r < H; r++)
        {
            for (int c = 0; c < W; c++)
            {
                if (connected[r][c] && !isObstacle(r, c))
                {
                    int score = calculateCoverageScore(r, c);
                    if (score > 0)
                    {
                        candidates.push_back(make_pair(score, Cell(r, c)));
                    }
                }
            }
        }
        
        sort(candidates.begin(), candidates.end(), [](const pair<int, Cell>& a, const pair<int, Cell>& b) {
            return a.first > b.first;
        });
        
        int currentCost = 0;
        for (int i = 0; i < candidates.size(); i++)
        {
            if (currentCost + Pt <= B)
            {
                Cell cell = candidates[i].second;
                tankCells.push_back(cell);
                currentCost += Pt;
                
                for (int dr = -R; dr <= R; dr++)
                {
                    for (int dc = -R; dc <= R; dc++)
                    {
                        int nr = cell.r + dr;
                        int nc = cell.c + dc;
                        if (isValid(nr, nc) && isTarget(nr, nc) && hasLineOfSight(cell.r, cell.c, nr, nc))
                        {
                            covered[nr][nc] = true;
                        }
                    }
                }
            }
        }
        
        calculateStatistics();
    }

    void calculateStatistics()
    {
        coveredTargetCells = 0;
        for (int r = 0; r < H; r++)
        {
            for (int c = 0; c < W; c++)
            {
                if (isTarget(r, c) && covered[r][c])
                {
                    coveredTargetCells++;
                }
            }
        }
        
        int backboneCost = backboneCells.size() * Pp;
        int tankCost = tankCells.size() * Pt;
        totalCost = backboneCost + tankCost;
        remainingBudget = B - totalCost;
        score = 1000 * coveredTargetCells + remainingBudget;
        
        if (totalTargetCells > 0)
        {
            coveragePercentage = (coveredTargetCells * 100.0) / totalTargetCells;
        }
        else
        {
            coveragePercentage = 0.0;
        }
        
        if (B > 0)
        {
            budgetUtilization = (totalCost * 100.0) / B;
        }
        else
        {
            budgetUtilization = 0.0;
        }
    }

    void writeOutput(string filename)
    {
        ofstream fout(filename);
        if (!fout.is_open())
        {
            cout << "Error creating file: " << filename << endl;
            return;
        }
        
        fout << backboneCells.size() << endl;
        for (int i = 0; i < backboneCells.size(); i++)
        {
            fout << backboneCells[i].r << " " << backboneCells[i].c << endl;
        }
        
        fout << tankCells.size() << endl;
        for (int i = 0; i < tankCells.size(); i++)
        {
            fout << tankCells[i].r << " " << tankCells[i].c << endl;
        }
        
        fout.close();
    }

    void printStatistics(string filename)
    {
        cout << "\n==========================================" << endl;
        cout << "STATISTICS FOR: " << filename << endl;
        cout << "==========================================" << endl;
        
        cout << "GRID INFO:" << endl;
        cout << "  Size: " << H << "x" << W << endl;
        cout << "  Tank Radius: " << R << endl;
        cout << "  Target Cells: " << totalTargetCells << endl;
        
        cout << "\nBUDGET:" << endl;
        cout << "  Pipe Cost: " << Pp << endl;
        cout << "  Tank Cost: " << Pt << endl;
        cout << "  Total Budget: " << B << endl;
        
        cout << "\nRESULTS:" << endl;
        cout << "  Covered: " << coveredTargetCells << "/" << totalTargetCells << endl;
        cout << "  Coverage: " << coveragePercentage << "%" << endl;
        cout << "  Backbone Cells: " << backboneCells.size() << endl;
        cout << "  Tanks: " << tankCells.size() << endl;
        cout << "  Total Cost: " << totalCost << "/" << B << endl;
        cout << "  Budget Used: " << budgetUtilization << "%" << endl;
        cout << "  Remaining: " << remainingBudget << endl;
        cout << "  SCORE: " << score << endl;
        
        cout << "\nPERFORMANCE:" << endl;
        if (coveragePercentage >= 90)
        {
            cout << "  Coverage: EXCELLENT" << endl;
        }
        else if (coveragePercentage >= 70)
        {
            cout << "  Coverage: GOOD" << endl;
        }
        else if (coveragePercentage >= 50)
        {
            cout << "  Coverage: FAIR" << endl;
        }
        else
        {
            cout << "  Coverage: POOR" << endl;
        }
        
        if (budgetUtilization >= 95)
        {
            cout << "  Budget: OPTIMAL" << endl;
        }
        else if (budgetUtilization >= 80)
        {
            cout << "  Budget: EFFICIENT" << endl;
        }
        else if (budgetUtilization >= 60)
        {
            cout << "  Budget: MODERATE" << endl;
        }
        else
        {
            cout << "  Budget: UNDERUSED" << endl;
        }
        
    }

    void solve(string inputFile, string outputFile)
    {
        cout << "\nProcessing: " << inputFile << " -> " << outputFile << endl;
        
        readInput(inputFile);
        optimizePlacement();
        writeOutput(outputFile);
        printStatistics(inputFile);
    }
};

int main()
{
    WaterOptimization solver;
    
    cout << "WATER OPTIMIZATION PROGRAM" << endl;    
    for (int i = 1; i <= 5; i++)
    {
        string inputFile = "input/input" + to_string(i) + ".in";
        string outputFile = "output/output" + to_string(i) + ".out";
        solver.solve(inputFile, outputFile);
    }
    
    cout << "\nALL FILES PROCESSED!" << endl;
    cout << "Output files saved in output folder" << endl;
    
    return 0;
}