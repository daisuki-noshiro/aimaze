#include "visualizer.h"

#include <filesystem>
#include <fstream>

namespace
{
// statusToString（游戏状态转字符串）输入形式 GameStatus status 输入含义 游戏状态枚举 输出形式 string 输出含义 可读状态文本
string statusToString(GameStatus status)
{
    if (status == GameStatus::WIN)
    {
        return "WIN";
    }
    if (status == GameStatus::LOSE)
    {
        return "LOSE";
    }
    return "RUNNING";
}

// actionToString（动作转字符串）输入形式 const Action& action 输入含义 玩家动作 输出形式 string 输出含义 可读动作文本
string actionToString(const Action& action)
{
    if (action.type == ActionType::NONE)
    {
        return "NONE";
    }
    if (action.type == ActionType::USE_SKILL)
    {
        return "USE_SKILL(" + to_string(action.skillIndex) + ")";
    }

    string direction = "UP";
    if (action.direction == Direction::DOWN)
    {
        direction = "DOWN";
    }
    else if (action.direction == Direction::LEFT)
    {
        direction = "LEFT";
    }
    else if (action.direction == Direction::RIGHT)
    {
        direction = "RIGHT";
    }
    return "MOVE(" + direction + ")";
}

// eventToString（事件转字符串）输入形式 EventType type 输入含义 事件类型枚举 输出形式 string 输出含义 可读事件文本
string eventToString(EventType type)
{
    switch (type)
    {
    case EventType::MOVE_SUCCESS: return "MOVE_SUCCESS";
    case EventType::MOVE_BLOCKED: return "MOVE_BLOCKED";
    case EventType::PICK_GOLD: return "PICK_GOLD";
    case EventType::TRIGGER_TRAP: return "TRIGGER_TRAP";
    case EventType::ENTER_BOSS: return "ENTER_BOSS";
    case EventType::USE_SKILL_SUCCESS: return "USE_SKILL_SUCCESS";
    case EventType::USE_SKILL_FAILED: return "USE_SKILL_FAILED";
    case EventType::BOSS_DAMAGED: return "BOSS_DAMAGED";
    case EventType::BOSS_DEFEATED: return "BOSS_DEFEATED";
    case EventType::REVIVE: return "REVIVE";
    case EventType::REACH_END: return "REACH_END";
    case EventType::GAME_WIN: return "GAME_WIN";
    case EventType::GAME_LOSE: return "GAME_LOSE";
    default: return "NONE";
    }
}

// ensureParentDirectory（确保父目录存在）输入形式 const string& path 输入含义 输出文件路径 输出形式 void 输出含义 无返回值，必要时创建父目录
void ensureParentDirectory(const string& path)
{
    filesystem::path filePath(path);
    if (filePath.has_parent_path())
    {
        filesystem::create_directories(filePath.parent_path());
    }
}

// writePathJson（输出路径JSON数组）输入形式 ofstream& out, const vector<Position>& path 输入含义 输出流、路径坐标列表 输出形式 void 输出含义 无返回值，写入二维坐标数组
void writePathJson(ofstream& out, const vector<Position>& path)
{
    out << "[";
    for (size_t i = 0; i < path.size(); ++i)
    {
        if (i > 0)
        {
            out << ",";
        }
        out << "[" << path[i].x << "," << path[i].y << "]";
    }
    out << "]";
}

// writeIntArrayJson（输出整数JSON数组）输入形式 ofstream& out, const vector<int>& values 输入含义 输出流、整数列表 输出形式 void 输出含义 无返回值，写入一维整数数组
void writeIntArrayJson(ofstream& out, const vector<int>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
        {
            out << ",";
        }
        out << values[i];
    }
    out << "]";
}

// writeSkillSequencesJson（输出技能序列JSON数组）输入形式 ofstream& out, const vector<vector<int>>& sequences 输入含义 输出流、技能序列二维数组 输出形式 void 输出含义 无返回值，写入二维整数数组
void writeSkillSequencesJson(ofstream& out, const vector<vector<int>>& sequences)
{
    out << "[";
    for (size_t i = 0; i < sequences.size(); ++i)
    {
        if (i > 0)
        {
            out << ",";
        }
        writeIntArrayJson(out, sequences[i]);
    }
    out << "]";
}
}

// saveResult（保存最终结果）输入形式 const GameResult& result, const string& path 输入含义 游戏统计结果、输出文件路径 输出形式 void 输出含义 无返回值，写入接口对齐的JSON结果文件
void Visualizer::saveResult(const GameResult& result, const string& path)
{
    ensureParentDirectory(path);
    ofstream out(path);

    int moveSteps = result.steps;
    int pathLength = static_cast<int>(result.path.size());
    double ratio = result.status == GameStatus::WIN && moveSteps != 0
        ? static_cast<double>(result.coins) / moveSteps
        : 0.0;

    out << "{\n";
    out << "  \"success\": " << (result.status == GameStatus::WIN ? "true" : "false") << ",\n";
    out << "  \"path\": ";
    writePathJson(out, result.path);
    out << ",\n";
    out << "  \"path_length\": " << pathLength << ",\n";
    out << "  \"move_steps\": " << moveSteps << ",\n";
    out << "  \"final_coin\": " << result.coins << ",\n";
    out << "  \"coin_step_ratio\": " << ratio << ",\n";
    out << "  \"boss_success\": " << (result.bossSuccess ? "true" : "false") << ",\n";
    out << "  \"boss_total_turns\": " << result.bossTotalTurns << ",\n";
    out << "  \"boss_revive_count\": " << result.bossReviveCount << ",\n";
    out << "  \"boss_coin_cost\": " << result.bossCoinCost << ",\n";
    out << "  \"boss_skill_sequence_lengths\": ";
    writeIntArrayJson(out, result.bossSkillSequenceLengths);
    out << ",\n";
    out << "  \"boss_skill_sequences\": ";
    writeSkillSequencesJson(out, result.bossSkillSequences);
    out << "\n";
    out << "}\n";
}

// saveProcess（保存运行过程）输入形式 const vector<StepRecord>& records, const string& path 输入含义 每步记录列表、输出文件路径 输出形式 void 输出含义 无返回值，写入过程文本文件
void Visualizer::saveProcess(const vector<StepRecord>& records, const string& path)
{
    ensureParentDirectory(path);
    ofstream out(path);
    for (size_t i = 0; i < records.size(); ++i)
    {
        const StepRecord& record = records[i];
        out << "step " << i + 1 << "\n";
        out << "action: " << actionToString(record.action) << "\n";
        out << "position: (" << record.after.player.position.x << ", "
            << record.after.player.position.y << ")\n";
        out << "coins: " << record.after.player.coins << "\n";
        out << "events:";
        for (const Event& event : record.events)
        {
            out << " " << eventToString(event.type);
        }
        out << "\n\n";
    }
}
