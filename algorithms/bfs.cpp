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
    // ======================================================
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

   
    // Step 1：生成候选目标（Target List）
    vector<Position> targets;

    // 从记忆中找已经发现的目标
    for (auto& it : memory)
    {
        if (!it.second.discovered) continue;

        char t = it.second.type;

        // 优先考虑：金币 / 终点 / Boss
        if (t == 'G' || t == 'E' || t == 'B')
            targets.push_back(it.first);
    }

    // 如果没有明确目标 → 转向探索模式
    if (targets.empty())
    {
        for (auto& it : memory)
        {
            if (!it.second.discovered || it.second.type == '#')
                continue;

            bool isFrontier = false;

            // 判断是否靠近未知区域
            for (auto& nxt : neighbors(it.first))
            {
                auto jt = memory.find(nxt);

                if (jt == memory.end() || !jt->second.discovered)
                {
                    isFrontier = true;
                    break;
                }
            }

            if (isFrontier)
                targets.push_back(it.first);
        }
    }

    // 如果仍然没有目标 → fallback安全走一步
    if (targets.empty())
        return safeMove(state, memory);

 
    // Step 2：Branch & Bound 搜索最优目标

    double bestScore = -1e18;
    Position bestTarget = targets.front();
    map<Position, Position> bestParent;

    // 对每一个候选目标做 BFS
    for (const auto& target : targets)
    {
        queue<Position> q;

        map<Position, int> dist;          // 当前路径代价
        map<Position, Position> parent;   // 路径回溯
        set<Position> visited;

        q.push(start);
        dist[start] = 0;

        while (!q.empty())
        {
            Position cur = q.front();
            q.pop();

            if (visited.count(cur)) continue;
            visited.insert(cur);

            int g = dist[cur];

         
            // Branch & Bound 剪枝条件
            // f = g + heuristic
            
            int bound = g + heuristic(cur, target);

            // 如果已经明显太差 → 剪枝
            if (bound > 60) continue;

          
            // 找到目标节点
            if (cur == target)
            {
                char type = memory.count(target)? memory[target].type: ' ';

                double reward = 0;

                
                // reward设计（核心评分函数）
              
                if (type == 'G') reward = 50;        // 金币
                else if (type == 'E') reward = 200;  // 终点
                else if (type == 'B') reward = 150;  // Boss
                else reward = 20 * frontierGain(target, memory);

                // 越短路径越好
                double score = reward -2.0 *g;

                // 更新最优解
                if (score > bestScore)
                {
                    bestScore = score;
                    bestTarget = target;
                    bestParent = parent;
                }

                continue;
            }

            // 扩展邻居节点
           
            for (auto& nxt : neighbors(cur))
            {
                auto it = memory.find(nxt);

                // 在线地图核心：允许未知，但要付代价
                int cost = 1;

                if (it == memory.end())
                    cost = 3;                 // 未知
                else if (!it->second.discovered)
                    cost = 3;                 // 不确定区域
                else
                    cost = cellCost(&it->second); // 已知区域

                if (dist.count(nxt))
                    continue;

                dist[nxt] = g + cost;
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

    // 如果没有路径 → fallback
    if (path.empty())
        return safeMove(state, memory);

    // 返回下一步动作
    return makeMove(start, path.back());
}
