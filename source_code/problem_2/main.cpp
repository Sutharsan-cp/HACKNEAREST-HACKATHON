#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <direct.h>
#include <limits>
using namespace std;

struct Connection {
    int serverId;
    int latency;
};

struct Request {
    int assetId;
    int regionId;
    int count;
};

struct Region {
    int latencyCentral;
    vector<Connection> connections;
};

struct AssetValue {
    int assetId;
    int serverId;
    double ratio;      
    long long value;   
};

// read input file
void processFile(const string &inputFile, const string &outputFile) {
    ifstream fin(inputFile);
    if (!fin) {
        cerr << "Could not open input file: " << inputFile << "\n";
        return;
    }

    int A, P, R, G, X;
    fin >> A >> P >> R >> G >> X;

    vector<int> sizes(A);
    for (int i = 0; i < A; i++) fin >> sizes[i];

    vector<Region> regions(P);
    for (int p = 0; p < P; p++) {
        int LD, K;
        fin >> LD >> K;
        regions[p].latencyCentral = LD;
        for (int j = 0; j < K; j++) {
            int g, lr;
            fin >> g >> lr;
            regions[p].connections.push_back({g, lr});
        }
    }

    vector<Request> requests(R);
    for (int i = 0; i < R; i++) {
        fin >> requests[i].assetId >> requests[i].regionId >> requests[i].count;
    }
    fin.close();


    map<pair<int,int>, long long> valueMap;
    for (auto &req : requests) {
        int a = req.assetId;
        int p = req.regionId;
        int n = req.count;
        int centralLatency = regions[p].latencyCentral;

        for (auto &conn : regions[p].connections) {
            int g = conn.serverId;
            int lr = conn.latency;
            long long saving = (long long)(centralLatency - lr) * n;
            if (saving > 0 && sizes[a] <= X) {
                valueMap[{a,g}] += saving;
            }
        }
    }


    vector<AssetValue> assetValues;
    for (auto &kv : valueMap) {
        int a = kv.first.first;
        int g = kv.first.second;
        long long val = kv.second;
        if (val > 0) {
            double ratio = (double)val / sizes[a];
            assetValues.push_back({a, g, ratio, val});
        }
    }

    // Sort ratio
    sort(assetValues.begin(), assetValues.end(), [](const AssetValue &x, const AssetValue &y) {
        return x.ratio > y.ratio;
    });

    vector<vector<int>> bestLatency(P, vector<int>(A, numeric_limits<int>::max()));

    vector<int> usedCap(G, 0);
    vector<vector<int>> servers(G);

    for (auto &av : assetValues) {
        int a = av.assetId;
        int g = av.serverId;

        bool useful = false;
        for (int p = 0; p < P; p++) {
            for (auto &conn : regions[p].connections) {
                if (conn.serverId == g) {
                    if (conn.latency < bestLatency[p][a]) {
                        useful = true;
                        break;
                    }
                }
            }
            if (useful) break;
        }

        if (!useful) continue; 

        if (usedCap[g] + sizes[a] <= X) {
            if (find(servers[g].begin(), servers[g].end(), a) == servers[g].end()) {
                servers[g].push_back(a);
                usedCap[g] += sizes[a];

                for (int p = 0; p < P; p++) {
                    for (auto &conn : regions[p].connections) {
                        if (conn.serverId == g) {
                            bestLatency[p][a] = min(bestLatency[p][a], conn.latency);
                        }
                    }
                }
            }
        }
    }

    // store op in output file
    ofstream fout(outputFile);
    int usedServers = 0;
    for (int g = 0; g < G; g++) {
        if (!servers[g].empty()) usedServers++;
    }

    fout << usedServers << "\n";
    for (int g = 0; g < G; g++) {
        if (!servers[g].empty()) {
            fout << g;
            for (int a : servers[g]) fout << " " << a;
            fout << "\n";
        }
    }
    fout.close();
}

int main() {
    string inputDir = "input/";
    string outputDir = "output/";
    _mkdir(outputDir.c_str());

    int idx = 1;
    while (true) {
        string inFile = inputDir + "input" + to_string(idx) + ".in";
        ifstream test(inFile);
        if (!test) break;
        test.close();

        string outFile = outputDir + "output" + to_string(idx) + ".out";
        cout << "Processing " << inFile << " -> " << outFile << "\n";
        processFile(inFile, outFile);
        idx++;
    }

    if (idx == 1) {
        cout << "No input files found.\n";
    }
    return 0;
}
