#include "game_recorder.h"

// record（记录单步过程）输入形式 const StepRecord& record 输入含义 裁判层返回的单步记录 输出形式 void 输出含义 无返回值，直接保存到records
void GameRecorder::record(const StepRecord& record)
{
    records.push_back(record);
}

// getResult（生成最终统计结果）输入形式 无 输入含义 使用内部records 输出形式 GameResult 输出含义 最终状态、金币、步数、路径、动作和事件汇总
GameResult GameRecorder::getResult() const
{
    GameResult result;
    if (records.empty())
    {
        return result;
    }

    const StepRecord& last = records.back();
    result.status = last.after.status;
    result.coins = last.after.player.coins;
    result.steps = last.after.player.steps;
    result.ratio = result.steps == 0 ? 0.0 : static_cast<double>(result.coins) / result.steps;

    for (const StepRecord& record : records)
    {
        result.path.push_back(record.after.player.position);
        result.actions.push_back(record.action);
        result.events.insert(result.events.end(), record.events.begin(), record.events.end());
    }

    return result;
}

// getRecords（获取过程记录）输入形式 无 输入含义 使用内部records 输出形式 const vector<StepRecord>& 输出含义 只读的完整过程记录列表
const vector<StepRecord>& GameRecorder::getRecords() const
{
    return records;
}
