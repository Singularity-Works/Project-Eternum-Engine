# Eternum Engine

**Eternum Engine** is a C++ simulation framework for procedurally generated, game‑adjacent environments.
It focuses on **data structures, algorithms, and modular systems** inspired by game development, making it both a learning tool and a portfolio showcase.

---

## Features

* Procedural dungeon and world generation (BSP, cellular automata, graph‑based connections)
* Lightweight ECS (Entity Component System)
* AI pathfinding (BFS, A\*, Dijkstra)
* Turn‑based combat simulation
* Event system for traps, loot, and environmental changes
* Save/load system (JSON + binary)
* Unit tests using Google Test
* Cross‑platform build via CMake

---

## Quick Start

### Clone & Build

```bash
git clone https://github.com/Singularity-Works/eternum-engine.git
cd eternum-engine
mkdir build && cd build
cmake ..
make
```

### Run

```bash
./eternum-engine --mode interactive --seed 42
```

Modes:

* `interactive` – Player-controlled exploration
* `simulation` – AI vs AI, logs results
* `headless` – Runs without graphics for automated tests

---

## Example (Terminal Mode)

```
#############
#@....g.....#
#.....T.....#
#############
Turn: 1 | HP: 20/20 | Inventory: (empty)
Command? (WASD to move, Q to quit)
```

---

## License

This project is licensed under the [MIT License](LICENSE).

---

## Roadmap

### Current Goals

* Core architecture + ECS
* World generation algorithms (BSP, cellular automata, corridor graphs)
* Serialization/deserialization (JSON + binary)
* AI pathfinding (BFS, A\*, Dijkstra)
* Combat system with basic effects
* Event system for traps, loot, and environmental changes
* Unit test suite with >80% coverage
* CI/CD setup with GitHub Actions

### Stretch / Future Plans

* Visual debug renderer (SDL2/ImGui)
* Advanced AI (behavior trees)
* Multiplayer simulation mode
* Lua scripting for custom behaviors
* Procedural item generation
