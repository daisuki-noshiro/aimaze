#pragma once

#include <string>

#include "model.h"

// 可视化层：只负责把结果和过程输出为文本，不参与游戏逻辑。
class Visualizer
{
public:
    // saveResult（保存最终结果）输入形式 const GameResult& result, const string& path 输入含义 游戏统计结果、输出文件路径 输出形式 void 输出含义 无返回值，写入结果文本文件
    static void saveResult(const GameResult& result, const string& path);

    // saveProcess（保存运行过程）输入形式 const vector<StepRecord>& records, const string& path 输入含义 每步记录列表、输出文件路径 输出形式 void 输出含义 无返回值，写入过程文本文件
    static void saveProcess(const vector<StepRecord>& records, const string& path);
};
