#include "algorithm.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>

namespace
{
struct DirectionInfo
{
    Direction direction;
    int viewX;
    int viewY;
    int dx;
    int dy;
};

const std::vector<DirectionInfo> kDirections = {
    {Direction::RIGHT, 1, 2, 0, 1},
    {Direction::DOWN, 2, 1, 1, 0},
    {Direction::LEFT, 1, 0, 0, -1},
    {Direction::UP, 0, 1, -1, 0}};

struct QueueNode
{
    Position position;
    int g = 0;
    int f = 0;
};

struct QueueCompare
{
    bool operator()(const QueueNode& left, const QueueNode& right) const
    {
        return left.f > right.f;
    }
};

Action makeMove(Direction direction)
{
    Action action;
    action.type = ActionType::MOVE;
    action.direction = direction;
    return action;
}

Position nextPosition(Position position, const DirectionInfo& direction)
{
    return {position.x + direction.dx, position.y + direction.dy};
}

bool isWalkable(char cell)
{
    return cell != '#';
}

int heuristic(Position left, Position right)
{
    return std::abs(left.x - right.x) + std::abs(left.y - right.y);
}

bool isKnownWalkable(Position position, const std::map<Position, CellMemory>& memory)
{
    auto cell = memory.find(position);
    return cell != memory.end() && cell->second.discovered && isWalkable(cell->second.type);
}

bool isFrontier(Position position, const std::map<Position, CellMemory>& memory)
{
    if (!isKnownWalkable(position, memory))
    {
        return false;
    }

    for (const DirectionInfo& direction : kDirections)
    {
        Position neighbor = nextPosition(position, direction);
        auto cell = memory.find(neighbor);
        if (cell == memory.end() || !cell->second.discovered)
        {
            return true;
        }
    }
    return false;
}

Action astarFirstStep(Position start, Position target, const std::map<Position, CellMemory>& memory)
{
    if (start == target)
    {
        return Action{};
    }

    std::priority_queue<QueueNode, std::vector<QueueNode>, QueueCompare> open;
    std::map<Position, int> bestG;
    std::map<Position, Direction> firstDirection;

    bestG[start] = 0;
    open.push({start, 0, heuristic(start, target)});

    while (!open.empty())
    {
        QueueNode current = open.top();
        open.pop();

        auto knownBest = bestG.find(current.position);
        if (knownBest == bestG.end() || current.g != knownBest->second)
        {
            continue;
        }

        if (current.position == target)
        {
            auto first = firstDirection.find(current.position);
            return first == firstDirection.end() ? Action{} : makeMove(first->second);
        }

        for (const DirectionInfo& direction : kDirections)
        {
            Position next = nextPosition(current.position, direction);
            if (!isKnownWalkable(next, memory))
            {
                continue;
            }

            int newG = current.g + 1;
            auto previous = bestG.find(next);
            if (previous != bestG.end() && previous->second <= newG)
            {
                continue;
            }

            bestG[next] = newG;
            firstDirection[next] = (current.position == start)
                ? direction.direction
                : firstDirection[current.position];
            open.push({next, newG, newG + heuristic(next, target)});
        }
    }

    return Action{};
}

Action moveToVisibleTarget(const GameState& state)
{
    for (char target : {'G', 'B', 'E'})
    {
        for (const DirectionInfo& direction : kDirections)
        {
            if (state.vision[direction.viewX][direction.viewY] == target)
            {
                return makeMove(direction.direction);
            }
        }
    }
    return Action{};
}

Action moveToKnownCellType(Position start,
                           char targetType,
                           const std::map<Position, CellMemory>& memory)
{
    std::vector<Position> targets;
    for (const auto& item : memory)
    {
        if (item.second.discovered && item.second.type == targetType)
        {
            targets.push_back(item.first);
        }
    }

    std::sort(targets.begin(), targets.end(), [start](Position left, Position right) {
        return heuristic(start, left) < heuristic(start, right);
    });

    for (Position target : targets)
    {
        Action action = astarFirstStep(start, target, memory);
        if (action.type != ActionType::NONE)
        {
            return action;
        }
    }
    return Action{};
}

Action moveToFrontier(Position start, const std::map<Position, CellMemory>& memory)
{
    std::vector<Position> frontiers;
    for (const auto& item : memory)
    {
        if (isFrontier(item.first, memory))
        {
            frontiers.push_back(item.first);
        }
    }

    std::sort(frontiers.begin(), frontiers.end(), [start](Position left, Position right) {
        return heuristic(start, left) < heuristic(start, right);
    });

    for (Position frontier : frontiers)
    {
        Action action = astarFirstStep(start, frontier, memory);
        if (action.type != ActionType::NONE)
        {
            return action;
        }
    }
    return Action{};
}

Action moveToLeastVisitedNeighbor(const GameState& state,
                                  const std::map<Position, CellMemory>& memory)
{
    bool found = false;
    int bestVisits = std::numeric_limits<int>::max();
    Direction bestDirection = Direction::UP;

    for (const DirectionInfo& direction : kDirections)
    {
        char cell = state.vision[direction.viewX][direction.viewY];
        if (!isWalkable(cell))
        {
            continue;
        }

        Position next = nextPosition(state.player.position, direction);
        auto remembered = memory.find(next);
        int visits = remembered == memory.end() ? 0 : remembered->second.visitCount;
        if (!found || visits < bestVisits)
        {
            found = true;
            bestVisits = visits;
            bestDirection = direction.direction;
        }
    }

    return found ? makeMove(bestDirection) : Action{};
}
}

Action Algorithm::AStar(const GameState& state, std::map<Position, CellMemory>& memory)
{
    Position start = state.player.position;

    Action action = moveToVisibleTarget(state);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    action = moveToKnownCellType(start, 'B', memory);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    action = moveToKnownCellType(start, 'E', memory);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    action = moveToFrontier(start, memory);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    return moveToLeastVisitedNeighbor(state, memory);
}
