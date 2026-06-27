#include "algorithm.h"

// Dijkstra（Dijkstra算法接口）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 Dijkstra算法给出的动作
Action Algorithm::Dijkstra(const GameState& state, map<Position, CellMemory>& memory)
{
    (void)state;
    (void)memory;
    return Action{};
}
