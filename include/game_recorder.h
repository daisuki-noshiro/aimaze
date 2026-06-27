#pragma once

#include "model.h"

// 统计层：只保存 StepRecord，并从记录中生成 GameResult。
class GameRecorder
{
private:
    vector<StepRecord> records;

public:
    // record（记录单步过程）输入形式 const StepRecord& record 输入含义 裁判层返回的单步记录 输出形式 void 输出含义 无返回值，直接保存到records
    void record(const StepRecord& record);

    // getResult（生成最终统计结果）输入形式 无 输入含义 使用内部records 输出形式 GameResult 输出含义 最终状态、金币、步数、路径、动作和事件汇总
    GameResult getResult() const;

    // getRecords（获取过程记录）输入形式 无 输入含义 使用内部records 输出形式 const vector<StepRecord>& 输出含义 只读的完整过程记录列表
    const vector<StepRecord>& getRecords() const;
};
