#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
typedef vector<int> vi;

vi& getIthNeighbours(const vector<vi>& neighbours, int i);
void increaseCapacity(int k,
                      int j,
                      vector<vector<vi>>& preferredNeigbours,
                      vector<vector<double>>& equalCap,
                      double trafficKJ);

int main()
{
  ifstream finTop("topology.txt");
  int nodes, edges;
  finTop >> nodes >> edges;

  vector<vi> neighbours(nodes, vi(nodes));
  vector<vi> dist(nodes, vi(nodes));
  vector<vi> traffic(nodes, vi(nodes));
  vector<vector<vi>> preferredNeigbours(nodes, vector<vi>(nodes));
  vector<vi> pureCap(nodes, vi(nodes));
  vector<vector<double>> equalCap(nodes, vector<double>(nodes));

  // set max distance to infinity
  int infinity = 10000;
  // read edges from file to neighbours and distances
  for (int i = 0; i < edges; i++)
  {
    int x, y, weight = 1;
    finTop >> x >> y;
    neighbours[x][y] = weight;
    neighbours[y][x] = weight;

    dist[x][y] = weight;
    dist[y][x] = weight;
  }
  finTop.close();

  // set distances to be infinity
  for (int i = 0; i < nodes; i++)
  {
    for (int j = 0; j < nodes; j++)
    {
      if (dist[i][j] == 0 && i != j) dist[i][j] = infinity;
    }
  }
  // read traffic from file
  ifstream finTraf("traffic.txt");
  finTraf >> nodes >> edges;
  for (int i = 0; i < edges; i++)
  {
    int x, y, weight;
    finTraf >> x >> y >> weight;

    traffic[x][y] = weight;
    traffic[y][x] = weight;
  }
  finTraf.close();

  // Floyd Warshal algorithm (dynamic programming)
  for (int k = 0; k < nodes; k++)
  {
    for (int i = 0; i < nodes; i++)
    {
      for (int j = 0; j < nodes; j++)
      {
        if (dist[i][j] > dist[i][k] + dist[k][j])
        {
          dist[i][j] = dist[i][k] + dist[k][j];
        }
      }
    }
  }

  // get preferred neighbours for each node i in the graph to each destination j
  // the construction already sort out those neighbours according to their
  // indices
  for (int i = 0; i < nodes; i++)
  {
    for (int j = 0; j < nodes; j++)
    {
      vi iNeighbours = getIthNeighbours(neighbours, i);
      vi* ijPreferredNeighbours = new vi();
      for (int k = 0; k < iNeighbours.size(); k++)
      {
        int neighbour = iNeighbours[k];
        if (dist[i][neighbour] + dist[neighbour][j] == dist[i][j])
        {
          ijPreferredNeighbours->push_back(neighbour);
        }
      }
      preferredNeigbours[i][j] = *ijPreferredNeighbours;
    }
  }
  // compute traffic using Pure-SP.
  // Here, we sum up the traffic from source to destination based on
  // lexicographical order basically first element in preferredNeighbours list
  // i is source
  for (int i = 0; i < nodes - 1; i++)
  {
    // j is destination
    for (int j = i + 1; j < nodes; j++)
    {
      // case there is a link that leads to j
      if (preferredNeigbours[i][j].size() > 0)
      {
        int neighbour = preferredNeigbours[i][j][0];  // smallest neighbour
        pureCap[i][neighbour] += traffic[i][j];
        pureCap[neighbour][i] += traffic[i][j];
        // loop to add traffic in each link between i and j
        while (neighbour != j)
        {
          int nextNeighbour = preferredNeigbours[neighbour][j][0];
          pureCap[neighbour][nextNeighbour] += traffic[i][j];
          pureCap[nextNeighbour][neighbour] += traffic[i][j];
          neighbour = nextNeighbour;
        }
      }
    }
  }
  int sumLinkCapacities = 0;

  for (int i = 0; i < nodes - 1; i++)
  {
    for (int j = i + 1; j < nodes; j++) sumLinkCapacities += pureCap[i][j];
  }
  // output
 /* cout << "EE 697 Network Design" << endl;
  cout << "Topology: topology.txt" << endl;
  cout << "Traffic matrix: traffic.txt" << endl;
  cout << "Shortest path routing: Pure-SP" << endl;
  cout << "Sum of link capacities = " << sumLinkCapacities << endl;
  cout << "Link capacities:" << endl;
  for (int i = 0; i < nodes - 1; i++)
  {
    for (int j = i + 1; j < nodes; j++)
      if (pureCap[i][j] > 0)
        cout << "(" << i << ',' << j << "): " << pureCap[i][j] << endl;
  }*/

  // compute traffic using Pure-SP.
  // Here, we sum up the traffic from source to destination with equal load
  // shared between neighbours i is source
  for (int i = 0; i < nodes - 1; i++)
  {
    // j is destination
    for (int j = i + 1; j < nodes; j++)
    {
      // case there is a link that leads to j
      if (preferredNeigbours[i][j].size() > 0)
      {
        increaseCapacity(i, j, preferredNeigbours, equalCap, traffic[i][j]);
      }
    }
  }

  int sumEqualLinkCapacities = 0;
  int ceiledSumEqualLinkCapacities = 0;
  for (int i = 0; i < nodes - 1; i++)
  {
    for (int j = i + 1; j < nodes; j++)
    {
      ceiledSumEqualLinkCapacities += ceil(equalCap[i][j]);
      sumEqualLinkCapacities += equalCap[i][j];
    }
  }

  // output
  cout << "EE 697 Network Design" << endl;
  cout << "Topology: topology.txt" << endl;
  cout << "Traffic matrix: traffic.txt" << endl;
  cout << "Shortest path routing: ECMP-SP" << endl;
  cout << "Sum of link capacities = " << sumEqualLinkCapacities << endl;
  //cout << "Sum of link (ceil) capacities = " << ceiledSumEqualLinkCapacities << endl;
  cout << "Link capacities:" << endl;
  for (int i = 0; i < nodes - 1; i++)
  {
    for (int j = i + 1; j < nodes; j++)
      if (equalCap[i][j] > 0)
        cout << "(" << i << ',' << j << "): " << equalCap[i][j] << endl;
  }

  return 0;
}

void increaseCapacity(int k,
                      int j,
                      vector<vector<vi>>& preferredNeigbours,
                      vector<vector<double>>& equalCap,
                      double trafficKJ)
{
  if (k != j)
  {
    double load = trafficKJ / (preferredNeigbours[k][j].size() * 1.0);
    for (int t = 0; t < preferredNeigbours[k][j].size(); t++)
    {
      int neighbour = preferredNeigbours[k][j][t];
      equalCap[k][neighbour] += load;
      equalCap[neighbour][k] += load;
      // recurse till you reach destination j
      double newTraffic = load;
      increaseCapacity(neighbour, j, preferredNeigbours, equalCap, newTraffic);
    }
  }
}

vi& getIthNeighbours(const vector<vi>& neighbours, int i)
{
  vi* nodeNeigbours = new vi();
  for (int k = 0; k < neighbours.size(); k++)
  {
    if (neighbours[i][k] == 1)
    {
      nodeNeigbours->push_back(k);
    }
  }
  return *nodeNeigbours;
}