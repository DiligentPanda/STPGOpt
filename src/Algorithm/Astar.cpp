#include "Astar.h"

int heuristic(Simulator simulator) {
  int stepSpend = 0;
  int totalSpend = 0;

  stepSpend = simulator.step(false);
  while (stepSpend != 0) {
    if (stepSpend < 0) return -1; // stuck
    totalSpend += stepSpend;
    stepSpend = simulator.step(false);
  }
  return totalSpend;
}

class Compare {
  public:
    bool operator() (Node s1, Node s2)
    {
      int g1 = get<1>(s1);
      int h1 = get<2>(s1);
      int g2 = get<1>(s2);
      int h2 = get<2>(s2);

      return (g1 + h1) < (g2 + h2);
    }
};

Simulator exploreNode(priority_queue<Node, vector<Node>, Compare> pq,
                      Simulator simulator, int g) {
  int agent1, state1, agent2, state2;
  tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  while (agent1 < 0) {
    int g_step = simulator.step(true);
    if (g_step == 0) return simulator; // All agents reach their goals
    g += g_step;

    tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  }
  
  // Detected a switchable edge
  ADG copy = copy_ADG(simulator.adg);
  // Forward child
  fix_type2_edge(simulator.adg, agent1, state1, agent2, state2);
  if (detectCycle(simulator.adg, agent1, state1)) // Prune node
  {
    free_underlying_graph(simulator.adg);
  } 
  else
  {
    Simulator simulator_h(simulator.adg, simulator.states);
    int h = heuristic(simulator_h);
    if (h < 0) // Prune node
    {
      free_underlying_graph(simulator.adg);
    } else {
      pq.push(make_tuple(simulator, g, h));
    }
  }

  // Backward child
  fix_type2_edge_reversed(copy, agent1, state1, agent2, state2);
  if (detectCycle(copy, agent2, state2)) // Prune node
  {
    free_underlying_graph(copy);
  }
  else // Add to the priority queue
  {
    Simulator simulator_h(copy, simulator.states);
    int h = heuristic(simulator_h);
    if (h < 0) // Prune node
    {
      free_underlying_graph(copy);
    } else {
      Simulator simulator_back(copy, simulator.states);
      pq.push(make_tuple(simulator_back, g, h)); 
    }
  }

  // Recursive call
  Node node = pq.top();
  pq.pop();
  Simulator new_simulator = get<0>(node);
  int new_g = get<1>(node);
  return exploreNode(pq, new_simulator, new_g);
}

ADG Astar(ADG root) {
  Simulator simulator(root);
  priority_queue<Node, vector<Node>, Compare> pq;
  int g = 0;
  simulator = exploreNode(pq, simulator, g);
  return simulator.adg;
}

int main(int argc, char** argv) {
  char* fileName = argv[1];
  ADG adg = construct_ADG(fileName);
  ADG res = Astar(adg);
  // for (pair<int, int> as: get_switchable_inNeibPair(adg, 13, 1)) {
  //   std::cout << get<0>(as) << ", " << get<1>(as) << ";  ";
  // }
  // Simulator simulator(adg);

  // int agent1, state1, agent2, state2;
  // int step = 0;
  // int g = 0;
  // tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  // while (agent1 < 0) {
  //   int g_step = simulator.step(true);
  //   if (g_step == 0) {
  //     std::cout << "reach goal\n";
  //     return 0;
  //   }
  //   step ++;
  //   g += g_step;

  //   tie(agent1, state1, agent2, state2) = simulator.detectSwitch();
  // }
  // if (agent1 >= 0) // Detected a switchable edge
  // {
  //   std::cout << "detected switchable: " << agent1 << ", " << state1 << ";  " << agent2 << ", " << state2 << "\n";
  //   std::cout << "step = " << step << ", g = " << g << "\n";
  // }

  return 0;
}


// Dead code below for longest path heuristic algorithms. Might restore later
// void topologicalSort(ADG adg, int v, bool* visited, vector<int> result)
// {
//   if (v >= 0) visited[v] = true;
//   vector<int> neighbors;
//   if (v >= 0) {
//     Graph graph = get<0>(adg);
//     neighbors = get_outNeighbors(graph, v);
//   } else {
//     for (int i = 0; i < get_agentCnt(adg); i++) {
//       neighbors.push_back(compute_vertex_ADG(adg, i, 0));
//     }
//   }

//   for (int neighbor: neighbors) {
//     if (!visited[neighbor]) {
//       topologicalSort(adg, neighbor, visited, result);
//     }
//   }
//   result.push_back(v);
// }

// int heuristic(ADG adg) {
//   int n = get_totalStateCnt(adg);
//   vector<int> result;
//   bool* visited = new bool[n]();
//   int v = -1;
//   topologicalSort(adg, v, visited, result);
// }