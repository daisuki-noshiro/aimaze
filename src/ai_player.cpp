#include "ai_player.h"

#include "algorithm.h"

// AIPlayer（默认构造函数）输入形式 无 输入含义 无 输出形式 AIPlayer对象 输出含义 默认使用DIVIDE模式的玩家对象
AIPlayer::AIPlayer() = default;

// AIPlayer（指定探索模式构造函数）输入形式 ExploreMode mode 输入含义 本次运行使用的探索算法模式 输出形式 AIPlayer对象 输出含义 使用指定探索模式的玩家对象
AIPlayer::AIPlayer(ExploreMode mode)
    : exploreMode(mode)
{
}

// decide（生成下一步动作）输入形式 const GameState& state 输入含义 裁判层提供的当前可见状态 输出形式 Action 输出含义 AI本回合提交给裁判层的动作
Action AIPlayer::decide(const GameState& state)
{
    updateMemory(state);
    if (needBattle(state))
    {
        return battleStrategy(state);
    }
    return exploreStrategy(state);
}

// updateMemory（更新玩家记忆）输入形式 const GameState& state 输入含义 当前3x3视野和玩家位置 输出形式 void 输出含义 无返回值，直接更新memory
void AIPlayer::updateMemory(const GameState& state)
{
    Position center = state.player.position;
    for (int i = 0; i < static_cast<int>(state.vision.size()); ++i)
    {
        for (int j = 0; j < static_cast<int>(state.vision[i].size()); ++j)
        {
            Position pos{center.x + i - 1, center.y + j - 1};
            CellMemory& cell = memory[pos];
            cell.discovered = true;
            cell.type = state.vision[i][j];
        }
    }
    memory[center].visitCount++;
}

// needBattle（判断是否进入战斗策略）输入形式 const GameState& state 输入含义 当前游戏状态 输出形式 bool 输出含义 true表示当前处于Boss战
bool AIPlayer::needBattle(const GameState& state) const
{
    return state.inBattle;
}

// exploreStrategy（选择探索算法）输入形式 const GameState& state 输入含义 当前游戏状态 输出形式 Action 输出含义 探索阶段产生的动作
Action AIPlayer::exploreStrategy(const GameState& state)
{
    if (exploreMode == ExploreMode::ASTAR)
    {
        return Algorithm::AStar(state, memory);
    }
    if (exploreMode == ExploreMode::DIJKSTRA)
    {
        return Algorithm::Dijkstra(state, memory);
    }
    if (exploreMode == ExploreMode::BFS)
    {
        return Algorithm::BFS(state, memory);
    }
    if (exploreMode == ExploreMode::DIVIDE)
    {
        return Algorithm::Divide(state, memory);
    }
    return Algorithm::Greedy(state, memory);
}

// battleStrategy（选择战斗算法）输入形式 const GameState& state 输入含义 当前Boss战状态 输出形式 Action 输出含义 战斗阶段产生的技能动作
Action AIPlayer::battleStrategy(const GameState& state)
{
    return Algorithm::Battle(state);
}
