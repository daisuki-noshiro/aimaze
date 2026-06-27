#pragma once

#include <string>

#include "model.h"

// JSON 读取结果：保存初始化 GameEngine 所需的四类数据。
struct LoadedGameData
{
    MapData mapData;
    GameConfig config;
    PlayerState player;
    BossState boss;
};

// JSON 读取器：负责把输入文件转换为游戏初始化数据。
class JsonLoader
{
public:
    // load（读取JSON输入）输入形式 const string& filePath 输入含义 JSON文件路径 输出形式 LoadedGameData 输出含义 初始化GameEngine所需的地图、配置、玩家和Boss数据
    static LoadedGameData load(const string& filePath);
};
