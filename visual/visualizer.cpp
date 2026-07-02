#include "visualizer.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>

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

string cellClass(char cell)
{
    if (cell == '#')
    {
        return "wall";
    }
    if (cell == 'S')
    {
        return "start";
    }
    if (cell == 'E')
    {
        return "end";
    }
    if (cell == 'G')
    {
        return "gold";
    }
    if (cell == 'T')
    {
        return "trap";
    }
    if (cell == 'B')
    {
        return "boss";
    }
    return "road";
}

string cellText(char cell)
{
    return cell == ' ' ? "" : string(1, cell);
}

void writeJsString(ofstream& out, const string& value)
{
    out << "\"";
    for (char ch : value)
    {
        if (ch == '\\')
        {
            out << "\\\\";
        }
        else if (ch == '"')
        {
            out << "\\\"";
        }
        else if (ch == '\n')
        {
            out << "\\n";
        }
        else
        {
            out << ch;
        }
    }
    out << "\"";
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

void Visualizer::saveHtml(const MapData& mapData,
                          const vector<StepRecord>& records,
                          const GameResult& result,
                          const string& path)
{
    ensureParentDirectory(path);
    ofstream out(path);

    int columnCount = 0;
    for (const vector<char>& row : mapData.maze)
    {
        columnCount = max(columnCount, static_cast<int>(row.size()));
    }

    PlayerState initialPlayer;
    if (!records.empty())
    {
        initialPlayer = records.front().before.player;
    }
    else
    {
        initialPlayer.position = mapData.start;
    }

    out << "<!doctype html>\n";
    out << "<html lang=\"en\">\n";
    out << "<head>\n";
    out << "  <meta charset=\"utf-8\">\n";
    out << "  <title>Maze Visualization</title>\n";
    out << "  <style>\n";
    out << "    body { margin: 0; font-family: Arial, sans-serif; background: #f6f7fb; color: #1f2937; }\n";
    out << "    .page { max-width: 1180px; margin: 28px auto; padding: 0 24px; }\n";
    out << "    h1 { margin: 0 0 18px; font-size: 28px; }\n";
    out << "    .runner { display: grid; grid-template-columns: minmax(160px, 1fr) 150px 110px minmax(160px, 1.4fr); gap: 10px; align-items: center; margin: 0 0 18px; }\n";
    out << "    select { height: 36px; border: 1px solid #cbd5e1; background: #ffffff; color: #0f172a; padding: 0 10px; font-weight: 700; }\n";
    out << "    .run-status { color: #475569; font-size: 14px; min-height: 20px; }\n";
    out << "    .layout { display: flex; align-items: flex-start; gap: 24px; }\n";
    out << "    .maze { display: grid; gap: 3px; width: max-content; padding: 16px; background: #ffffff; border: 1px solid #d7dce5; }\n";
    out << "    .cell { width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; font-weight: 700; font-size: 14px; box-sizing: border-box; }\n";
    out << "    .wall { background: #111827; color: #111827; }\n";
    out << "    .road { background: #ffffff; border: 1px solid #e5e7eb; }\n";
    out << "    .start { background: #2563eb; color: #ffffff; }\n";
    out << "    .end { background: #16a34a; color: #ffffff; }\n";
    out << "    .gold { background: #facc15; color: #713f12; }\n";
    out << "    .trap { background: #ef4444; color: #ffffff; }\n";
    out << "    .boss { background: #7c3aed; color: #ffffff; }\n";
    out << "    .path { box-shadow: inset 0 0 0 4px #38bdf8; }\n";
    out << "    .road.path { background: #bae6fd; }\n";
    out << "    .current { outline: 4px solid #f97316; outline-offset: -4px; }\n";
    out << "    .panel { width: 300px; padding: 16px; background: #ffffff; border: 1px solid #d7dce5; }\n";
    out << "    .stats { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 14px; }\n";
    out << "    .stat { padding: 10px; background: #f8fafc; border: 1px solid #e5e7eb; }\n";
    out << "    .label { display: block; margin-bottom: 4px; color: #64748b; font-size: 12px; }\n";
    out << "    .value { font-size: 18px; font-weight: 700; }\n";
    out << "    .controls { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 8px; margin: 12px 0; }\n";
    out << "    button { height: 34px; border: 1px solid #cbd5e1; background: #ffffff; color: #0f172a; font-weight: 700; cursor: pointer; }\n";
    out << "    button:hover { background: #f1f5f9; }\n";
    out << "    input[type=range] { width: 100%; }\n";
    out << "    .line { margin: 10px 0; font-size: 14px; }\n";
    out << "    .events { min-height: 44px; padding: 10px; background: #f8fafc; border: 1px solid #e5e7eb; font-size: 13px; line-height: 1.5; }\n";
    out << "    .legend { display: flex; flex-wrap: wrap; gap: 10px 18px; margin-top: 18px; }\n";
    out << "    .legend-item { display: inline-flex; align-items: center; gap: 6px; font-size: 14px; }\n";
    out << "    .legend-swatch { width: 18px; height: 18px; border: 1px solid #d1d5db; box-sizing: border-box; }\n";
    out << "  </style>\n";
    out << "</head>\n";
    out << "<body>\n";
    out << "  <main class=\"page\">\n";
    out << "    <h1>Maze Visualization</h1>\n";
    out << "    <div class=\"runner\">\n";
    out << "      <select id=\"mapSelect\"></select>\n";
    out << "      <select id=\"algorithmSelect\">\n";
    out << "        <option value=\"greedy\">greedy</option>\n";
    out << "        <option value=\"divide\">divide</option>\n";
    out << "        <option value=\"astar\">astar</option>\n";
    out << "        <option value=\"dijkstra\">dijkstra</option>\n";
    out << "        <option value=\"bfs\">bfs</option>\n";
    out << "      </select>\n";
    out << "      <button id=\"runBtn\">Run</button>\n";
    out << "      <span id=\"runStatus\" class=\"run-status\"></span>\n";
    out << "    </div>\n";
    out << "    <div class=\"layout\">\n";
    out << "      <section>\n";
    out << "        <div id=\"maze\" class=\"maze\"></div>\n";
    out << "        <div class=\"legend\">\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch start\"></span>Start</span>\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch end\"></span>End</span>\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch wall\"></span>Wall</span>\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch road\"></span>Road</span>\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch gold\"></span>Gold</span>\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch trap\"></span>Trap</span>\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch boss\"></span>Boss</span>\n";
    out << "          <span class=\"legend-item\"><span class=\"legend-swatch road path\"></span>Path</span>\n";
    out << "        </div>\n";
    out << "      </section>\n";
    out << "      <aside class=\"panel\">\n";
    out << "        <div class=\"stats\">\n";
    out << "          <div class=\"stat\"><span class=\"label\">Step</span><span id=\"stepValue\" class=\"value\">0</span></div>\n";
    out << "          <div class=\"stat\"><span class=\"label\">Coins</span><span id=\"coinValue\" class=\"value\">0</span></div>\n";
    out << "          <div class=\"stat\"><span class=\"label\">Position</span><span id=\"positionValue\" class=\"value\">(0,0)</span></div>\n";
    out << "          <div class=\"stat\"><span class=\"label\">Status</span><span class=\"value\">" << statusToString(result.status) << "</span></div>\n";
    out << "        </div>\n";
    out << "        <input id=\"stepRange\" type=\"range\" min=\"0\" value=\"0\">\n";
    out << "        <div class=\"controls\">\n";
    out << "          <button id=\"prevBtn\">Prev</button>\n";
    out << "          <button id=\"playBtn\">Play</button>\n";
    out << "          <button id=\"nextBtn\">Next</button>\n";
    out << "        </div>\n";
    out << "        <div class=\"line\"><strong>Action:</strong> <span id=\"actionValue\">START</span></div>\n";
    out << "        <div class=\"line\"><strong>Events:</strong></div>\n";
    out << "        <div id=\"eventsValue\" class=\"events\">START</div>\n";
    out << "        <div class=\"line\"><strong>Final:</strong> coins " << result.coins
        << ", steps " << result.steps << ", ratio " << result.ratio << "</div>\n";
    out << "      </aside>\n";
    out << "    </div>\n";
    out << "  </main>\n";
    out << "  <script>\n";
    out << "    const columnCount = " << columnCount << ";\n";
    out << "    const maze = [\n";
    for (size_t rowIndex = 0; rowIndex < mapData.maze.size(); ++rowIndex)
    {
        if (rowIndex > 0)
        {
            out << ",\n";
        }
        out << "      [";
        for (int column = 0; column < columnCount; ++column)
        {
            if (column > 0)
            {
                out << ",";
            }
            char cell = column < static_cast<int>(mapData.maze[rowIndex].size())
                ? mapData.maze[rowIndex][column]
                : '#';
            writeJsString(out, string(1, cell));
        }
        out << "]";
    }
    out << "\n    ];\n";
    out << "    const steps = [\n";
    out << "      {step:0, position:[" << initialPlayer.position.x << "," << initialPlayer.position.y
        << "], coins:" << initialPlayer.coins << ", action:\"START\", events:[\"START\"]}";
    for (size_t i = 0; i < records.size(); ++i)
    {
        const StepRecord& record = records[i];
        out << ",\n      {step:" << i + 1
            << ", position:[" << record.after.player.position.x << "," << record.after.player.position.y << "]"
            << ", coins:" << record.after.player.coins
            << ", action:";
        writeJsString(out, actionToString(record.action));
        out << ", events:[";
        for (size_t eventIndex = 0; eventIndex < record.events.size(); ++eventIndex)
        {
            if (eventIndex > 0)
            {
                out << ",";
            }
            writeJsString(out, eventToString(record.events[eventIndex].type));
        }
        out << "]}";
    }
    out << "\n    ];\n";
    out << "    let currentIndex = 0;\n";
    out << "    let timer = null;\n";
    out << "    const mazeEl = document.getElementById('maze');\n";
    out << "    const rangeEl = document.getElementById('stepRange');\n";
    out << "    const playBtn = document.getElementById('playBtn');\n";
    out << "    const mapSelect = document.getElementById('mapSelect');\n";
    out << "    const algorithmSelect = document.getElementById('algorithmSelect');\n";
    out << "    const runBtn = document.getElementById('runBtn');\n";
    out << "    const runStatus = document.getElementById('runStatus');\n";
    out << "    const selectionStorageKey = 'maze-visualizer-selection';\n";
    out << "    function classForCell(cell) {\n";
    out << "      if (cell === '#') return 'wall';\n";
    out << "      if (cell === 'S') return 'start';\n";
    out << "      if (cell === 'E') return 'end';\n";
    out << "      if (cell === 'G') return 'gold';\n";
    out << "      if (cell === 'T') return 'trap';\n";
    out << "      if (cell === 'B') return 'boss';\n";
    out << "      return 'road';\n";
    out << "    }\n";
    out << "    function textForCell(cell) { return cell === ' ' ? '' : cell; }\n";
    out << "    function key(position) { return position[0] + ',' + position[1]; }\n";
    out << "    function stopPlayback() {\n";
    out << "      if (timer !== null) { clearInterval(timer); timer = null; }\n";
    out << "      playBtn.textContent = 'Play';\n";
    out << "    }\n";
    out << "    function render() {\n";
    out << "      const step = steps[currentIndex];\n";
    out << "      const visited = new Set();\n";
    out << "      for (let i = 0; i <= currentIndex; ++i) visited.add(key(steps[i].position));\n";
    out << "      mazeEl.innerHTML = '';\n";
    out << "      mazeEl.style.gridTemplateColumns = `repeat(${columnCount}, 32px)`;\n";
    out << "      for (let row = 0; row < maze.length; ++row) {\n";
    out << "        for (let column = 0; column < columnCount; ++column) {\n";
    out << "          const cell = maze[row][column] || '#';\n";
    out << "          const pos = [row, column];\n";
    out << "          const div = document.createElement('div');\n";
    out << "          div.className = 'cell ' + classForCell(cell)\n";
    out << "            + (visited.has(key(pos)) ? ' path' : '')\n";
    out << "            + (key(pos) === key(step.position) ? ' current' : '');\n";
    out << "          div.textContent = textForCell(cell);\n";
    out << "          mazeEl.appendChild(div);\n";
    out << "        }\n";
    out << "      }\n";
    out << "      document.getElementById('stepValue').textContent = step.step;\n";
    out << "      document.getElementById('coinValue').textContent = step.coins;\n";
    out << "      document.getElementById('positionValue').textContent = '(' + step.position[0] + ',' + step.position[1] + ')';\n";
    out << "      document.getElementById('actionValue').textContent = step.action;\n";
    out << "      document.getElementById('eventsValue').textContent = step.events.length ? step.events.join(', ') : 'NONE';\n";
    out << "      rangeEl.value = currentIndex;\n";
    out << "    }\n";
    out << "    function goTo(index) {\n";
    out << "      currentIndex = Math.max(0, Math.min(index, steps.length - 1));\n";
    out << "      render();\n";
    out << "    }\n";
    out << "    function savedSelection() {\n";
    out << "      try { return JSON.parse(localStorage.getItem(selectionStorageKey)) || {}; }\n";
    out << "      catch (_) { return {}; }\n";
    out << "    }\n";
    out << "    function saveSelection() {\n";
    out << "      localStorage.setItem(selectionStorageKey, JSON.stringify({\n";
    out << "        map: mapSelect.value,\n";
    out << "        algorithm: algorithmSelect.value\n";
    out << "      }));\n";
    out << "    }\n";
    out << "    function restoreAlgorithm() {\n";
    out << "      const saved = savedSelection();\n";
    out << "      if (saved.algorithm) algorithmSelect.value = saved.algorithm;\n";
    out << "    }\n";
    out << "    async function loadMaps() {\n";
    out << "      try {\n";
    out << "        const response = await fetch('/api/maps');\n";
    out << "        if (!response.ok) return;\n";
    out << "        const data = await response.json();\n";
    out << "        const saved = savedSelection();\n";
    out << "        mapSelect.innerHTML = '';\n";
    out << "        for (const map of data.maps) {\n";
    out << "          const option = document.createElement('option');\n";
    out << "          option.value = map;\n";
    out << "          option.textContent = map;\n";
    out << "          mapSelect.appendChild(option);\n";
    out << "        }\n";
    out << "        if (saved.map && data.maps.includes(saved.map)) mapSelect.value = saved.map;\n";
    out << "      } catch (_) {}\n";
    out << "    }\n";
    out << "    async function runSelected() {\n";
    out << "      stopPlayback();\n";
    out << "      saveSelection();\n";
    out << "      runBtn.disabled = true;\n";
    out << "      runStatus.textContent = 'Running...';\n";
    out << "      try {\n";
    out << "        const response = await fetch('/api/run', {\n";
    out << "          method: 'POST',\n";
    out << "          headers: {'Content-Type': 'application/json'},\n";
    out << "          body: JSON.stringify({map: mapSelect.value, algorithm: algorithmSelect.value})\n";
    out << "        });\n";
    out << "        const data = await response.json();\n";
    out << "        if (!response.ok || !data.ok) throw new Error(data.error || 'Run failed');\n";
    out << "        runStatus.textContent = 'Done';\n";
    out << "        window.location.reload();\n";
    out << "      } catch (error) {\n";
    out << "        runStatus.textContent = error.message;\n";
    out << "        runBtn.disabled = false;\n";
    out << "      }\n";
    out << "    }\n";
    out << "    document.getElementById('prevBtn').addEventListener('click', () => { stopPlayback(); goTo(currentIndex - 1); });\n";
    out << "    document.getElementById('nextBtn').addEventListener('click', () => { stopPlayback(); goTo(currentIndex + 1); });\n";
    out << "    mapSelect.addEventListener('change', saveSelection);\n";
    out << "    algorithmSelect.addEventListener('change', saveSelection);\n";
    out << "    runBtn.addEventListener('click', runSelected);\n";
    out << "    playBtn.addEventListener('click', () => {\n";
    out << "      if (timer !== null) { stopPlayback(); return; }\n";
    out << "      playBtn.textContent = 'Pause';\n";
    out << "      timer = setInterval(() => {\n";
    out << "        if (currentIndex >= steps.length - 1) { stopPlayback(); return; }\n";
    out << "        goTo(currentIndex + 1);\n";
    out << "      }, 220);\n";
    out << "    });\n";
    out << "    rangeEl.max = steps.length - 1;\n";
    out << "    rangeEl.addEventListener('input', () => { stopPlayback(); goTo(Number(rangeEl.value)); });\n";
    out << "    restoreAlgorithm();\n";
    out << "    loadMaps();\n";
    out << "    render();\n";
    out << "  </script>\n";
    out << "</body>\n";
    out << "</html>\n";
}
