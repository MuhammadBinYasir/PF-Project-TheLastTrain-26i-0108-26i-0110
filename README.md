# The Last Train - Programming Fundamentals Project

CS-1002: Programming Fundamentals (AI & DS)

A deterministic, text-based rescue game in C++. The player explores a 5-floor, 20-room building to rescue an injured passenger and a stranded cat, find the clue digits for the security cabinet, repair Lift 2, and board the last train before Energy or Time runs out.

## Architecture Overview
This project follows procedural programming concepts:
- **No Structs / No OOP**: All variables, game statuses, inventory, and building rooms are stored in primitive types (`int`, `char`, `bool`) and multi-dimensional parallel arrays.
- `include/`: Modular C++ header definitions (`config.h`, `state.h`, `level.h`, `simulator.h`, `physics.h`, `switches.h`, `logger.h`, `util.h`, `viewer_iface.h`).
- `src/core/`: Modular implementation files (`main_core.cpp`, `level.cpp`, `simulator.cpp`, `physics.cpp`, `switches.cpp`, `logger.cpp`, `util.cpp`, `viewer_iface_core.cpp`).
- `data/`: Building map (`building.txt`), initial state (`initial-state.txt`), and input test files.
- `out/`: Runtime save state (`savegame.txt`) and session summary report (`game_summary.txt`).
- `docs/`: Project documentation (`Flowchart.png`).

## Requirements
- Windows with g++ supporting C++17 (MinGW-w64, GCC 7 or newer).

## 1. Build

Open Command Prompt (cmd) in the project root folder (the folder that contains `include/` and `src/`) and run:

```bat
g++ -std=c++17 -Wall -Wextra -Iinclude src/core/*.cpp -o last_train
```

The build must finish with no errors and no warnings. It produces `last_train.exe`.

## 2. Run the Simulation

Always run the program from the project root, because it uses relative paths (`data/building.txt`, `data/initial-state.txt`, `out/savegame.txt`, `out/game_summary.txt`).

```bat
last_train.exe
```

Step by step:
1. The splash screen loads, then the main menu appears.
2. Enter `1` to start a new game (or `2` to load `out/savegame.txt`, `3` for instructions, `4` to exit).
3. Enter a starting floor (1-5) and room (1-4).
4. In the game menu, enter an action number:
   - `1` Move: enter the destination floor and room (one connection per move).
   - `2` Inspect the room (discovers clue digits).
   - `3` Collect an item by its symbol, e.g. `<` for the Master Key.
   - `4` Use an item by its symbol, e.g. `%` at F4-R1. At F5-R1, use `^` and type the cabinet code.
   - `5` Show status, `6` Show map, `7` Show objectives, `8` Save game, `0` Return to main menu.
5. After each action, enter `-1` to continue.
6. When you win or lose, the result screen is shown and `out/game_summary.txt` is written.

## 3. Run the Test Input Files

Each file in `data/` is a full command sequence. Feed it to the program with input redirection.

Command Prompt (cmd):

```bat
last_train.exe < data\win-best.txt
```

PowerShell (which does not support `<`):

```powershell
Get-Content data\win-best.txt | .\last_train.exe
```

Expected results in `out/game_summary.txt`:

| Input file                    | Outcome | Final score | Energy left | Time left |
|-------------------------------|---------|-------------|-------------|-----------|
| `data/win-best.txt`           | VICTORY | 520 / 520   | 22          | 2         |
| `data/win-extra-actions.txt`  | VICTORY | 520 / 520   | 21          | 1         |
| `data/loss-energy.txt`        | DEFEAT  | 0 / 520     | 0           | 14        |
| `data/loss-time.txt`          | DEFEAT  | 0 / 520     | 50          | 0         |

`win-extra-actions.txt` also saves the game, returns to the main menu, and loads the save again, so it tests pause/resume and writes `out/savegame.txt`.

## 4. Verify Deterministic Output

The game uses no randomness, so the same input must always give the same output files. Run the same input file twice, keep a copy of the outputs after each run, and compare them.

Command Prompt (cmd):

```bat
last_train.exe < data\win-best.txt > run1_console.txt
copy /Y out\game_summary.txt run1_summary.txt

last_train.exe < data\win-best.txt > run2_console.txt
copy /Y out\game_summary.txt run2_summary.txt

fc /b run1_summary.txt run2_summary.txt
fc /b run1_console.txt run2_console.txt
```

PowerShell (including the VS Code terminal). PowerShell has no `<` operator and `fc` means a different command there, so use `cmd /c` to run the game and `fc.exe` to compare:

```powershell
cmd /c "last_train.exe < data\win-best.txt > run1_console.txt"
Copy-Item out\game_summary.txt run1_summary.txt -Force

cmd /c "last_train.exe < data\win-best.txt > run2_console.txt"
Copy-Item out\game_summary.txt run2_summary.txt -Force

fc.exe /b run1_summary.txt run2_summary.txt
fc.exe /b run1_console.txt run2_console.txt
```

Each comparison prints "FC: no differences encountered" when the files match. Repeat with the other files in `data\` to check every scenario.

To check save/resume, run `data\win-extra-actions.txt` twice in the same way, and after each run also copy `out\savegame.txt` (for example to `run1_save.txt` and `run2_save.txt`) and compare those with `fc /b`.

## Troubleshooting
- **"Could not load data/building.txt"**: run `last_train.exe` from the project root, not from inside `src\` or another folder.
