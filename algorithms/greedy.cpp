#include "algorithm.h"

#include <algorithm>
#include <limits>
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

struct PathResult
{
    std::vector<Position> path;
    int profit = 0;
    int steps = 0;
    double ratio = 0.0;
};

struct Candidate
{
    Position target;
    PathResult path;
};

const std::vector<DirectionInfo> kDirections = {
    {Direction::RIGHT, 1, 2, 0, 1},
    {Direction::UP, 0, 1, -1, 0},
    {Direction::DOWN, 2, 1, 1, 0},
    {Direction::LEFT, 1, 0, 0, -1}};

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

int cellValue(char cell)
{
    if (cell == 'G')
    {
        return 50;
    }
    if (cell == 'T')
    {
        return -30;
    }
    return 0;
}

bool isKnownWalkable(Position position, const std::map<Position, CellMemory>& memory)
{
    auto cell = memory.find(position);
    return cell != memory.end() && cell->second.discovered && isWalkable(cell->second.type);
}

int manhattan(Position left, Position right)
{
    return std::abs(left.x - right.x) + std::abs(left.y - right.y);
}

void dfsBestRatioPath(Position current,
                      Position target,
                      const std::map<Position, CellMemory>& memory,
                      std::map<Position, bool>& visited,
                      std::vector<Position>& currentPath,
                      int currentProfit,
                      PathResult& best)
{
    if (current == target)
    {
        int steps = static_cast<int>(currentPath.size()) - 1;
        if (steps <= 0)
        {
            return;
        }

        double ratio = static_cast<double>(currentProfit) / steps;
        if (best.path.empty() ||
            ratio > best.ratio ||
            (ratio == best.ratio && steps < best.steps))
        {
            best.path = currentPath;
            best.profit = currentProfit;
            best.steps = steps;
            best.ratio = ratio;
        }
        return;
    }

    for (const DirectionInfo& direction : kDirections)
    {
        Position next = nextPosition(current, direction);
        if (!isKnownWalkable(next, memory) || visited[next])
        {
            continue;
        }

        visited[next] = true;
        currentPath.push_back(next);
        auto cell = memory.find(next);
        int profit = currentProfit + (cell == memory.end() ? 0 : cellValue(cell->second.type));

        dfsBestRatioPath(next, target, memory, visited, currentPath, profit, best);

        currentPath.pop_back();
        visited[next] = false;
    }
}

PathResult bestRatioPath(Position start,
                         Position target,
                         const std::map<Position, CellMemory>& memory)
{
    PathResult best;
    std::map<Position, bool> visited;
    std::vector<Position> currentPath;

    visited[start] = true;
    currentPath.push_back(start);
    dfsBestRatioPath(start, target, memory, visited, currentPath, 0, best);
    return best;
}

Action firstStepFromPath(const std::vector<Position>& path)
{
    if (path.size() < 2)
    {
        return Action{};
    }

    Position start = path[0];
    Position next = path[1];
    if (next.x < start.x)
    {
        return makeMove(Direction::UP);
    }
    if (next.x > start.x)
    {
        return makeMove(Direction::DOWN);
    }
    if (next.y < start.y)
    {
        return makeMove(Direction::LEFT);
    }
    return makeMove(Direction::RIGHT);
}

Action moveToBestGold(Position start, const std::map<Position, CellMemory>& memory)
{
    std::vector<Candidate> candidates;
    for (const auto& item : memory)
    {
        if (!item.second.discovered || item.second.type != 'G')
        {
            continue;
        }

        PathResult path = bestRatioPath(start, item.first, memory);
        if (!path.path.empty() && path.profit > 0)
        {
            candidates.push_back({item.first, path});
        }
    }

    if (candidates.empty())
    {
        return Action{};
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& left, const Candidate& right) {
        if (left.path.ratio != right.path.ratio)
        {
            return left.path.ratio > right.path.ratio;
        }
        if (left.path.steps != right.path.steps)
        {
            return left.path.steps < right.path.steps;
        }
        if (left.target.x != right.target.x)
        {
            return left.target.x < right.target.x;
        }
        return left.target.y < right.target.y;
    });

    return firstStepFromPath(candidates.front().path.path);
}

Action moveToKnownType(Position start, char targetType, const std::map<Position, CellMemory>& memory)
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
        return manhattan(start, left) < manhattan(start, right);
    });

    for (Position target : targets)
    {
        PathResult path = bestRatioPath(start, target, memory);
        Action action = firstStepFromPath(path.path);
        if (action.type != ActionType::NONE)
        {
            return action;
        }
    }
    return Action{};
}

bool isFrontier(Position position, const std::map<Position, CellMemory>& memory)
{
    if (!isKnownWalkable(position, memory))
    {
        return false;
    }

    for (const DirectionInfo& direction : kDirections)
    {
        Position next = nextPosition(position, direction);
        auto cell = memory.find(next);
        if (cell == memory.end() || !cell->second.discovered)
        {
            return true;
        }
    }
    return false;
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
        return manhattan(start, left) < manhattan(start, right);
    });

    for (Position frontier : frontiers)
    {
        PathResult path = bestRatioPath(start, frontier, memory);
        Action action = firstStepFromPath(path.path);
        if (action.type != ActionType::NONE)
        {
            return action;
        }
    }
    return Action{};
}

Action moveToLeastVisitedNeighbor(const GameState& state, const std::map<Position, CellMemory>& memory)
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

std::map<Position, CellMemory> memoryFromKnownMap(const GameState& state,
                                                  const std::map<Position, CellMemory>& memory)
{
    std::map<Position, CellMemory> fullMemory;
    for (int row = 0; row < static_cast<int>(state.knownMap.size()); ++row)
    {
        for (int column = 0; column < static_cast<int>(state.knownMap[row].size()); ++column)
        {
            Position position{row, column};
            CellMemory cell;
            cell.discovered = true;
            cell.type = state.knownMap[row][column];
            auto old = memory.find(position);
            if (old != memory.end())
            {
                cell.visitCount = old->second.visitCount;
            }
            fullMemory[position] = cell;
        }
    }
    return fullMemory;
}
}

Action Algorithm::Greedy(const GameState& state, std::map<Position, CellMemory>& memory)
{
    Position start = state.player.position;
    std::map<Position, CellMemory> fullMemory;
    const std::map<Position, CellMemory>* decisionMemory = &memory;
    if (state.finishOnNoAction && !state.knownMap.empty())
    {
        fullMemory = memoryFromKnownMap(state, memory);
        decisionMemory = &fullMemory;
    }

    Action action = moveToBestGold(start, *decisionMemory);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    if (state.finishOnNoAction)
    {
        return Action{};
    }

    action = moveToKnownType(start, 'B', *decisionMemory);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    action = moveToKnownType(start, 'E', *decisionMemory);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    action = moveToFrontier(start, *decisionMemory);
    if (action.type != ActionType::NONE)
    {
        return action;
    }

    return moveToLeastVisitedNeighbor(state, *decisionMemory);
}
