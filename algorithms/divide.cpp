#include "algorithm.h"

#include <algorithm>
#include <cstdlib>
#include <queue>
#include <set>

namespace
{
// DirectionInfo（候选方向结构）输入形式 无 输入含义 无 输出形式 DirectionInfo 输出含义 保存方向、视野坐标和坐标增量
struct DirectionInfo
{
    Direction direction;
    int viewX;
    int viewY;
    int dx;
    int dy;
};

// 四个移动方向：顺序会影响同分时的选择。
const vector<DirectionInfo> kDirections = {
    {Direction::RIGHT, 1, 2, 0, 1},
    {Direction::DOWN, 2, 1, 1, 0},
    {Direction::LEFT, 1, 0, 0, -1},
    {Direction::UP, 0, 1, -1, 0}
};

// makeMove（构造移动动作）输入形式 Direction direction 输入含义 移动方向 输出形式 Action 输出含义 MOVE类型动作
Action makeMove(Direction direction)
{
    Action action;
    action.type = ActionType::MOVE;
    action.direction = direction;
    return action;
}

// isWalkable（判断格子是否可通行）输入形式 char cell 输入含义 地图格子字符 输出形式 bool 输出含义 true表示不是墙
bool isWalkable(char cell)
{
    return cell != '#';
}

// nextPosition（计算相邻坐标）输入形式 Position position, const DirectionInfo& direction 输入含义 当前坐标、方向信息 输出形式 Position 输出含义 相邻目标坐标
Position nextPosition(Position position, const DirectionInfo& direction)
{
    return {position.x + direction.dx, position.y + direction.dy};
}

// isFrontier（判断是否为探索前沿）输入形式 const Position& position, const map<Position, CellMemory>& memory 输入含义 待判断坐标、AI已知地图记忆 输出形式 bool 输出含义 true表示该格可走且邻接未知区域
bool isFrontier(const Position& position, const map<Position, CellMemory>& memory)
{
    auto current = memory.find(position);
    if (current == memory.end() || !isWalkable(current->second.type))
    {
        return false;
    }

    for (const DirectionInfo& direction : kDirections)
    {
        Position neighbor = nextPosition(position, direction);
        if (memory.find(neighbor) == memory.end())
        {
            return true;
        }
    }
    return false;
}

// firstStepToTarget（在已知地图中寻找第一步）输入形式 Position start, Position target, const map<Position, CellMemory>& memory 输入含义 起点、目标、AI已知地图记忆 输出形式 Action 输出含义 通往目标的第一步MOVE动作，找不到则为NONE
Action firstStepToTarget(Position start, Position target, const map<Position, CellMemory>& memory)
{
    if (start == target)
    {
        return Action{};
    }

    queue<Position> q;
    set<Position> visited;
    map<Position, Direction> firstDirection;

    q.push(start);
    visited.insert(start);

    while (!q.empty())
    {
        Position current = q.front();
        q.pop();

        for (const DirectionInfo& direction : kDirections)
        {
            Position next = nextPosition(current, direction);
            if (visited.count(next) > 0)
            {
                continue;
            }

            auto cell = memory.find(next);
            if (cell == memory.end() || !isWalkable(cell->second.type))
            {
                continue;
            }

            visited.insert(next);
            firstDirection[next] = (current == start) ? direction.direction : firstDirection[current];
            if (next == target)
            {
                return makeMove(firstDirection[next]);
            }
            q.push(next);
        }
    }

    return Action{};
}

// chooseLeastVisitedNeighbor（选择最少访问邻格）输入形式 const GameState& state, const map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 访问次数最少的相邻可走格对应动作
Action chooseLeastVisitedNeighbor(const GameState& state, const map<Position, CellMemory>& memory)
{
    bool found = false;
    int bestVisit = 0;
    Direction bestDirection = Direction::UP;

    for (const DirectionInfo& direction : kDirections)
    {
        char cell = state.vision[direction.viewX][direction.viewY];
        if (!isWalkable(cell))
        {
            continue;
        }

        Position next = nextPosition(state.player.position, direction);
        int visitCount = 0;
        auto iter = memory.find(next);
        if (iter != memory.end())
        {
            visitCount = iter->second.visitCount;
        }

        if (!found || visitCount < bestVisit)
        {
            found = true;
            bestVisit = visitCount;
            bestDirection = direction.direction;
        }
    }

    return found ? makeMove(bestDirection) : Action{};
}
}

// Divide（分治探索算法）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 分治策略给出的下一步动作
Action Algorithm::Divide(const GameState& state, map<Position, CellMemory>& memory)
{
    // 第一层：直接处理 3x3 视野中的高价值目标。
    for (const DirectionInfo& direction : kDirections)
    {
        char cell = state.vision[direction.viewX][direction.viewY];
        if (cell == 'G' || cell == 'E' || cell == 'B')
        {
            return makeMove(direction.direction);
        }
    }

    // 第二层：在 AI 已知地图中选择访问次数较少、靠近未知区域的前沿格。
    bool hasTarget = false;
    Position bestTarget;
    int bestVisit = 0;
    int bestDistance = 0;

    for (const auto& item : memory)
    {
        const Position& position = item.first;
        const CellMemory& cell = item.second;
        if (!isFrontier(position, memory))
        {
            continue;
        }

        int distance = abs(position.x - state.player.position.x) +
                       abs(position.y - state.player.position.y);
        if (!hasTarget ||
            cell.visitCount < bestVisit ||
            (cell.visitCount == bestVisit && distance < bestDistance))
        {
            hasTarget = true;
            bestTarget = position;
            bestVisit = cell.visitCount;
            bestDistance = distance;
        }
    }

    if (hasTarget)
    {
        Action action = firstStepToTarget(state.player.position, bestTarget, memory);
        if (action.type != ActionType::NONE)
        {
            return action;
        }
    }

    // 第三层：如果找不到前沿路径，就在当前 3x3 内选择访问次数最少的相邻格。
    return chooseLeastVisitedNeighbor(state, memory);
}
