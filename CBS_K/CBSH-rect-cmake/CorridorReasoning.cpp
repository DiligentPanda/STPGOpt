#include "CorridorReasoning.h"
#include "constrained_map_loader.h"

bool validMove(int curr, int next, int map_cols, int map_size)
{
	if (next < 0 || next >= map_size)
		return false;
	return getMahattanDistance(curr, next, map_cols) < 2;
}

int getMahattanDistance(int loc1, int loc2, int map_cols)
{
	int loc1_x = loc1 / map_cols;
	int loc1_y = loc1 % map_cols;
	int loc2_x = loc2 / map_cols;
	int loc2_y = loc2 % map_cols;
	return std::abs(loc1_x - loc2_x) + std::abs(loc1_y - loc2_y);
}


int getDegree(int loc, const bool*map, int num_col, int map_size)
{
	if (loc < 0 || loc >= map_size || map[loc])
		return -1;
	int degree = 0;
	if (0 < loc - num_col && !map[loc - num_col])
		degree++;
	if (loc + num_col < map_size && !map[loc + num_col])
		degree++;
	if (loc % num_col > 0 && !map[loc - 1])
		degree++;
	if (loc % num_col < num_col - 1 && !map[loc + 1])
		degree++;
	return degree;
}


int getCorridorLength(const std::vector<PathEntry>& path, int t_start, int loc_end, std::pair<int, int>& edge)
{
	int curr = path[t_start].Loc.location;
	int next;
	int prev = -1;
	int length = 0; // distance to the start location
	int t = t_start;
	bool moveForward = true;
	bool updateEdge = false;
	while (curr != loc_end)
	{
		t++;
		next = path[t].Loc.location;
		if (next == curr) // wait
			continue;
		else if (next == prev) // turn aournd
			moveForward = !moveForward;
		if (moveForward)
		{
			if (!updateEdge)
			{
				edge = std::make_pair(curr, next);
				updateEdge = true;
			}
			length++;
		}
		else
			length--;
		prev = curr;
		curr = next;
	}
	return length;
}

template<class Map>
int CorridorReasoning<Map>::getEnteringTime(const std::vector<PathEntry>& path, const std::vector<PathEntry>& path2, int t,
	Map* map)
{
	if (t >= path.size())
		t = path.size() - 1;
	int loc = path[t].Loc.location;
	while (loc != path.front().Loc.location && loc != path2.back().Loc.location &&
		map->getDegree(loc) == 2)
	{
		t--;
		loc = path[t].Loc.location;
	}
	return t;
}

template<class Map>
int CorridorReasoning<Map>::getExitTime(const std::vector<PathEntry>& path, const std::vector<PathEntry>& path2, int t,
	Map* map)
{
	if (t >= path.size())
		t = path.size() - 1;
	int loc = path[t].Loc.location;
	while (loc != path.back().Loc.location && loc != path2.front().Loc.location &&
		map->getDegree(loc) == 2)
	{
		t++;
		loc = path[t].Loc.location;
	}
	return t;
}


template<class Map>
        int CorridorReasoning<Map>::getBypassLength(Location start, int end, std::pair<int, int> blocked, Map* my_map, int num_col, int map_size,
                                                    ConstraintTable& constraint_table, int upper_bound,SingleAgentICBS<Map>* solver, int start_heading, int end_heading, int k)
{
	solver->ml->set_agent_idx(solver->agent_id);

	int length = INT_MAX;
	// generate a heap that can save nodes (and a open_handle)
	boost::heap::fibonacci_heap< LLNode*, boost::heap::compare<LLNode::compare_node> > heap;
	boost::heap::fibonacci_heap< LLNode*, boost::heap::compare<LLNode::compare_node> >::handle_type open_handle;
	// generate hash_map (key is a node pointer, data is a node handler,
	//                    NodeHasher is the hash function to be used,
	//                    eqnode is used to break ties when hash values are equal)
    typedef boost::unordered_set<LLNode*, LLNode::NodeHasher, LLNode::eqnode> hashtable_t;
    hashtable_t nodes;
    hashtable_t::iterator it; // will be used for find()

    int start_h_val = abs(solver->my_heuristic_i[end].get_hval(end_heading) - solver->my_heuristic_i[start.location].get_hval(start_heading));
    LLNode* root = new LLNode(list<Location>(), 0, start_h_val, NULL, 0);
    root->locs.resize(k+1,start); //todo:: start also should be occupation.
	root->heading = start_heading;
	root->open_handle = heap.push(root);  // add root to heap
	nodes.insert(root);       // add root to hash_table (nodes)
	int moves_offset[5] = { 1, -1, num_col, -num_col, 0 };
	LLNode* curr = NULL;
	int time_generated = 0;
	while (!heap.empty())
	{
		curr = heap.top();
		heap.pop();
		curr->closed= true;
		if (curr->locs.front().location == end)// todo:: ignore heading for now. maybe adding back if run experiment on flatland
		{
			length = curr->g_val;
			break;
		}
		vector<pair<Location, int>> transitions;
		transitions = my_map->get_transitions(curr->locs.front(), curr->heading, false);

		for (const pair<Location, int> move : transitions)
		{
			Location next_loc = move.first;
			time_generated += 1;
			list<Location> next_locs;
			if (!getOccupations(next_locs, next_loc, curr,k) && constraint_table.has_train)
                continue;

			int next_timestep = curr->timestep + 1;
			if (constraint_table.latest_timestep <= curr->timestep)
			{
				if (move.second == 4)
				{
					continue;
				}
				next_timestep--;
			}

			//check does head have edge constraint or body have vertex constraint.
			bool constrained = false;

			//Check edge constraint on head
			if (curr->locs.front().location != -1 && constraint_table.is_constrained(curr->locs.front().location * map_size + next_loc.location, next_timestep))
			    constrained = true;

			//Check vertex constraint on body and head
			for(auto loc:next_locs){
			    if (loc.location == -1)
			        break;
			    if (constraint_table.is_constrained(loc.location, next_timestep, loc != next_locs.front()) )
			        constrained = true;
			    if(!constraint_table.has_train) //if not train, only check head
			        break;
			}

			if (constrained)
			    continue;


            if ((curr->locs.front().location == blocked.first && next_loc.location == blocked.second) ||
                (curr->locs.front().location == blocked.second && next_loc.location == blocked.first)) // use the prohibited edge
            {
                continue;
            }
            int next_heading;

            if (curr->heading == -1) //heading == 4 means no heading info
                next_heading = -1;
            else
                if (move.second == 4) //move == 4 means wait
                    next_heading = curr->heading;
                else
                    next_heading = move.second;

            int next_g_val = curr->g_val + 1;
            int next_h_val;
            if (next_loc.location == -1)
                next_h_val = abs(solver->my_heuristic_i[end].get_hval(end_heading) - solver->my_heuristic_i[start.location].get_hval(start_heading));
            else
                next_h_val = abs(solver->my_heuristic_i[end].get_hval(end_heading) - solver->my_heuristic_i[next_loc.location].get_hval(next_heading));
            if (next_g_val + next_h_val >= upper_bound) // the cost of the path is larger than the upper bound
                continue;
            LLNode* next = new LLNode(next_locs, next_g_val, next_h_val, NULL, next_timestep);
            next->heading = next_heading;
            next->actionToHere = move.second;
            next->time_generated = time_generated;
            assert(next->timestep <= constraint_table.latest_timestep);
            if (constraint_table.has_train)
                next->train_mode = true;

            it = nodes.find(next);
            if (it == nodes.end())
            {  // add the newly generated node to heap and hash table
                next->open_handle = heap.push(next);
                nodes.insert(next);
            }
            else {  // update existing node's g_val if needed (only in the heap)
                delete(next);  // not needed anymore -- we already generated it before
                LLNode* existing_next = (*it);
                if (existing_next->g_val > next_g_val)
                {

                    existing_next->g_val = next_g_val;
                    existing_next->timestep = next_timestep;
                    if (existing_next->closed){
                        existing_next->closed = false;
                        existing_next->open_handle = heap.push(existing_next);
                    }
                    else
                        heap.update(existing_next->open_handle);
                }
            }

		}
	}
	for (it = nodes.begin(); it != nodes.end(); it++)
	{
		delete (*it);
	}
	return length;
}

template<class Map>
bool CorridorReasoning<Map>::getOccupations(list<Location> & next_locs, Location next_id, LLNode* curr, int k){
    next_locs.push_back(next_id);
    auto parent = curr;
    Location & pre_loc = next_id;
    bool conf_free = true;
    while(next_locs.size()<=k){
        if (parent == nullptr){
            next_locs.push_back(next_locs.back());
        }
        else {
            if (pre_loc != parent->locs.front()) {
                next_locs.push_back(parent->locs.front());
                pre_loc = parent->locs.front();
                if (next_locs.front() == next_locs.back()) {
                    conf_free = false;
                }
            }
            parent = parent->parent;
        }

    }

    return conf_free;
}



bool isConstrained(int curr_id, int next_id, int next_timestep, const std::vector< std::list< std::pair<int, int> > >* cons)
{
	if (cons == NULL)
		return false;
	// check vertex constraints (being in next_id at next_timestep is disallowed)
	if (next_timestep < static_cast<int>(cons->size()))
	{
		for (std::list< std::pair<int, int> >::const_iterator it = cons->at(next_timestep).begin(); it != cons->at(next_timestep).end(); ++it)
		{
			if ((std::get<0>(*it) == next_id && std::get<1>(*it) < 0)//vertex constraint
				|| (std::get<0>(*it) == curr_id && std::get<1>(*it) == next_id)) // edge constraint
				return true;
		}
	}
	return false;
};
template class CorridorReasoning<ConstrainedMapLoader>;
