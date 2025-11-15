# LyraNPC – Living NPC AI Framework (Concept Overview)

**LyraNPC** is a plugin for Unreal Engine 5 that enables fully living NPCs.  
It supports enemies, villagers, workers, guards, travelers, and neutral characters.  
Every NPC has their own life, their own intelligence level, and reacts logically to the world – without heavy system load.

---

## Core Idea

NPCs should feel like real individuals.  
They work, sleep, eat, interact, make mistakes, take breaks, and make decisions that fit their personality and routine.

---

## Intelligence & Dumbness Factor

Each NPC has a configurable **CognitiveSkill level** (0–1):

- affects perception: precise vs. sloppy  
- affects decision-making: smart vs. chaotic  
- affects movement: clean vs. clumsy  
- affects combat: accurate vs. panicked  
- affects daily life: follows plans vs. forgets tasks

This gives you fine-grained control over difficulty and world believability.

---

## NPCs With Their Own Life

Every NPC has:

- a biography (name, job, origin, personality)  
- a daily schedule (sleep, work, eat, leisure)  
- needs (hunger, energy, social needs)  
- relationships (friends, enemies, family)  
- village integration (home, workplace, favorite places)

NPCs follow day/night rhythms and make your world feel alive.

---

## Task & Interaction System

NPCs interact with **Task Actors**, for example:

- beds  
- workbenches  
- chairs and benches  
- viewpoints  
- market stands  
- forges  
- cooking spots  
- etc.

Each task defines:

- how many NPCs may use it at once  
- which roles are allowed  
- which animation is played  
- how long the NPC stays  
- whether the task is private (e.g. a personal bed)

All tasks are fully configurable in the editor.

---

## Behavior Trees & EQS Integration

LyraNPC integrates cleanly with UE5’s AI workflow:

- NPCs find task locations through EQS  
- BT flow: Find Task → Reserve → Move → Use → Exit  
- EQS considers role, needs, daytime, and intelligence  
- Ready-to-use BT modules for villagers and guards  
- Designers can extend everything without touching C++  
- Ideal combination of utility AI + Behavior Trees

NPCs behave logically while designers stay in full control.

---

## Optimisation & Performance

This plugin is built to handle **large numbers of NPCs simultaneously**:

- **AI LOD System**  
  - Close range = full AI  
  - Mid range = reduced perception + simplified decision logic  
  - Far range = schedule-only or lightweight simulation  

- **Distance-Based Update Rates**  
  - Vision, needs, and EQS updates run less often when far away

- **Task Query Optimiser**  
  - Task pools instead of expensive world scans  
  - Subsystem lookups instead of searching the entire level

- **Async Navigation & Perception**  
  - Pathfinding and traces run on async workers  
  - Multiple NPCs share query batches

- **Memory Modulation by CognitiveSkill**  
  - Dumb NPCs produce simpler data  
  - Smart NPCs run full logic only when relevant

- **Optimised Multiplayer Replication**  
  - Only intent, task, and state are replicated  
  - Compressed task usage  
  - No tick-based replication

Supports **100–300 NPCs at once** without performance collapse.

---

## Multiplayer Ready (Server-Authoritative)

- Fully server-side AI logic  
- Compact replication (intent/task/state only)  
- Correct task usage replication across clients  
- AI LOD reduces network load  
- Compatible with dedicated and listen servers

NPCs behave identically and predictably in all network setups.

---

## World Feel

NPCs created with this plugin do not feel scripted; they feel **alive**:

- they make mistakes  
- they can be dumb or smart  
- they work, sit, sleep, eat  
- they react to danger  
- they talk or socialize  
- they use furniture and world objects  
- they follow routines  
- they move believably through the world

This plugin enables complete villages and towns with minimal design effort.

---

## Who Is This Plugin For?

For any game that needs **atmosphere, world life, and believable NPCs**:

- Open-world  
- Survival  
- RPG  
- Adventure  
- Simulation  
- Village-building  
- Sandbox games

LyraNPC is not just an AI framework —  
it is the **foundation for living, breathing worlds**.
