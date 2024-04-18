#pragma once
#include "types.h"

// NOTE(rivers): we almost copy codes from https://github.com/Jiaoyang-Li/CBSH2-RTC/blob/main/src/CBSHeuristic.cpp
enum HeuristicType {
    ZERO,
    CG_GREEDY,
    WCG_GREEDY,
    FAST_WCG_GREEDY
};

class HeuristicManager {
public:
    HeuristicType type=HeuristicType::ZERO;

    HeuristicManager(HeuristicType type);

    double computeInformedHeuristics(
        const shared_ptr<Graph> & graph, 
        const shared_ptr<vector<int> > & longest_paths, 
        const shared_ptr<vector<shared_ptr<map<int,int> > > > & reverse_longest_paths,
        double time_limit,
        bool fast_approximate
    );
    void buildCardinalConflictGraph(
        const shared_ptr<Graph> & graph, 
        const shared_ptr<vector<int> > & longest_paths, 
        const shared_ptr<vector<shared_ptr<map<int,int> > > > & old_reverse_longest_path_lengths_ptr, 
        vector<int> & CG, 
        bool weighted,
        bool fast_approximate
    );
    double greedyMatching(const std::vector<int> & CG, int num_vertices);

};