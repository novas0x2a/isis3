#include "CubeManager.h"
#include "Preference.h"
#include "Cube.h"
#include "Filename.h"

using namespace std;
using namespace Isis;

int main (int argc, char *argv[]) {
  Isis::Preference::Preferences(true);

  std::cout << "CubeManager Unit Test" << std::endl;
  std::vector<Cube *> cubes;

  // Read Cubes Into Memory
  cubes.push_back(CubeManager::Open("$base/testData/isisTruth.cub"));
  cubes.push_back(CubeManager::Open("$base/testData/isisTruth.cub"));
  cubes.push_back(CubeManager::Open("$base/testData/blobTruth.cub"));
  cubes.push_back(CubeManager::Open("$base/testData/blobTruth.cub"));
  cubes.push_back(CubeManager::Open("$base/testData/isisTruth.cub"));

  // Print Cube Filenames To Verify We Have Correct Ones
  std::cout << "Verify proper cubes have been read" << std::endl;
  std::cout << "Cube Filenames: " << std::endl;

  for(int i = 0; i < (int)cubes.size(); i++) {
    std::cout << "  " << i+1 << " : " << Filename(cubes[i]->Filename()).Basename() << std::endl;
  }

  std::cout << std::endl;

  // Print Cube Pointer Comparisons (1 if match, 0 if differ)
  std::cout << "Verify We Don't Have Duplicates..." << std::endl;

  // Print Table Header... up to 9 (for formatting)
  std::cout << "  Cube #";
  for(int i = 0; i < 9 && i < (int)cubes.size(); i++) {
    std::cout << " | Equals " << i+1;
  }
  std::cout << std::endl;

  // Print Comparison Table Data
  for(int i = 0; i < (int)cubes.size(); i++) {
    std::cout << "  Cube " << i+1;

    for(int j = 0; j < (int)cubes.size(); j++) {
      std::cout << " |        " << (int)(cubes[i] == cubes[j]);
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
}
