# The Last Train - Programming Fundamentals Project (Procedural / No Structs)

CS-1002: Programming Fundamentals (AI & DS)

Explore a five-floor building, rescue a passenger and a cat, and board the last train at F5-R2 before energy or time runs out. The standard game starts with 100 energy, 50 time, an empty inventory and 0 points. Your choice of starting room affects which routes you can finish in time.

## Build and run

### 1. Compile the game

Open a terminal in the folder containing this README. Use a C++17-capable `g++` compiler to build the game:

```sh
g++ -std=c++17 -Wall -Wextra -Iinclude src/core/*.cpp -o last_train
```

### 2. Launch from the same folder

The game reads its map from `data/` and writes saves and reports to `out/`.

| Terminal | Command |
|---|---|
| Windows PowerShell | `.\last_train.exe` |
| Linux | `./last_train` |

### 3. Play your first turn

1. Select **1. Start New Game**. Enter floor `1`, then room `2`, to start in **F1-R2**.
2. Choose **2. Inspect room** to see the Master Key. Enter `-1` to continue.
3. Choose **3. Collect item** and enter `<` at the symbol prompt. Enter `-1`, then choose **5. Show current state** to see the key in your inventory.

To resume later, choose **8. Save game** during play. At the main menu, choose **2. Load Saved Game**.

## Controls

From the main menu, enter **1** for a new game, **2** to load a save, **3** for instructions or **4** to exit. During a game, the numbers have the following meanings. Enter `-1` when prompted to continue after an action.

| Command | What to enter next | Effect |
|---|---|---|
| 1. Move | Destination floor, then room | Attempt one direct connection. |
| 2. Inspect room | No additional input | List the room contents and discover any clues. Each clue scores once. |
| 3. Collect item | Item symbol | Pick up one matching item. Carried items enter inventory. Consumables apply immediately. |
| 4. Use item / interact | Item symbol | Use equipment in its required room. At the cabinet, enter `^` and then the code when prompted. |
| 5. Show current state | No additional input | View location, energy, time, score, inventory and remaining emergency trips. |
| 6. Show map | No additional input | Display the building with the current room highlighted and remaining pickups visible. |
| 7. Show objectives | No additional input | See which milestones are complete. |
| 8. Save game | No additional input | Write the current checkpoint to `out/savegame.txt`. |
| 0. Return to main menu | No additional input | Leave this session. Use command 8 first to keep progress. |

## Movement and resources

Each successful move uses one connection. Energy and time are capped at 100. Rejected actions, viewing a screen and saving cost nothing. If either resource reaches zero before boarding is complete, the mission ends in defeat.

| Connection or action | Rule | Energy | Time |
|---|---|---:|---:|
| Door `D` or locked door `L` | Cross between adjacent rooms. The F3-R1/F3-R2 lock requires the Master Key. | -2 | -1 |
| Stairs `S1`, `S2`, `S3` | Use R1 at both ends. Floor pairs are 1/2, 2/3 and 4/5. | -2 | -1 |
| Window `W` | Cross between adjacent rooms. | -3 | -2 |
| Lift 1 `L1` | Travel between F1 and F2. Either end may be R2 or R3. | -4 | -3 |
| Lift 2 `L2` | Links R4 on F2, F4 and F5. Before repair, the card grants three trips. After repair, carry the Lift Pass. | -4 | -3 |
| Solid wall `#` | Blocks direct travel between F4-R2 and F4-R3. | 0 | 0 |
| Inspect, collect a carried item, use equipment or unlock the cabinet | Charged on an accepted action. | -1 | -1 |

## Items, puzzles and scoring

| Symbol | Found in | What it does | Points |
|---|---|---|---:|
| `<` Master Key | F1-R2 | Carry it to cross the locked door on F3. | 10 |
| `)` Energy Pack | F1-R4 | Two packs start in the room. Each pickup restores 25 energy and removes one pack. | 0 |
| `\` Emergency Toolkit | F2-R3 | Immediately restores 15 energy. | 0 |
| `4`, `2`, `7` Clues | F3-R2, F4-R3, F5-R1 respectively | Inspect each room to discover its digit. Repeated inspection still costs resources. | 10 each |
| `*` Fuse and `;` Control Panel | Fuse in F3-R2, panel in F3-R4 | Carry the fuse to the panel and use `*` to repair Lift 2. | 20 on installation |
| `(` Lift Pass | F3-R4 | Authorizes travel on the repaired Lift 2. | 20 |
| `$` Emergency Card | F4-R2 | Grants three Lift 2 trips before repair. Each successful emergency ride uses one. | 0 |
| `%` Medical Kit and `!` Passenger | Kit in F4-R2, passenger in F4-R1 | Use the kit in the passenger's room to complete the rescue. | 100 |
| `@` Cat Carrier and `~` Cat | Carrier in F5-R3, cat in F3-R3 | Use the carrier in the cat's room to secure the cat. | 100 |
| `&` Observation Token | F5-R4 | Immediately restores 15 time. | 0 |
| `^` Cabinet / Train Key | F5-R1 | After finding the three clues, unlock the cabinet, then collect the key. | 30 to unlock, 10 to collect |

**Boarding:** Use the Train Key at F5-R2 after completing the required objectives. Boarding earns 100 points and victory adds another 100. A full win from the standard initial state earns **520/520**. Different winning routes can finish with different energy and time.

## Program structure

| Source or file | Role |
|---|---|
| `main_core.cpp`, `level.cpp` | Allocate game state, parse the ASCII building and initial settings, and validate the starting room. |
| `simulator.cpp` | Run menus and the turn loop, dispatch actions, and decide victory or defeat. |
| `physics.cpp`, `switches.cpp` | Check direct connections, access requirements and puzzles. Manage inventory, resources and score. |
| `viewer_iface_core.cpp`, `util.cpp` | Render the dashboard, rooms, live map and checklist. Provide text and terminal helpers. |
| `include/config.h`, `include/state.h` | Define capacities, costs, bonuses, entity symbols and objective indices. |
| `data/building.txt`, `data/initial-state.txt` | Provide the original map and starting state. |
| `logger.cpp`, `out/` | Save and restore the player and remaining room contents. Write the final score, resources and objective checklist to `game_summary.txt`. |

The procedural C++ implementation uses fixed-size arrays. Each of the five floors has four rooms and three horizontal boundaries. Each room has up to five entity slots and a count of occupied slots. Inventory uses ten slots, and sixteen Boolean flags track progress. Functions share these arrays and update scalar values through references.

## Replay scenarios

| Input file in `data/` | Scenario | Outcome | Final energy | Final time |
|---|---|---|---:|---:|
| `win-best.txt` | Full mission | Victory, 520/520 | 22 | 2 |
| `win-extra-actions.txt` | Extra inspection plus a mid-game save and reload | Victory, 520/520 | 21 | 1 |
| `loss-energy.txt` | Energy exhausted while time remains | Defeat, 0/520 | 0 | 14 |
| `loss-time.txt` | Time exhausted while energy remains | Defeat, 0/520 | 50 | 0 |

## Verify deterministic outputs

Use `data/win-extra-actions.txt` to play a full mission, save, reload, and win. The replay replaces `out/savegame.txt` and `out/game_summary.txt`. Keep the same executable, map, and initial state for both runs.

**Windows PowerShell** — run twice and compare both SHA-256 hashes:

```powershell
$files = 'out/savegame.txt', 'out/game_summary.txt'
Get-Content data/win-extra-actions.txt | .\last_train.exe
$first = (Get-FileHash -Path $files -Algorithm SHA256).Hash
Get-Content data/win-extra-actions.txt | .\last_train.exe
$second = (Get-FileHash -Path $files -Algorithm SHA256).Hash
($first[0] -eq $second[0]) -and ($first[1] -eq $second[1])
```

**Linux** — record the first run's hashes and check them after the second run:

```sh
./last_train < data/win-extra-actions.txt
hashes=$(sha256sum out/savegame.txt out/game_summary.txt)
./last_train < data/win-extra-actions.txt
printf '%s\n' "$hashes" | sha256sum --check
```

PowerShell should print `True`; Linux should print `OK` for both files. Open `out/game_summary.txt` and confirm **VICTORY**, **520/520 points**, **21 energy**, and **1 time**. The save file records the mid-game checkpoint.
