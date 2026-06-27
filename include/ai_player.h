#pragma once

#include "model.h"

// 玩家层：只根据 GameState 的 3x3 视野和自身 memory 生成 Action。
class AIPlayer
{
private:
    map<Position, CellMemory> memory;
    ExploreMode exploreMode = ExploreMode::DIVIDE;

public:
    // AIPlayer（默认构造函数）输入形式 无 输入含义 无 输出形式 AIPlayer对象 输出含义 默认使用DIVIDE模式的玩家对象
    AIPlayer();

    // AIPlayer（指定探索模式构造函数）输入形式 ExploreMode mode 输入含义 本次运行使用的探索算法模式 输出形式 AIPlayer对象 输出含义 使用指定探索模式的玩家对象
    explicit AIPlayer(ExploreMode mode);

    // decide（生成下一步动作）输入形式 const GameState& state 输入含义 裁判层提供的当前可见状态 输出形式 Action 输出含义 AI本回合提交给裁判层的动作
    Action decide(const GameState& state);

private:
    // updateMemory（更新玩家记忆）输入形式 const GameState& state 输入含义 当前3x3视野和玩家位置 输出形式 void 输出含义 无返回值，直接更新memory
    void updateMemory(const GameState& state);

    // needBattle（判断是否进入战斗策略）输入形式 const GameState& state 输入含义 当前游戏状态 输出形式 bool 输出含义 true表示当前处于Boss战
    bool needBattle(const GameState& state) const;

    // exploreStrategy（选择探索算法）输入形式 const GameState& state 输入含义 当前游戏状态 输出形式 Action 输出含义 探索阶段产生的动作
    Action exploreStrategy(const GameState& state);

    // battleStrategy（选择战斗算法）输入形式 const GameState& state 输入含义 当前Boss战状态 输出形式 Action 输出含义 战斗阶段产生的技能动作
    Action battleStrategy(const GameState& state);
};
