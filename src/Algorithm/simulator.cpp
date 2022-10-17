#include "simulator.h"

Simulator::Simulator(ADG adg) {
  adg = adg;
  int init_states[get_agentCnt(adg)] = {0};
  state = init_states
}

Simulator::Simulator(ADG adg, int *visited_states) {
  adg = adg;
  int agentCnt = get_agentCnt(adg);
  int new_states[agentCnt];
  for (int agent = 0; agent < agentCnt; agent ++) {
    new_states[agent] = visited_states[agent];
  }
  states = new_states;
}

bool Simulator::move(int *moved, int agent, int *timeSpent) {
  if (moved[agent] == 1) return false;
  moved[agent] = 1;
  int state = states[agent];
  if (state >= get_stateCnt(adg, agent) - 1) return false;

  timeSpent[0] += 1;
  int next_state = state + 1;
  // Sanity check: no more switchable in neighbors 
  vector<pair<int, int>> switchables = get_switchable_inNeibPair(adg, agent, next_state);
  assert(switchables.size() == 0);

  vector<pair<int, int>> dependencies = get_nonSwitchable_inNeibPair(adg, agent, next_state);
  for (pair<int, int> dependency: dependencies) {
    int dep_agent = get<0>(dependency);
    int dep_state = get<1>(dependency);
    
    if (dep_agent != agent) {
      if (dep_state > states[dep_agent]) {
        return false;
      } else if (dep_state == state[dep_agent]) {
        if (!move(moved, dep_agent, timeSpent)) {
          return false;
        }
      }
    }
  }
  // All nonSwitchable dependencies resolved
  states[agent] += 1;
  return true;
}

int Simulator::step() {
  int timeSpent[1] = {0};
  int agentCnt = get_agentCnt(adg);
  int moved[agentCnt] = {0};
  for (int agent = 0; agent < agentCnt; agent++) {
    move(moved, agent, timeSpent);
  }
  return timeSpent[0];
}

pair<pair<int, int>, pair<int, int>> Simulator::detectSwitch() {
  int agentCnt = get_agentCnt(adg);
  int moved[agentCnt] = {0};
  for (int agent = 0; agent < agentCnt; agent++) {
    int state = states[agent];
    int next_state = state + 1;
    vector<pair<int, int>> dependencies = get_switchable_inNeibPair(adg, agent, next_state);

    for (pair<int, int> dependency: dependencies) {
      int dep_agent = get<0>(dependency);
      int dep_state = get<1>(dependency);
      
      assert(dep_agent != agent);
      if (dep_state > states[dep_agent]) {
        return make_pair(make_pair(dep_agent, dep_state), make_pair(agent, next_state));
      } else {
        fix_type2_edge(adg, dep_agent, dep_state, agent, next_state);
      }
    }
  }
  // No switchable edge detected
  return make_pair(make_pair(-1, -1), make_pair(-1, -1));
}

