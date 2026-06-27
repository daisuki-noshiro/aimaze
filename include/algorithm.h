#pragma once

#include "model.h"

// 算法层：只根据 GameState 和 AI memory 计算 Action，不修改游戏状态。
namespace Algorithm
{
    // Greedy（贪心算法接口）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 贪心算法给出的动作
    Action Greedy(const GameState& state, map<Position, CellMemory>& memory);

    // AStar（A星算法接口）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 A星算法给出的动作
    Action AStar(const GameState& state, map<Position, CellMemory>& memory);

    // Dijkstra（Dijkstra算法接口）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 Dijkstra算法给出的动作
    Action Dijkstra(const GameState& state, map<Position, CellMemory>& memory);

    // BFS（BFS或分支限界算法接口）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 BFS或分支限界算法给出的动作
    Action BFS(const GameState& state, map<Position, CellMemory>& memory);

    // Divide（分治算法接口）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 分治算法给出的动作
    Action Divide(const GameState& state, map<Position, CellMemory>& memory);

    // Battle（Boss战算法接口）输入形式 const GameState& state 输入含义 当前Boss战状态 输出形式 Action 输出含义 技能动作或NONE动作
    Action Battle(const GameState& state);
}
