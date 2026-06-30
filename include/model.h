#pragma once

#include <map>
#include <string>
#include <vector>

using namespace std;

// 游戏状态：表示整局游戏当前是否仍在运行、胜利或失败。
enum class GameStatus
{
    RUNNING,
    WIN,
    LOSE
};

// 动作类型：AI 每回合只能选择无动作、移动或使用技能。
enum class ActionType
{
    NONE,
    MOVE,
    USE_SKILL
};

// 事件类型：GameEngine 执行动作后产生的结果事件。
enum class EventType
{
    NONE,
    MOVE_SUCCESS,
    MOVE_BLOCKED,
    PICK_GOLD,
    TRIGGER_TRAP,
    ENTER_BOSS,
    USE_SKILL_SUCCESS,
    USE_SKILL_FAILED,
    BOSS_DAMAGED,
    BOSS_DEFEATED,
    REVIVE,
    REACH_END,
    GAME_WIN,
    GAME_LOSE
};

// 移动方向：用于 MOVE 动作。
enum class Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// 探索模式：用于选择 AIPlayer 调用哪一种探索算法接口。
enum class ExploreMode
{
    GREEDY,
    ASTAR,
    DIJKSTRA,
    BFS,
    DIVIDE
};

// 坐标：x 表示行，y 表示列，可作为 map 的键。
struct Position
{
    int x = 0;
    int y = 0;

    // operator<（坐标排序比较）输入形式 const Position& other 输入含义 另一个坐标 输出形式 bool 输出含义 true表示当前坐标排序在other之前
    bool operator<(const Position& other) const
    {
        if (x != other.x)
        {
            return x < other.x;
        }
        return y < other.y;
    }

    // operator==（坐标相等比较）输入形式 const Position& other 输入含义 另一个坐标 输出形式 bool 输出含义 true表示两个坐标的x和y都相等
    bool operator==(const Position& other) const
    {
        return x == other.x && y == other.y;
    }
};

// 技能：保存技能伤害、基础冷却和当前剩余冷却。
struct Skill
{
    int damage = 0;
    int cooldown = 0;
    int currentCD = 0;
};

// 玩家动作：由 AIPlayer 生成，交给 GameEngine 执行。
struct Action
{
    ActionType type = ActionType::NONE;
    Direction direction = Direction::UP;
    int skillIndex = -1;
};

// 游戏事件：GameEngine 执行动作后产生，用于记录和可视化。
struct Event
{
    EventType type = EventType::NONE;
    Position position;
    int coinDelta = 0;
    int hpDelta = 0;
    int skillIndex = -1;
    int bossIndex = -1;
};

// 地图数据：保存完整真实地图，只应由 JsonLoader 和 GameEngine 使用。
struct MapData
{
    vector<vector<char>> maze;
    Position start;
    Position end;
    Position boss;
};

// 游戏配置：保存 Boss 战限制回合数和复活金币消耗。
struct GameConfig
{
    int minRounds = 0;
    int coinConsumption = 0;
};

// 玩家状态：保存玩家位置、金币、步数和技能列表。
struct PlayerState
{
    Position position;
    int coins = 0;
    int steps = 0;
    vector<Skill> skills;
};

// Boss 状态：保存 Boss 血量列表、当前 Boss 下标和当前战斗回合数。
struct BossState
{
    vector<int> hpList;
    int currentBoss = 0;
    int currentRound = 0;
};

// 当前游戏状态：GameEngine 返回给 AIPlayer 的只读信息，vision 仅为 3x3。
struct GameState
{
    GameStatus status = GameStatus::RUNNING;
    PlayerState player;
    BossState boss;
    vector<vector<char>> vision;
    bool inBattle = false;
    int minRounds = 0;
    int coinConsumption = 0;
};

// 格子记忆：AIPlayer 基于 3x3 视野逐步建立的已知地图。
struct CellMemory
{
    bool discovered = false;
    char type = '?';
    int visitCount = 0;
};

// 单步记录：保存动作执行前状态、动作、事件列表和动作执行后状态。
struct StepRecord
{
    GameState before;
    Action action;
    vector<Event> events;
    GameState after;
};

// 游戏结果：由 GameRecorder 根据 StepRecord 汇总得到。
struct GameResult
{
    GameStatus status = GameStatus::RUNNING;
    int coins = 0;
    int steps = 0;
    double ratio = 0.0;
    vector<Position> path;
    vector<Action> actions;
    vector<Event> events;
    bool bossSuccess = false;
    int bossTotalTurns = 0;
    int bossReviveCount = 0;
    int bossCoinCost = 0;
    vector<int> bossSkillSequenceLengths;
    vector<vector<int>> bossSkillSequences;
};
