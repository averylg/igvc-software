/**
D* lite path planner implementation. D* lite is an incremental search
algorithm that keeps track of previous search results to recaculate a new path
when cell traversal costs have changed.

the DLitePlanner interfaces with the Graph object to calculate the optimal
path through the occupancy grid in an eight-connected grid space. This means
that each node has 8 neighbors and can only travel to those eight neighbors.

Author: Alejandro Escontrela <aescontrela3@gatech.edu>
Date Created: December 22nd, 2018
*/

#ifndef FIELDDPLANNER_H
#define FIELDDPLANNER_H

#include "Graph.h"
#include "Node.h"
#include "PriorityQueue.h"

#include <unordered_map>
#include <utility>
#include <vector>
#include <limits>
#include <cmath>

#include <iostream>

class DLitePlanner
{
public:
    // Graph contains methods to deal with Node(s) as well as updated occupancy
    // grid cells
    Graph graph;

    std::vector<std::tuple<int,int>> path;

    float GOAL_DIST = 0.95f;

    DLitePlanner();
    ~DLitePlanner();

    /**
    Calculate the key for a node S.

    key defined as <f1(s), f2(s)>
    where...
    f1(s) = min(g(s), rhs(s)) + h(s_start, s) + K_M
    f2(s)min(g(s), rhs(s))

    @param[in] s Node to calculate key for
    @return calculated key
    */
    Key calculateKey(Node s);
    /**
    Initializes the graph search problem by setting g and rhs values for start
    node equal to infinity. For goal node, sets g value to infinity and rhs value
    to 0. Inserts goal node into priority queue to initialize graph search problem.
    */
    void initialize();
    /**
    Updates a node's standing in the graph search problem. Update dependant upon
    the node's g value and rhs value relative to each other.

    Locally inconsistent nodes (g != rhs) are inserted into the priority queue
    while locally consistent nodes are not.

    @param[in] s Node to update
    */
    void updateNode(Node s);
    /**
    Expands nodes in the priority queue until optimal path to goal node has been
    found. The first search is equivalent to an A* heuristic search. All calls
    to computeShortestPath() thereafter only expand the nodes necessary to
    compute the optimal path to the goal node.

    @return number of nodes expanded in graph search
    */
    int computeShortestPath();

    /**
    Returns g-value for a node s
    */
    float getG(Node s);
    /**
    Returns rhs value for a node s
    */
    float getRHS(Node s);

    /**
    Tries to insert an entry into the unordered map. If an entry with that key
    already exists, overrides the value with the new g and rhs values.

    @param[in] s key to create entry for
    @param[in] g g value for entry
    @param[in] rhs rhs value for entry
    */
    void insert_or_assign(Node s, float g, float rhs);

    /**
    Updates nodes around cells whose occupancy values have changed while taking
    into account c-space. This step is performed after the robot moves and the
    occupancy grid is updated with new sensor information

    @return the number of nodes updated
    */
    int updateNodesAroundUpdatedCells();

    /**
    Constructs the optimal path through the search space by greedily choosing
    the next node that minimizes c(s,s') + g(s').
    */
    void constructOptimalPath();

    /**
    Checks whether a specified node is within range of the goal node. This 'range'
    is specified by the GOAL_RANGE instance variable.

    @param[in] s Node to check
    @return whether or not node s is within range of the goal
    */
    bool isWithinRangeOfGoal(Node s);



private:
    // hashed map contains all nodes and <g,rhs> values in search
    std::unordered_map<Node,std::tuple<float,float>> umap;
    // priority queue contains all locally inconsistent nodes whose values
    // need updating
    PriorityQueue PQ;
};

#endif
