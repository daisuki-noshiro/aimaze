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
