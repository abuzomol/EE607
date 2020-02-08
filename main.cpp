#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

using namespace std;

typedef vector<int> vi;
typedef pair<int, int> ii;
typedef vector<ii> vii;
typedef vector <vii> adjlist;
typedef vector< pair<int, ii>> edgelist;


int main() {
  ifstream fin("toplogy.txt");
  int nodes,edges;
  fin>>nodes >> edges;

  vector< vi > neighbours(nodes, vi(nodes));

  for(int i = 0 ; i< edges; i++)
  {
    int x, y, weight = 1;
    fin >> x >> y ;

    neighbours[x][y] = weight ;
    neighbours[y][x] = weight;
  }



  std::cout << "Hello, World!" << std::endl;
  return 0;
}
