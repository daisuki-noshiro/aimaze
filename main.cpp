#include <iostream>
#include <string>

#include "ai_player.h"
#include "game_engine.h"
#include "game_recorder.h"
#include "json_loader.h"
#include "visualizer.h"

using namespace std;

// parseMode（解析探索模式）输入形式 const string& text 输入含义 命令行传入的模式字符串 输出形式 ExploreMode 输出含义 对应的探索算法枚举
ExploreMode parseMode(const string& text)
{
    if (text == "astar")
    {
        return ExploreMode::ASTAR;
    }
    if (text == "dijkstra")
    {
        return ExploreMode::DIJKSTRA;
    }
    if (text == "bfs")
    {
        return ExploreMode::BFS;
    }
    if (text == "divide")
    {
        return ExploreMode::DIVIDE;
    }
    return ExploreMode::DIVIDE;
}

// main（程序入口）输入形式 int argc, char* argv[] 输入含义 命令行参数数量和参数数组 输出形式 int 输出含义 0表示运行成功，1表示运行失败
int main(int argc, char* argv[])
{
    string inputPath = argc > 1 ? argv[1] : "data/maze_15_15.json";
    string modeText = argc > 2 ? argv[2] : "divide";

    try
    {
        LoadedGameData data = JsonLoader::load(inputPath);
        GameEngine engine(data.mapData, data.config, data.player, data.boss);
        AIPlayer player(parseMode(modeText));
        GameRecorder recorder;

        while (!engine.isGameOver())
        {
            GameState state = engine.getState();
            Action action = player.decide(state);
            StepRecord record = engine.step(action);
            recorder.record(record);
        }

        GameResult result = recorder.getResult();
        Visualizer::saveResult(result, "output/result.json");
        Visualizer::saveProcess(recorder.getRecords(), "output/process.txt");
        Visualizer::saveHtml(data.mapData, recorder.getRecords(), result, "output/visualization.html");
    }
    catch (const exception& ex)
    {
        cerr << "运行失败: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
