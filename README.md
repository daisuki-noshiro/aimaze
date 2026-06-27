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
│   ├── model.h
│   ├── game_engine.h
│   ├── ai_player.h
│   ├── game_recorder.h
│   ├── json_loader.h
│   └── algorithm.h
│
├── src/
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
