#include "algorithm.h"

#include <cstdlib>
#include <limits>

using namespace std;

namespace
{
static Direction lastDirection = Direction::UP;  //记录上次移动的方向
static bool hasLastDirection = false;

// makeMove（构造移动动作）
Action makeMove(Direction direction)
{
    Action action;
    action.type = ActionType::MOVE;
    action.direction = direction;
    return action;
}

// nextPosition（计算相邻坐标）
Position nextPosition(Position pos, int dx, int dy)
{
    return {pos.x + dx, pos.y + dy};
}

// isOpposite（判断是否反向移动）
bool isOpposite(Direction a, Direction b)
{
    return (a == Direction::UP && b == Direction::DOWN) ||
           (a == Direction::DOWN && b == Direction::UP) ||
           (a == Direction::LEFT && b == Direction::RIGHT) ||
           (a == Direction::RIGHT && b == Direction::LEFT);
}

// frontierValue（统计探索前沿数量）
int frontierValue(Position pos, const map<Position, CellMemory>& memory)
{
    int value = 0;

    if (memory.find({pos.x - 1, pos.y}) == memory.end()) ++value;
    if (memory.find({pos.x + 1, pos.y}) == memory.end()) ++value;
    if (memory.find({pos.x, pos.y - 1}) == memory.end()) ++value;
    if (memory.find({pos.x, pos.y + 1}) == memory.end()) ++value;

    return value;
}

// bossScore（计算Boss格分数）动态变化boss权重，防止太早或者太晚去打
int bossScore(const GameState& state)
{
    int cost = state.coinConsumption;

    if (cost <= 0)
    {
        cost = 1;
    }

    if (state.player.coins < cost)
    {
        return -300;
    }

    if (state.player.coins < cost * 2)
    {
        return -100;
    }

    if (state.player.coins < cost * 3)
    {
        return 100;
    }

    return 500;
}

// calcScore（计算相邻格评分）输入形式 char cell, Position pos, const GameState& state, const map<Position, CellMemory>& memory 输入含义 格子字符、格子坐标、当前状态、AI记忆地图 输出形式 int 输出含义 该方向相邻格子的综合评分
int calcScore(char cell,
              Position pos,
              const GameState& state,
              const map<Position, CellMemory>& memory)
{
    if (cell == '#')
    {
        return -9999;
    }

    int score = 0;

    if (cell == 'E')
    {
        score += 10000;
    }
    else if (cell == 'G')
    {
        score += 500;
    }
    else if (cell == 'T')
    {
        score -= 300;
    }
    else if (cell == 'B')
    {
        score += bossScore(state);
    }
    else
    {
        score += 30;
    }

    // 该格周围未知越多，越像探索前沿
    score += frontierValue(pos, memory) * 20;

    // 已经访问过多次的格子，降低优先级
    auto iter = memory.find(pos);

    if (iter != memory.end())
    {
        score -= iter->second.visitCount * 80;
    }
    else
    {
        score += 100;
    }

    return score;
}
}

// Divide（分治探索算法）输入形式 const GameState& state, map<Position, CellMemory>& memory 输入含义 当前状态、AI已知地图记忆 输出形式 Action 输出含义 对上下左右四个相邻格评分后得到的下一步动作
Action Algorithm::Divide(const GameState& state, map<Position, CellMemory>& memory)
{
    Position player = state.player.position; //获取当前玩家位置

    int upScore = calcScore(
        state.vision[0][1],
        nextPosition(player, -1, 0),
        state,
        memory
    );

    int downScore = calcScore(
        state.vision[2][1],
        nextPosition(player, 1, 0),
        state,
        memory
    );

    int leftScore = calcScore(
        state.vision[1][0],
        nextPosition(player, 0, -1),
        state,
        memory
    );

    int rightScore = calcScore(
        state.vision[1][2],
        nextPosition(player, 0, 1),
        state,
        memory
    );

    if (hasLastDirection && isOpposite(lastDirection, Direction::UP))  //重复走格子惩罚
    {
        upScore -= 250;
    }

    if (hasLastDirection && isOpposite(lastDirection, Direction::DOWN))
    {
        downScore -= 250;
    }

    if (hasLastDirection && isOpposite(lastDirection, Direction::LEFT))
    {
        leftScore -= 250;
    }

    if (hasLastDirection && isOpposite(lastDirection, Direction::RIGHT))
    {
        rightScore -= 250;
    }

    int bestScore = upScore;    //大小比较
    Direction bestDirection = Direction::UP;

    if (downScore > bestScore)
    {
        bestScore = downScore;
        bestDirection = Direction::DOWN;
    }

    if (leftScore > bestScore)
    {
        bestScore = leftScore;
        bestDirection = Direction::LEFT;
    }

    if (rightScore > bestScore)
    {
        bestScore = rightScore;
        bestDirection = Direction::RIGHT;
    }

    lastDirection = bestDirection;
    hasLastDirection = true;

    return makeMove(bestDirection);
}

