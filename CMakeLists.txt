cmake_minimum_required(VERSION 3.10)

project(MazeAI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include visual)

add_executable(MazeAI
    main.cpp
    src/game_engine.cpp
    src/ai_player.cpp
    src/game_recorder.cpp
    src/json_loader.cpp
    algorithms/greedy.cpp
    algorithms/astar.cpp
    algorithms/dijkstra.cpp
    algorithms/bfs.cpp
    algorithms/divide.cpp
    algorithms/battle.cpp
    visual/visualizer.cpp
)

if(MSVC)
    target_compile_options(MazeAI PRIVATE /utf-8)
endif()
