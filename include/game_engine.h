#pragma once

#include "model.h"

// 裁判层：唯一保存完整真实地图并修改游戏状态的模块。
class GameEngine
{
private:
    MapData map;
    GameConfig config;
    PlayerState player;
    BossState boss;
    GameStatus status = GameStatus::RUNNING;
    bool inBattle = false;

public:
    // GameEngine（默认构造函数）输入形式 无 输入含义 无初始化数据 输出形式 GameEngine对象 输出含义 创建空裁判层对象
    GameEngine();

    // GameEngine（初始化游戏）输入形式 const MapData& mapData, const GameConfig& gameConfig, const PlayerState& playerState, const BossState& bossState 输入含义 地图数据、游戏配置、玩家初始状态、Boss初始状态 输出形式 GameEngine对象 输出含义 创建可运行的裁判层对象
    GameEngine(const MapData& mapData,
               const GameConfig& gameConfig,
               const PlayerState& playerState,
               const BossState& bossState);

    // getState（获取当前游戏状态）输入形式 无 输入含义 无 输出形式 GameState 输出含义 玩家状态、Boss状态、3x3视野和战斗配置
    GameState getState() const;

    // step（执行玩家动作）输入形式 const Action& action 输入含义 AIPlayer提交的动作 输出形式 StepRecord 输出含义 动作前后状态和本步事件
    StepRecord step(const Action& action);

    // isGameOver（判断游戏是否结束）输入形式 无 输入含义 无 输出形式 bool 输出含义 true表示游戏已胜利或失败
    bool isGameOver() const;

private:
    // getVision（生成3x3视野）输入形式 无 输入含义 使用当前玩家位置 输出形式 vector<vector<char>> 输出含义 玩家周围3x3格子
    vector<vector<char>> getVision() const;

    // isWalkable（判断坐标是否可通行）输入形式 const Position& pos 输入含义 待判断坐标 输出形式 bool 输出含义 true表示在地图内且不是墙
    bool isWalkable(const Position& pos) const;

    // getNextPosition（计算下一坐标）输入形式 Direction direction 输入含义 移动方向 输出形式 Position 输出含义 玩家按该方向移动后的目标坐标
    Position getNextPosition(Direction direction) const;

    // handleMove（处理移动动作）输入形式 const Action& action 输入含义 MOVE类型动作 输出形式 vector<Event> 输出含义 移动产生的事件列表
    vector<Event> handleMove(const Action& action);

    // handleSkill（处理技能动作）输入形式 const Action& action 输入含义 USE_SKILL类型动作 输出形式 vector<Event> 输出含义 技能释放和Boss战产生的事件列表
    vector<Event> handleSkill(const Action& action);

    // updateCellEffect（处理进入格子的效果）输入形式 const Position& pos 输入含义 玩家进入的坐标 输出形式 vector<Event> 输出含义 金币、陷阱、Boss、终点等事件列表
    vector<Event> updateCellEffect(const Position& pos);

    // updateSkillCooldown（更新技能冷却）输入形式 无 输入含义 使用player.skills当前冷却状态 输出形式 void 输出含义 无返回值，直接修改技能冷却
    void updateSkillCooldown();

    // isAtBoss（判断玩家是否在Boss格）输入形式 无 输入含义 使用当前玩家位置 输出形式 bool 输出含义 true表示玩家位于Boss坐标
    bool isAtBoss() const;

    // isAtEnd（判断玩家是否在终点）输入形式 无 输入含义 使用当前玩家位置 输出形式 bool 输出含义 true表示玩家位于终点坐标
    bool isAtEnd() const;
};
