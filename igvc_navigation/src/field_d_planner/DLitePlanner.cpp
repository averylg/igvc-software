#include "DLitePlanner.h"

//TODO remove std::cout and iostream from import list

DLitePlanner::DLitePlanner() {}

DLitePlanner::~DLitePlanner() {}

Key DLitePlanner::calculateKey(Node s)
{
    // obtain g-values and rhs-values for node s
    float g = getG(s);
    float rhs = getRHS(s);
    return Key(std::min(g, rhs) + graph.euclidian_heuristic(s) + graph.K_M, std::min(g, rhs));
}

void DLitePlanner::initialize()
{
    umap.insert(std::make_pair(graph.Start, std::make_tuple(
        std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::infinity())));
    umap.insert(std::make_pair(graph.Goal, std::make_tuple(
        std::numeric_limits<float>::infinity(),
        0)));

    PQ.insert(graph.Goal, calculateKey(graph.Goal));
}

void DLitePlanner::updateNode(Node s)
{
    if (umap.find(s) == umap.end()) // s never visited before, add to unordered map
        umap.insert(std::make_pair(s, std::make_tuple(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity())));

    /**
    looks for a node in the priority queue and removes it if found
    same as calling:

    if PQ.contains(s)
        PQ.remove(s)
    */
    PQ.remove(s);

    // update rhs value of Node s
    if (s != graph.Goal)
    {
        float minRHS = std::numeric_limits<float>::infinity();
        for (Node succ : graph.nbrs(s))
        {
            if (umap.find(succ) == umap.end()) // node never visited before, add to unordered map
                umap.insert(std::make_pair(succ, std::make_tuple(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity())));

            minRHS = std::min(minRHS, graph.getTraversalCost(s, succ) + getG(succ));
        }

        insert_or_assign(s, getG(s), minRHS);
    }


    // insert node into priority queue if it is locally inconsistent
    if (getG(s) != getRHS(s))
        PQ.insert(s, calculateKey(s));
}

int DLitePlanner::computeShortestPath()
{
    int numNodesExpanded = 0;

    // std::cout << "Start Expanding" << std::endl;
    // std::cout << "Start Node: " << graph.Start << std::endl;
    // std::cout << "Goal Node: " << graph.Goal << std::endl;

    while((PQ.topKey() < calculateKey(graph.Start)) || (getRHS(graph.Start) != getG(graph.Start)))
    {
        numNodesExpanded++;

        // std::cout << "PQ: " << PQ << std::endl;
        // std::cout << "Start Key: " << calculateKey(graph.Start) << std::endl;


        Node topNode = PQ.topNode();
        Key topKey = PQ.topKey();
        PQ.pop();

        if (topKey < calculateKey(topNode))
        {
            PQ.insert(topNode, calculateKey(topNode));
        }
        else if (getG(topNode) > getRHS(topNode))
        {
            // locally overconsistent case. This node is now favorable.

            // make node locally consistent by setting g = rhs
            insert_or_assign(topNode, getRHS(topNode), getRHS(topNode)); // update node's standing in unordered map

            for (Node nbr : graph.nbrs(topNode)) updateNode(nbr);
        }
        else
        {
            // locally underconsistent case. This node is now less favorable
            // than it was before

            // make node locally consistent or overconsistent by setting g = inf
            insert_or_assign(topNode, std::numeric_limits<float>::infinity(), getRHS(topNode));

            // propagate changes to neighbors and to topNode
            std::vector<Node> toPropagate = graph.nbrs(topNode);
            toPropagate.push_back(topNode);

            for (Node n : toPropagate) updateNode(n);
        }
    }
    return numNodesExpanded;
}

float DLitePlanner::getG(Node s)
{
    // return g value if node has been looked at before (is in unordered map)
    // otherwise, return infinity
    if (umap.find(s) != umap.end())
        return std::get<0>(umap.at(s));
    else
        return std::numeric_limits<float>::infinity();
}

float DLitePlanner::getRHS(Node s)
{
    // return rhs value if node has been looked at before (is in unordered map)
    // otherwise, return infinity
    if (umap.find(s) != umap.end())
        return std::get<1>(umap.at(s));
    else
        return std::numeric_limits<float>::infinity();
}

void DLitePlanner::insert_or_assign(Node s, float g, float rhs)
{
    // re-assigns value of node in unordered map or inserts new entry if
    // node not found

    if (umap.find(s) != umap.end())
        umap.erase(s);

    umap.insert(std::make_pair(s, std::make_tuple(g, rhs)));
}

int DLitePlanner::updateNodesAroundUpdatedCells()
{
    int numNodesUpdated = 0;

    for (std::tuple<int,int> cellUpdate : graph.updatedCells)
    {
        std::vector<Node> affectedNodes = \
                graph.getNodesAroundCellWithCSpace(cellUpdate);

        for (Node n : affectedNodes)
        {
            if (umap.find(n) == umap.end()) // node hasn't been explored yet. Leave alone
                continue;

            updateNode(n);
            numNodesUpdated++;
        }
    }

    return numNodesUpdated;
}

void DLitePlanner::constructOptimalPath()
{
    path.clear();
    std::vector<std::tuple<int,int>>::iterator start_it;

    Node currNode = graph.Start;
    Node goalNode = graph.Goal;

    std::cout << "Constructing Optimal Path: " << "Start -> " << currNode << " Goal  -> " << goalNode << std::endl;

    do
    {
        start_it = path.begin();
        path.insert(start_it, currNode.getIndex());

        float minCost = std::numeric_limits<float>::infinity();
        float tempCost;
        Node tempNode;
        for (Node nbr : graph.nbrs(currNode))
        {
            if (umap.find(nbr) == umap.end()) // node has not been explored yet
                continue;

            tempCost = graph.getTraversalCost(currNode, nbr) + getG(nbr);
            if (tempCost < minCost)
            {
                minCost = tempCost;
                tempNode = nbr;
            }
        }
        std::cout << " --> Curr Node: " << currNode << " - Cost: " << minCost << " - Valid: " << isWithinRangeOfGoal(currNode) << " <--";

        currNode = tempNode;

    } while(!isWithinRangeOfGoal(currNode));
}

bool DLitePlanner::isWithinRangeOfGoal(Node s)
{
    int goal_x, goal_y;
    std::tie(goal_x, goal_y) = graph.Goal.getIndex();

    int x, y;
    std::tie(x,y) = s.getIndex();

    int sep = static_cast<int>(GOAL_DIST/graph.Resolution);

    bool satisfiesXBounds = (x >= (goal_x - sep)) && (x <= (goal_x + sep));
    bool satisfiesYBounds = (y >= (goal_y - sep)) && (y <= (goal_y + sep));

    return satisfiesXBounds && satisfiesYBounds;
}
