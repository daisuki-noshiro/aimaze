# aimaze
算法课程设计
目标：
胜利条件：走出迷宫
失败条件：金币耗尽
## 项目整体结构

```text
MazeAI/
│
├── include/    
│   ├── model.h          //数据结构  
│   ├── game_engine.h    //裁判层头文件
│   ├── ai_player.h      //玩家层头文件
│   ├── game_recorder.h  //数据统计层头文件
│   ├── json_loader.h
│   └── algorithm.h
│
├── src/                 //各层对应实现
│   ├── game_engine.cpp
│   ├── ai_player.cpp
│   ├── game_recorder.cpp
│   └── json_loader.cpp
│
├── algorithms/  //算法部分，等待填充，将自己的算法填入进去
│   ├── greedy.cpp    
│   ├── astar.cpp
│   ├── dijkstra.cpp
│   ├── bfs.cpp
│   ├── divide.cpp
│   └── battle.cpp
│
├── visual/    //可视化部分，等待填充
│   ├── visualizer.h
│   └── visualizer.cpp
│
├── data/
├── output/
├── main.cpp
└── CMakeLists.txt
```


## 项目说明

GameEngine（裁判层）负责维护整个游戏，包括地图、玩家状态、Boss 状态、游戏规则以及胜负判断，是整个项目中唯一能够修改游戏状态的模块。AIPlayer（玩家层）只负责根据当前游戏状态进行决策，每回合只能获得当前位置的 3×3 视野，并利用 `map<Position, CellMemory>` 逐步构建自己的地图记忆，最终返回一个 `Action` 给 Engine。Algorithm（算法层）负责实现各种寻路算法和 Boss 战斗策略，不维护任何游戏状态，统一根据 `GameState` 和 `memory` 计算下一步动作，并返回 `Action`。GameRecorder（记录层）负责保存整个游戏过程，每回合记录一个 `StepRecord`，最终统计生成 `GameResult`，包括路径、动作、事件、步数、金币等信息。Visualizer（可视化层）只负责根据 Recorder 保存的数据进行 txt 输出或后续图形化展示，不参与任何游戏逻辑。
程序运行流程为：首先读取 JSON 初始化游戏，然后创建 GameEngine、AIPlayer 和 GameRecorder。游戏循环中，Engine 先返回当前 `GameState`，AIPlayer 根据当前状态调用对应算法得到 `Action`，Engine 执行动作并更新游戏状态，生成 `StepRecord`，Recorder 保存该记录。循环直到游戏结束，最后由 Recorder 统计 `GameResult`，Visualizer 根据统计结果完成输出。
各模块之间的数据交换固定为：Engine 向 AIPlayer 提供 `GameState`，AIPlayer 向 Engine 返回 `Action`，Engine 向 Recorder 返回 `StepRecord`，Recorder 向 Visualizer 提供 `GameResult` 和完整记录。任何模块均不能直接修改其它模块的数据，所有游戏状态的更新必须经过 GameEngine。

算法接口统一设计为：
Action Algorithm::XXX(const GameState& state, map<Position, CellMemory>& memory);

Boss 战统一使用：
Action Algorithm::Battle(const GameState& state);
所有算法均只负责返回 `Action`，不修改地图、玩家或 Boss 状态。
测试不同寻路算法时，只需在 `main.cpp` 创建 AIPlayer 时指定探索模式，例如：

AIPlayer player(ExploreMode::GREEDY);
AIPlayer player(ExploreMode::ASTAR);
AIPlayer player(ExploreMode::DIJKSTRA);
整个游戏运行过程中不会切换算法，如需测试其它算法，只需重新创建 AIPlayer 并修改对应的 `ExploreMode` 即可。
