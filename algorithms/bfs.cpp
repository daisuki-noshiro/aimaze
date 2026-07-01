#include "algorithm.h"
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <limits>
#include <cmath>

using namespace std;

namespace
{
    
    // 判断是否可走（墙不能走）
   
    bool walkable(char c)
    {
        return c != '#';
    }

    // 曼哈顿距离启发函数
    // 用于 Branch & Bound 剪枝

    int heuristic(const Position& a, const Position& b)
    {
        return abs(a.x - b.x) + abs(a.y - b.y);
    }

    // 获取上下左右四个邻居格子
    vector<Position> neighbors(const Position& p)
    {
        return {
            {p.x - 1, p.y},
            {p.x + 1, p.y},
            {p.x, p.y - 1},
            {p.x, p.y + 1}
        };
    }

 
    // 生成移动动作

    Action makeMove(const Position& from, const Position& to)
    {
        Action action;
        action.type = ActionType::MOVE;

        if (to.x < from.x)
            action.direction = Direction::UP;
        else if (to.x > from.x)
            action.direction = Direction::DOWN;
        else if (to.y < from.y)
            action.direction = Direction::LEFT;
        else
            action.direction = Direction::RIGHT;

        return action;
    }

   
    // 设计思想：
    // - 未探索：有风险 → 中等惩罚
    // - 墙：不可走 → 极大惩罚
    // - 陷阱：高惩罚
    // - 金币/出口/Boss：鼓励
   
    int cellCost(const CellMemory* cell)
    {
        if (!cell) return 3;              // 完全未知：中等风险
        if (!cell->discovered) return 3;  // 未确认区域：仍然危险

        if (cell->type == '#') return 100; // 墙（禁止）
        if (cell->type == 'T') return 30;  // 陷阱（高风险）
        if (cell->type == 'G') return 1;   // 金币（奖励）
        if (cell->type == 'E') return 1;   // 终点
        if (cell->type == 'B') return 1;   // Boss

        return 1;
    }

    // frontier评分（探索驱动）
    // 用来判断一个位置“是否靠近未知区域”
    // 未知越多 → 越值得探索
 
    int frontierGain(const Position& pos,
        map<Position, CellMemory>& memory)
    {
        int cnt = 0;

        for (auto& nxt : neighbors(pos))
        {
            auto it = memory.find(nxt);

            // 如果邻居不存在或没探索 → 说明该位置靠近未知区域
            if (it == memory.end() || !it->second.discovered)
                cnt++;
        }

        return cnt;
    }


    // fallback策略：找不到目标时的安全移动

    Action safeMove(const GameState& state,
        map<Position, CellMemory>& memory)
    {
        Position s = state.player.position;

        for (auto& nxt : neighbors(s))
        {
            auto it = memory.find(nxt);

            // 只走已探索 + 可通行区域
            if (it != memory.end() &&
                it->second.discovered &&
                walkable(it->second.type))
            {
                return makeMove(s, nxt);
            }
        }

        // 如果没有路 → 随便走一步避免卡死
        Action a;
        a.type = ActionType::MOVE;
        a.direction = Direction::RIGHT;
        return a;
    }
}

// BFS + Branch & Bound

// 特点：
//  3×3视野
//  memory逐步扩展地图
//  unknown区域允许但有惩罚
// heuristic剪枝
// reward驱动选择目标

Action Algorithm::BFS(
    const GameState& state,
    map<Position, CellMemory>& memory)
{
    Position start = state.player.position;

    vector<Position> targets;

   
    // Step 1：找目标
    
    for (auto& it : memory)
    {
        if (!it.second.discovered) continue;

        char t = it.second.type;
        if (t == 'G' || t == 'E' || t == 'B')
            targets.push_back(it.first);
    }

    // 没目标 → 探索frontier
    if (targets.empty())
    {
        for (auto& it : memory)
        {
            if (!it.second.discovered || it.second.type == '#')
                continue;

            for (auto& nxt : neighbors(it.first))
            {
                auto jt = memory.find(nxt);
                if (jt == memory.end() || !jt->second.discovered)
                {
                    targets.push_back(it.first);
                    break;
                }
            }
        }
    }

    if (targets.empty())
        return safeMove(state, memory);

   
    // Step 2：搜索
    
    double bestScore = -1e18;
    int bestKnownCost = INT_MAX;

    Position bestTarget = targets.front();
    map<Position, Position> bestParent;

    for (const auto& target : targets)
    {
        queue<Position> q;
        map<Position, int> dist;
        map<Position, Position> parent;

        q.push(start);
        dist[start] = 0;

        while (!q.empty())
        {
            Position cur = q.front();
            q.pop();

            int g = dist[cur];

          
            // 剪枝
            
            int bound = g + heuristic(cur, target);

            int limit;
            if (bestKnownCost == INT_MAX)
                limit = 200;              // 初始宽松
            else
                limit = bestKnownCost + 10; // 自适应收紧

            if (bound > limit)
                continue;

            
            // 到达目标
         
            if (cur == target)
            {
                char type = memory.count(target)
                    ? memory[target].type
                    : ' ';

                double reward = 0;

                if (type == 'G') reward = 50;
                else if (type == 'E') reward = 200;
                else if (type == 'B') reward = 150;
                else reward = 20 * frontierGain(target, memory);

                double score = reward - 2.0 * g;

                if (score > bestScore)
                {
                    bestScore = score;
                    bestTarget = target;
                    bestParent = parent;
                    bestKnownCost = min(bestKnownCost, g);
                }

                continue;
            }

          
            // 扩展邻居
           
            for (auto& nxt : neighbors(cur))
            {
                auto it = memory.find(nxt);

                int cost = 1;

                if (it == memory.end())
                    cost = 3;
                else if (!it->second.discovered)
                    cost = 3;
                else
                    cost = cellCost(&it->second);

                int newCost = g + cost;

                // 更优路径更新
                if (dist.count(nxt) && dist[nxt] <= newCost)
                    continue;

                dist[nxt] = newCost;
                parent[nxt] = cur;
                q.push(nxt);
            }
        }
    }

    
    // Step 3：回溯路径
    
    vector<Position> path;
    Position cur = bestTarget;

    while (!(cur == start))
    {
        path.push_back(cur);

        if (!bestParent.count(cur))
            break;

        cur = bestParent[cur];
    }

    if (path.empty())
        return safeMove(state, memory);

    return makeMove(start, path.back());
}
