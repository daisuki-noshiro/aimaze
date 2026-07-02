#include "json_loader.h"

#include <cctype>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace
{
// readFile（读取文件）输入形式 const string& filePath 输入含义 JSON文件路径 输出形式 string 输出含义 文件完整文本
string readFile(const string& filePath)
{
    ifstream input(filePath);
    if (!input)
    {
        throw runtime_error("Cannot open JSON file: " + filePath);
    }
    return string(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
}

// findValueStart（查找字段值起点）输入形式 const string& text, const string& key 输入含义 JSON文本、字段名 输出形式 size_t 输出含义 字段值第一个非空白字符下标
size_t findValueStart(const string& text, const string& key)
{
    string pattern = "\"" + key + "\"";
    size_t pos = text.find(pattern);
    if (pos == string::npos)
    {
        throw runtime_error("Missing JSON field: " + key);
    }
    pos = text.find(':', pos + pattern.size());
    if (pos == string::npos)
    {
        throw runtime_error("Invalid JSON field: " + key);
    }
    ++pos;
    while (pos < text.size() && isspace(static_cast<unsigned char>(text[pos])))
    {
        ++pos;
    }
    return pos;
}

// extractArray（提取数组字段）输入形式 const string& text, const string& key 输入含义 JSON文本、数组字段名 输出形式 string 输出含义 包含方括号的数组文本
string extractArray(const string& text, const string& key)
{
    size_t start = findValueStart(text, key);
    if (start >= text.size() || text[start] != '[')
    {
        throw runtime_error("JSON field is not array: " + key);
    }

    int depth = 0;
    bool inString = false;
    for (size_t i = start; i < text.size(); ++i)
    {
        char ch = text[i];
        if (ch == '"' && (i == 0 || text[i - 1] != '\\'))
        {
            inString = !inString;
        }
        if (inString)
        {
            continue;
        }
        if (ch == '[')
        {
            ++depth;
        }
        else if (ch == ']')
        {
            --depth;
            if (depth == 0)
            {
                return text.substr(start, i - start + 1);
            }
        }
    }
    throw runtime_error("Unclosed JSON array: " + key);
}

// extractInt（提取整数字段）输入形式 const string& text, const string& key 输入含义 JSON文本、整数字段名 输出形式 int 输出含义 字段对应的整数值
int extractInt(const string& text, const string& key)
{
    size_t pos = findValueStart(text, key);
    bool negative = false;
    if (text[pos] == '-')
    {
        negative = true;
        ++pos;
    }

    int value = 0;
    bool hasDigit = false;
    while (pos < text.size() && isdigit(static_cast<unsigned char>(text[pos])))
    {
        value = value * 10 + (text[pos] - '0');
        hasDigit = true;
        ++pos;
    }
    if (!hasDigit)
    {
        throw runtime_error("JSON field is not integer: " + key);
    }
    return negative ? -value : value;
}

bool hasField(const string& text, const string& key)
{
    return text.find("\"" + key + "\"") != string::npos;
}

// parseIntArray（解析整数数组）输入形式 const string& arrayText 输入含义 数组文本 输出形式 vector<int> 输出含义 数组中提取出的全部整数
vector<int> parseIntArray(const string& arrayText)
{
    vector<int> values;
    for (size_t i = 0; i < arrayText.size(); ++i)
    {
        if (!isdigit(static_cast<unsigned char>(arrayText[i])) && arrayText[i] != '-')
        {
            continue;
        }

        bool negative = false;
        if (arrayText[i] == '-')
        {
            negative = true;
            ++i;
        }

        int value = 0;
        while (i < arrayText.size() && isdigit(static_cast<unsigned char>(arrayText[i])))
        {
            value = value * 10 + (arrayText[i] - '0');
            ++i;
        }
        values.push_back(negative ? -value : value);
    }
    return values;
}

// parseString（解析JSON字符串）输入形式 const string& text, size_t& pos 输入含义 JSON文本、当前扫描位置 输出形式 string 输出含义 字符串内容并更新pos
string parseString(const string& text, size_t& pos)
{
    if (text[pos] != '"')
    {
        throw runtime_error("Invalid JSON string");
    }
    string value;
    ++pos;
    while (pos < text.size())
    {
        char ch = text[pos++];
        if (ch == '"')
        {
            return value;
        }
        if (ch == '\\' && pos < text.size())
        {
            value.push_back(text[pos++]);
        }
        else
        {
            value.push_back(ch);
        }
    }
    throw runtime_error("Unclosed JSON string");
}

// parseMaze（解析迷宫矩阵）输入形式 const string& arrayText 输入含义 maze字段数组文本 输出形式 vector<vector<char>> 输出含义 二维字符迷宫
vector<vector<char>> parseMaze(const string& arrayText)
{
    vector<vector<char>> maze;
    vector<char> row;
    int depth = 0;

    for (size_t i = 0; i < arrayText.size(); ++i)
    {
        char ch = arrayText[i];
        if (ch == '"')
        {
            string cell = parseString(arrayText, i);
            if (depth == 2)
            {
                row.push_back(cell.empty() ? ' ' : cell[0]);
            }
            --i;
            continue;
        }

        if (ch == '[')
        {
            ++depth;
            if (depth == 2)
            {
                row.clear();
            }
        }
        else if (ch == ']')
        {
            if (depth == 2)
            {
                maze.push_back(row);
            }
            --depth;
        }
    }
    return maze;
}

// fillPositions（扫描特殊坐标）输入形式 MapData& mapData 输入含义 待补充坐标的地图数据 输出形式 void 输出含义 无返回值，直接写入start、end、boss
void fillPositions(MapData& mapData)
{
    for (int i = 0; i < static_cast<int>(mapData.maze.size()); ++i)
    {
        for (int j = 0; j < static_cast<int>(mapData.maze[i].size()); ++j)
        {
            char cell = mapData.maze[i][j];
            if (cell == 'S')
            {
                mapData.start = {i, j};
            }
            else if (cell == 'E')
            {
                mapData.end = {i, j};
            }
            else if (cell == 'B')
            {
                mapData.boss = {i, j};
            }
        }
    }
}

vector<vector<char>> parseGridAsMaze(const string& arrayText)
{
    vector<vector<char>> maze = parseMaze(arrayText);
    for (vector<char>& row : maze)
    {
        for (char& cell : row)
        {
            if (cell == 'P')
            {
                cell = 'S';
            }
            else if (cell == '.')
            {
                cell = ' ';
            }
        }
    }
    return maze;
}
}

// load（读取JSON输入）输入形式 const string& filePath 输入含义 JSON文件路径 输出形式 LoadedGameData 输出含义 初始化GameEngine所需的地图、配置、玩家和Boss数据
LoadedGameData JsonLoader::load(const string& filePath)
{
    string text = readFile(filePath);

    LoadedGameData data;
    if (hasField(text, "grid") && !hasField(text, "maze"))
    {
        data.mapData.maze = parseGridAsMaze(extractArray(text, "grid"));
        fillPositions(data.mapData);
        data.config.finishOnNoAction = true;
        data.player.position = data.mapData.start;
        data.player.coins = 0;
        data.player.steps = 0;
        return data;
    }

    data.mapData.maze = parseMaze(extractArray(text, "maze"));
    fillPositions(data.mapData);

    data.boss.hpList = parseIntArray(extractArray(text, "B"));

    vector<int> skillValues = parseIntArray(extractArray(text, "PlayerSkills"));
    for (size_t i = 0; i + 1 < skillValues.size(); i += 2)
    {
        Skill skill;
        skill.damage = skillValues[i];
        skill.cooldown = skillValues[i + 1];
        skill.currentCD = 0;
        data.player.skills.push_back(skill);
    }

    data.config.minRounds = extractInt(text, "minRounds");
    data.config.coinConsumption = extractInt(text, "CoinConsumption");
    data.player.position = data.mapData.start;
    data.player.coins = 0;
    data.player.steps = 0;
    return data;
}
