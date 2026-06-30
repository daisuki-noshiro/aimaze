#include "game_recorder.h"

#include <cstdlib>

// record（记录单步过程）输入形式 const StepRecord& record 输入含义 裁判层返回的单步记录 输出形式 void 输出含义 无返回值，直接保存到records
void GameRecorder::record(const StepRecord& record)
{
    records.push_back(record);
}

// getResult（生成最终统计结果）输入形式 无 输入含义 使用内部records 输出形式 GameResult 输出含义 最终状态、金币、步数、路径、动作、事件和Boss统计
GameResult GameRecorder::getResult() const
{
    GameResult result;
    if (records.empty())
    {
        return result;
    }

    const StepRecord& first = records.front();
    const StepRecord& last = records.back();
    result.status = last.after.status;
    result.coins = last.after.player.coins;
    result.steps = last.after.player.steps;
    result.ratio = result.status == GameStatus::WIN && result.steps != 0
        ? static_cast<double>(result.coins) / result.steps
        : 0.0;
    result.bossSuccess = last.after.boss.hpList.empty() ||
        last.after.boss.currentBoss >= static_cast<int>(last.after.boss.hpList.size());

    result.path.push_back(first.before.player.position);

    vector<int> currentBossSequence;
    for (const StepRecord& record : records)
    {
        result.actions.push_back(record.action);
        result.events.insert(result.events.end(), record.events.begin(), record.events.end());

        bool moveSuccess = false;
        bool revived = false;
        bool usedSkill = false;
        bool bossFinished = false;

        for (const Event& event : record.events)
        {
            if (event.type == EventType::MOVE_SUCCESS)
            {
                moveSuccess = true;
            }
            else if (event.type == EventType::USE_SKILL_SUCCESS)
            {
                usedSkill = true;
                ++result.bossTotalTurns;
            }
            else if (event.type == EventType::REVIVE)
            {
                revived = true;
                ++result.bossReviveCount;
                result.bossCoinCost += std::abs(event.coinDelta);
            }
            else if (event.type == EventType::BOSS_DEFEATED &&
                     record.after.boss.currentBoss >= static_cast<int>(record.after.boss.hpList.size()))
            {
                bossFinished = true;
            }
        }

        if (moveSuccess)
        {
            result.path.push_back(record.after.player.position);
        }

        if (usedSkill && record.action.type == ActionType::USE_SKILL)
        {
            currentBossSequence.push_back(record.action.skillIndex);
        }

        if ((revived || bossFinished) && !currentBossSequence.empty())
        {
            result.bossSkillSequenceLengths.push_back(static_cast<int>(currentBossSequence.size()));
            result.bossSkillSequences.push_back(currentBossSequence);
            currentBossSequence.clear();
        }
    }

    if (!currentBossSequence.empty())
    {
        result.bossSkillSequenceLengths.push_back(static_cast<int>(currentBossSequence.size()));
        result.bossSkillSequences.push_back(currentBossSequence);
    }

    return result;
}

// getRecords（获取过程记录）输入形式 无 输入含义 使用内部records 输出形式 const vector<StepRecord>& 输出含义 只读的完整过程记录列表
const vector<StepRecord>& GameRecorder::getRecords() const
{
    return records;
}
