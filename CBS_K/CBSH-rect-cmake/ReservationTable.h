#pragma once

#include <boost/unordered_map.hpp>
#include <unordered_set> 
#include "common.h"
#include "constrained_agents_loader.h"
#include "Conflict.h"
#include <memory>
#include "Path.h"
using namespace std;



class AgentStep {
public:
	AgentStep() {};
	AgentStep(int id, int l, int t0,bool head0, int position0) {
		agent_id = id;
		loc = l;
		t = t0;
		head = head0;
		position = position0;
	}
	int agent_id;
	int loc;
	int t;
	bool head;
	int position;
	AgentStep* preStep = NULL;
	AgentStep* nextStep = NULL;

	bool operator == (AgentStep const& s2)
	{
		return agent_id == s2.agent_id;
	}
};


class ReservationTable {
public:
	struct eqint
	{
		bool operator()(int s1, int s2) const
		{
			return s1 == s2;
		}
	};

	typedef boost::unordered_map<int, AgentStep> agentList;//stores agents in a loc at a certain timestep
	typedef boost::unordered_map<int, agentList> timeline;//key is time step, value is agentlist
	typedef boost::unordered_map<int, timeline> map_table;//hash table, key is map location, value is time line
	typedef boost::unordered_map<int, int> goalAgentList;
	boost::unordered_map<int, goalAgentList> goalTable;
	map_table res_table;
	ConstrainedAgentsLoader* agentsLoader;
	void addPath(int agent_id, std::vector<PathEntry>* path);
	void addPaths(vector<vector<PathEntry>*>* paths, int exclude = -1);
	void deletePath(int agent_id, std::vector<PathEntry>* path);
	std::list<Conflict> findConflict(int agent, int currLoc, list<int> next_locs, int currT, bool parking = false);
	int countConflict(int agent, int currLoc, list<int> next_locs, int currT);



	ReservationTable(int mapSize, ConstrainedAgentsLoader* agentsLoader);
	ReservationTable(int mapSize, vector<vector<PathEntry>*>* paths, ConstrainedAgentsLoader* agentsLoader, int exclude=-1);

};