#ifndef ASTAR
#define ASTAR

#include <queue>
#include <chrono>
using namespace std::chrono;

#include "ADG/ADG_utilities.h"
#include "simulator.h"
#include "nlohmann/json.hpp"
#include "group/group.h"
#include <memory>
#include <vector>
#include "Algorithm/heuristic.h"

enum BranchOrder {
  DEFAULT,
  CONFLICT,
  LARGEST_DIFF,
  RANDOM,
  EARLIEST
};

class Astar {
  public:
    Astar();
    Astar(int input_timeout);
    Astar(
      int input_timeout, 
      bool input_fast_version,
      const string & branch_order="default", 
      bool use_grouping=false, 
      const string & _heuristic="zero", 
      const bool early_termination=false,
      float _weight_h=1.0,
      uint random_seed=0
    );
    ADG startExplore(ADG &adg, float cost, int input_sw_cnt, vector<int> & states);
    float heuristic_graph(ADG &adg, shared_ptr<vector<int> > ts, shared_ptr<vector<int> > values);
    int slow_heuristic(ADG &adg, vector<int> &states);

    int compute_partial_cost(ADG &adg);

    void print_stats();
    void print_stats(ofstream &outFile);
    void print_stats(nlohmann::json & stats);

    struct Compare {
      public:
        bool operator() (const shared_ptr<Node> & s1, const shared_ptr<Node> & s2)
        {
          int val1 = get<1>(*s1);
          int val2 = get<1>(*s2);

          return val1 > val2;
        }
    };

  private:
    int calcTime(Simulator simulator);
    ADG exploreNode();
    tuple<int, int, int> enhanced_branch(Graph &graph, shared_ptr<vector<int> > values);
    tuple<int, int, int> branch(Graph &graph, shared_ptr<vector<int> > values);
    bool terminated(Graph &graph, shared_ptr<vector<int> > values);

    microseconds extraHeuristicT = std::chrono::microseconds::zero();
    microseconds groupingT = std::chrono::microseconds::zero();
    microseconds heuristicT = std::chrono::microseconds::zero();
    microseconds branchT = std::chrono::microseconds::zero();
    microseconds sortT = std::chrono::microseconds::zero();
    microseconds pqT = std::chrono::microseconds::zero();
    microseconds copy_free_graphsT = std::chrono::microseconds::zero();
    microseconds dfsT = std::chrono::microseconds::zero();
    microseconds termT = std::chrono::microseconds::zero();

    microseconds totalT  = std::chrono::seconds::zero();

    int explored_node_cnt = 0;
    int pruned_node_cnt = 0;
    int added_node_cnt = 0;

    int vertex_cnt = 0;
    int sw_edge_cnt = 0;
    
    int timeout = 300;

    vector<int> currents;
    priority_queue<shared_ptr<Node>, vector<shared_ptr<Node> >, Compare> pq;
    int agentCnt = 0;

    bool fast_version = false;
    BranchOrder branch_order=BranchOrder::DEFAULT;  
    std::mt19937 rng;

    bool use_grouping=false;
    std::shared_ptr<GroupManager> group_manager;
    std::shared_ptr<HeuristicManager> heuristic_manager;

    bool early_termination = false;

    float weight_h = 1.0;

    ADG init_adg;
    float init_cost;
};
#endif