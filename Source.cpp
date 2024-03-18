#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void createFiles(string volumePath, int volumeSize, int clusterSize) {
    int totalClusters = volumeSize / clusterSize;
    int fileCount = 0;
    int clusterCount = 0;

    while (clusterCount < totalClusters && fileCount < 100) {
        string filename = volumePath + "/F" + to_string(fileCount) + ".Dat";
        ofstream file(filename); 

        if (!file.is_open()) {
            cout << "Error creating file: " << filename << endl;
            return;
        }

        int clustersNeeded = 4 - (fileCount % 4);
        int linesPerCluster = clusterSize / sizeof(int); // Number of lines per cluster

        for (int i = 0; i < clustersNeeded * linesPerCluster; ++i) {
            int value = 2020 + fileCount;
            file << value << endl;
        }

        clusterCount += clustersNeeded;

        fileCount++;
    }

    cout << "Files created successfully on volume at path: " << volumePath << endl;
}

int main() {
    // Define volumes
    string volumePaths[] = { "A:\\", "B:\\", "E:\\" };
    int volumeSizes[] = { 30 * 1024 * 1024, 50 * 1024 * 1024, 97 * 1024 * 1024 };
    int clusterSizes[] = { 8, 4, 2 };

    // Create files on each volume
    for (int i = 0; i < 3; ++i) {
        createFiles(volumePaths[i], volumeSizes[i], clusterSizes[i] * 512); // Convert sector to bytes
    }

    return 0;
}
