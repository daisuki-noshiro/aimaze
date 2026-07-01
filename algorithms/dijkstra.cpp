#include "algorithm.h"

#include <queue>

namespace
{
    // DirectionInfo（方向信息结构体）输入形式 无 输入含义 无 输出形式 DirectionInfo 输出含义 保存方向、视野坐标和坐标增量
    struct DirectionInfo
    {
        Direction direction;
        int viewX;   // 在 3x3 视野中的行索引
        int viewY;   // 在 3x3 视野中的列索引
        int dx;      // 在地图中的行偏移
        int dy;      // 在地图中的列偏移
    };

    // 四个移动方向：顺序影响同分时的选择
    const vector<DirectionInfo> kDirections = {
        {Direction::RIGHT, 1, 2, 0, 1},
        {Direction::DOWN, 2, 1, 1, 0},
        {Direction::LEFT, 1, 0, 0, -1},
        {Direction::UP, 0, 1, -1, 0}};

    // makeMove（构造移动动作）输入形式 Direction direction 输入含义 移动方向 输出形式 Action 输出含义 MOVE 类型动作
    Action makeMove(Direction direction)
    {
        Action action;
        action.type = ActionType::MOVE;
        action.direction = direction;
        return action;
    }

    // isWalkable（判断格子是否可通行）输入形式 char cell 输入含义 地图格子字符 输出形式 bool 输出含义 true 表示不是墙
    bool isWalkable(char cell)
    {
        return cell != '#';
    }

    // nextPosition（计算相邻坐标）输入形式 Position p, const DirectionInfo& d 输入含义 当前坐标、方向信息 输出形式 Position 输出含义 相邻目标坐标
    Position nextPosition(Position position, const DirectionInfo& direction)
    {
        return {position.x + direction.dx, position.y + direction.dy};
    }

    // cellWeight（Dijkstra 加权边的权重函数）输入形式 char cell 输入含义 格子类型 输出形式 double 输出含义 通过该格的代价值
    // 金币（G）权重低于 1 → 算法优先选择经过金币的路径（鼓励收集资源）
    // 陷阱（T）权重远高于 1 → 算法尽量绕开陷阱（除非绕路代价过大）
    // 空地、终点、Boss 格 → 标准权重 1
    double cellWeight(char cell)
    {
        switch (cell)
        {
        case 'G': // 金币：低权重，鼓励经过
            return 0.2;
        case 'T': // 陷阱：高权重，促使绕路
            return 8.0;
        case ' ': // 空地
        case 'E': // 终点
        case 'B': // Boss 格
            return 1.0;
        default: // 未知、墙壁等
            return 1.0;
        }
    }

    // isFrontier（判断是否为探索前沿）输入形式 const Position& p, const map<Position, CellMemory>& m 输入含义 待判断坐标、AI记忆 输出形式 bool 输出含义 true表示该格可走且邻接未探索区域
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
                return true; // 存在未探索的邻居
            }
        }
        return false;
    }

    // DistCompare（优先队列比较器）输入形式 两个 pair<double, Position> 输入含义 加权距离和坐标 输出形式 bool 输出含义 true 表示 a 排在 b 后面，实现小顶堆
    struct DistCompare
    {
        bool operator()(const pair<double, Position>& a, const pair<double, Position>& b) const
        {
            return a.first > b.first;
        }
    };

    // dijkstraFirstStep（加权 Dijkstra 寻路，返回通往目标的第一步）输入形式 Position start, Position target, const map<Position, CellMemory>& memory
    // 输入含义 起点、目标、AI已知地图记忆 输出形式 Action 输出含义 通往目标的第一步 MOVE 动作，找不到则为 NONE
    Action dijkstraFirstStep(Position start, Position target, const map<Position, CellMemory>& memory)
    {
        if (start == target)
        {
            return Action{};
        }

        map<Position, double> distance;                          // 从起点到各位置的当前已知最短距离
        map<Position, Direction> firstDirection;                  // 从起点到达各位置的第一步方向
        priority_queue<pair<double, Position>, vector<pair<double, Position>>, DistCompare> pq;

        distance[start] = 0.0;
        pq.push({0.0, start});

        while (!pq.empty())
        {
            auto [dist, current] = pq.top();
            pq.pop();

            // 跳过已过时的记录（同一位置可能有多次入队）
            auto distIter = distance.find(current);
            if (distIter == distance.end() || distIter->second < dist)
            {
                continue;
            }

            // 到达目标，返回第一步动作
            if (current == target)
            {
                return makeMove(firstDirection[current]);
            }

            for (const DirectionInfo& direction : kDirections)
            {
                Position next = nextPosition(current, direction);

                auto cell = memory.find(next);
                if (cell == memory.end() || !isWalkable(cell->second.type))
                {
                    continue;
                }

                double newDist = dist + cellWeight(cell->second.type);
                auto nextDistIter = distance.find(next);

                if (nextDistIter == distance.end() || newDist < nextDistIter->second)
                {
                    distance[next] = newDist;
                    // 起点的邻居记录自己的方向，其余继承第一步方向
                    firstDirection[next] = (current == start) ? direction.direction : firstDirection[current];
                    pq.push({newDist, next});
                }
            }
        }

        return Action{}; // 无可达路径
    }

    // chooseBestNeighbor（在 3x3 视野中选择最优相邻格）输入形式 const GameState& state 输入含义 当前状态 输出形式 Action 输出含义 最优方向 MOVE 动作
    Action chooseBestNeighbor(const GameState& state)
    {
        bool found = false;
        double bestWeight = 0.0;
        Direction bestDirection = Direction::UP;

        for (const DirectionInfo& direction : kDirections)
        {
            char cell = state.vision[direction.viewX][direction.viewY];
            if (!isWalkable(cell))
            {
                continue;
            }

            double weight = cellWeight(cell);
            if (!found || weight < bestWeight)
            {
                found = true;
                bestWeight = weight;
                bestDirection = direction.direction;
            }
        }

        return found ? makeMove(bestDirection) : Action{};
    }
}

// Dijkstra（加权 Dijkstra 迷宫探索算法）输入形式 const GameState& state, map<Position, CellMemory>& memory
// 输入含义 当前游戏状态、AI 已知地图记忆 输出形式 Action 输出含义 Dijkstra 策略给出的下一步动作
//
// 四层策略：
//   第一层（直接目标）：E/G/B 在 3x3 视野中 → 直接走过去
//   第二层（Dijkstra 到终点）：终点 E 已在 memory 中 → 加权 Dijkstra 寻路到终点
//   第三层（Dijkstra 探索前沿）：终点未知 → 加权 Dijkstra 到最近的探索前沿格
//   第四层（回退策略）：前沿不可达 → 选择 3x3 视野中权重最优的相邻格
Action Algorithm::Dijkstra(const GameState& state, map<Position, CellMemory>& memory)
{
    // ===== 第一层：直接目标处理 =====
    // 如果 3x3 视野内直接有金币（G）、终点（E）或 Boss 格（B），直接走过去
    for (const DirectionInfo& direction : kDirections)
    {
        char cell = state.vision[direction.viewX][direction.viewY];
        if (cell == 'E' || cell == 'G' || cell == 'B')
        {
            return makeMove(direction.direction);
        }
    }

    // ===== 第二层：若已知终点位置，运行加权 Dijkstra 到终点 =====
    // 终点 E 在探索过程中被发现后，存入 memory，即可使用 Dijkstra 规划最短路径
    Position endPos;
    bool endKnown = false;
    for (const auto& item : memory)
    {
        if (item.second.type == 'E')
        {
            endPos = item.first;
            endKnown = true;
            break;
        }
    }

    if (endKnown)
    {
        Action action = dijkstraFirstStep(state.player.position, endPos, memory);
        if (action.type != ActionType::NONE)
        {
            return action;
        }
    }

    // ===== 第三层：加权 Dijkstra 探索到最近前沿 =====
    // 在尚无完整地图（终点未知）时，以探索未知区域为目标
    // 利用 Dijkstra 的出队性质：第一个被出队的探索前沿格即为加权距离最近的前沿
    // 加权边的设计使得算法在到达前沿的路上优先经过金币、避开陷阱
    map<Position, double> distance;
    map<Position, Direction> firstDirection;
    priority_queue<pair<double, Position>, vector<pair<double, Position>>, DistCompare> pq;

    distance[state.player.position] = 0.0;
    pq.push({0.0, state.player.position});

    while (!pq.empty())
    {
        auto [dist, current] = pq.top();
        pq.pop();

        // 跳过已过时的记录
        auto distIter = distance.find(current);
        if (distIter == distance.end() || distIter->second < dist)
        {
            continue;
        }

        // 检查是否到达探索前沿（排除起点自身，起点不满足前沿条件但显式跳过更安全）
        if (!(current == state.player.position) && isFrontier(current, memory))
        {
            // 由 Dijkstra 正确性保证：第一个出队的前沿格就是加权距离最近的前沿格
            return makeMove(firstDirection[current]);
        }

        for (const DirectionInfo& direction : kDirections)
        {
            Position next = nextPosition(current, direction);

            auto cell = memory.find(next);
            if (cell == memory.end() || !isWalkable(cell->second.type))
            {
                continue;
            }

            double newDist = dist + cellWeight(cell->second.type);
            auto nextDistIter = distance.find(next);

            if (nextDistIter == distance.end() || newDist < nextDistIter->second)
            {
                distance[next] = newDist;
                firstDirection[next] = (current == state.player.position) ? direction.direction : firstDirection[current];
                pq.push({newDist, next});
            }
        }
    }

    // ===== 第四层：回退策略 =====
    // 如果所有已知可通行区域都无法通向任何前沿，在 3x3 视野中选择最优相邻格
    // 优先选择权重最低的格子（金币权重 0.2 < 空地 1.0 < 陷阱 8.0）
    return chooseBestNeighbor(state);
}
