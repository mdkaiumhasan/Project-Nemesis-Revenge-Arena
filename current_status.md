# Project Nemesis - Development Status Tracker 🎮🔥

This file serves as the source of truth for the project's current state, implemented features, and pending tasks. Any starting agent should read this file first.

---

## 🧠 0. Agent Working Guidelines (CRITICAL MEMORY)

*   **No Choices/Options:** Never present the user with multiple choices or ask them to select a path. The agent MUST select the single best technical option itself, explain the rationale, and proceed.
*   **Single Step-by-Step Guidance:** Always guide the user one single step at a time. Wait for the user to complete and confirm the current step before explaining the next. Do not group multiple interactive tasks together.

---

## 📖 1. The Vision: The "Game Developer Doctor" Paradigm

Project Nemesis is a high-intensity, psychological 1v1 Battle Royale / Arena game developed under the **3D Game Developer Doctor** concept—blending human psychology, neuroscience, and adrenaline loops with high-performance C++ and Unreal Engine 5.7 technology.

*   **Story & Lore (The Revenge Motif):** 
    *   Set in a dystopian cyberpunk world where corporations manipulate human memories.
    *   The protagonist is betrayed by their elite special-force partner, who wipes their memory and forces them into a virtual **Revenge Arena**.
    *   The game is a 1v1 combat trial over 5 rounds (Best of 5).
    *   Winning rounds unlocks memory flashbacks (glitches showing the player's past), leading to the final confrontation in the "Memory Core".
*   **Adrenaline & Psychological Mechanics:**
    *   *Fight-or-Flight Response:* Audio cues (heartbeats, heavy breathing) track stress levels.
    *   *Comeback Buff:* Players on a losing streak get passive tracking buffs to prevent one-sided matches.
    *   *Diegetic HUD:* No traditional floating health bars. Player stats (ECG health line, coins, round dots) are projected on a holographic **wristband/wrist-display** on the player's arm.
*   **Technical Stack:**
    *   Unreal Engine 5.7 Custom Build.
    *   High-tick network code (Client-Side Prediction, Lag Compensation, C++ RPCs).
    *   Hybrid architecture: C++ for core attributes, weapons, and physics; Blueprints for VFX, UI, and animation.

---

## 🛠️ 2. Roadmap Alignment & Current Implementation Status

### Phase 1: Core Character & Movement (Status: [/] Partially Implemented)
*   [/] **Enhanced & Hybrid Input Mapping:** Added dynamic fallback loading in C++ for `IMC_Default`, `IMC_MouseLook`, `IA_Move`, `IA_MouseLook`, `IA_Look`, and `DefaultWeaponClass` to handle missing editor references and blueprint serialization overrides. Reconfigured mapping context injection to occur both in `BeginPlay()` and `SetupPlayerInputComponent()` (possession-time) to prevent client-side desyncs. Implemented direct legacy key fallback bindings for mouse look axis, and added a direct hardware key polling system in `Tick()` using `WasInputKeyJustPressed()` for firing (`LeftMouseButton`/`RightMouseButton`) and reloading (`R`) to bypass Enhanced Input action intercepts and guarantee weapon mechanics work immediately without editor asset dependencies.
*   [x] **C++ Movement Physics:** Acceleration configurations, spring-arm camera damping/lag, and momentum-based speed transitions in `ANemesisCharacter`.
*   [x] **Sprint & Stamina Loop:** Frame-independent stamina drainage and regeneration logic mapped to movement states.
*   [x] **Camera Collision Glitch Fix:** Configured character capsule, main skeletal mesh, modular meshes, decoy components, and weapon skeletal/static meshes to ignore the `ECC_Camera` collision channel, eliminating camera jitter and sudden zoom-in/out glitches.

### Phase 2: Character States & Attribute System (Status: [x] Implemented)
*   [x] **Custom Attribute Component:** `UNemesisAttributeComponent` manages Health, Stamina, and Coins (with delegates for UI binding).
*   [x] **Damage Override:** Overrode `TakeDamage()` in `ANemesisCharacter` to process hit damage through attributes.
*   [x] **Death States:** Implemented ragdoll simulation, input locking, capsule collision disabling, and notifying `ANemesisGameMode` of death.

### Phase 3: Combat & Interaction (Status: [/] Partially Implemented)
*   [x] **Weapon Attachment & Socket Alignment:** Authoritative spawning and skeletal/static mesh socket binding via C++. Reconfigured `ANemesisWeapon` with a scene root to support both Static and Skeletal meshes. Implemented fixes for client-side replication timing, scale override issues (using `SnapToTargetNotIncludingScale`), and verified weapon attachment at runtime when mesh changes. Configured the default attach socket name to `hand_r` in C++ to align with the modular character skeleton.
*   [x] **Precision Hit Detection & Visual/Screen Feedback:** Server-side line-trace single hit detection (`LineTraceSingleByChannel`) inside `ANemesisWeapon`. Implemented visual debug tracer lines (`DrawDebugLine` in red) and impact spheres (`DrawDebugSphere` in red) to show bullet trajectories instantly on screen. Added real-time screen text feedback (`GEngine->AddOnScreenDebugMessage`) for ammo count and reload state, and resolved a double-firing bug. Programmed C++ properties for `FireSound`, `MuzzleFlash`, and `ImpactEffect`, loading AAA Paragon particle assets (`P_BelicaMuzzle` and `P_BelicaHitWorld`) and a default high-quality Lt. Belica gunshot cue (`LtBelica_Ability_LMB_Engage`) in the constructor. Implemented a dynamic runtime fallback in C++ to load and play this sound cue if the Blueprint's `FireSound` property is null. Configured these cosmetic effects (sound, particle emitters, tracers) to replicate to all clients using a NetMulticast RPC (`MulticastPlayFireEffects`).
*   [x] **Weapon Blueprints & Stats Configuration:** Created and configured all 13 weapons (USP, Deagle, M4A1S, AK47, AWP, MP5, MP40, Thompson, Model 12, Sawn-Off, Yalguzag, Knife, Katana) as Blueprints inheriting from `ANemesisWeapon` with appropriate static/skeletal meshes, custom stats, and material masking (`M_Invisible`).
*   [x] **Replicated Modular Customization:** Added `FModularCharacterParts` struct and `CharacterParts` replicated property to C++ `ANemesisCharacter` to support dynamic mesh swapping and animation syncing via `LeaderPoseComponent` for both modular and single-mesh outfits.
*   [x] **Montage System:** Implemented character weapon firing animation montages in C++. Configured the `FireMontage` variable in `ANemesisCharacter` with a dynamic C++ fallback to automatically load and play `/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage` for the standard pistol shoot animation when firing locally (snappy client feedback) and synchronizing it on non-local clients via the weapon's NetMulticast RPC. Created a public `PlayFireMontage()` function to wrap montage playing cleanly.
*   [ ] **Interface-Driven Interaction:** Global `IInteractionInterface` for world interactions ("Press E" triggers) has not yet been started.

### Phase 4: Enemy AI & Encounter Design (Status: [ ] Not Started)
*   [ ] **AI Perception:** Sight/Hearing setup via `UAIPerceptionComponent`.
*   [ ] **Behavior Trees & Blackboards:** Patrol, chase, attack, investigate behaviors.
*   [ ] **Hit Reactions:** Directional hit-stun montages for physical feedback.

### Phase 5: UI, Audio, & Game Loop (Status: [/] Partially Implemented)
*   [x] **GameMode Rules:** Win-condition tracking (best of 5 rounds), coin awards (1000 to winner, 600 to loser), and character state resets/round teleportation.
*   [/] **Diegetic HUD (Wristband):** Designed HUD layout and event-driven Blueprint bindings in `WBP_WristHUD` for Health, Stamina, Coins, and Weapon/Ammo delegates. Next is assigning the class to character's WristHUDComponent and alignment.
*   [x] **Event-Driven UI Delegates:** Implemented `OnWeaponChanged` in `ANemesisCharacter` and `OnAmmoChanged` in `ANemesisWeapon` with automatic `OnRep_CurrentAmmo` replication support for performant event-driven HUD updates. Widget design and placement configurations to be fine-tuned in the Editor.
*   [ ] **Save System:** Player stats and level transitions via binary SaveGames (`USaveGame`).
*   [ ] **VFX/SFX Integration:** Impact particles, footsteps synchronized via Anim Notifies.

### Phase 6: Optimization & Shipping (Status: [ ] Not Started)
*   [ ] Asset cleanup and final packaging configurations for Windows (x64 Shipping).

---

## 🔍 3. Code Review & Replication Audits (Replication Fixed: [x] Yes)

All critical multiplayer replication issues identified in our codebase review have been successfully implemented and compiled:

*   **Attribute Component Replication (Health, Stamina, Coins):** Resolved. Properties are now marked `ReplicatedUsing` pointing to `OnRep` functions that broadcast changes on clients. The component itself is set to replicate.
*   **Weapon Ammo Replication:** Resolved. `CurrentAmmo` is now registered as `Replicated` via `GetLifetimeReplicatedProps`.
*   **Decoy Clone Replication:** Resolved. Constructor now contains `bReplicates = true;` to replicate spawning and movement.

---

## 📍 4. Character Customization Plan (Selected Assets)

To deliver a high-quality tactical military experience with customizable outfits (shirt, vest, helmet, pants, etc.), we have finalized the selection of the following **Free Fab Assets**:

1.  **Quantum Modular Character** (Baseline - *Already Imported in project*): Complete modular soldier skeleton used as the skeleton/movement standard.
2.  **S.W.A.T. Operator - 4k** (For heavy tactical SWAT gear & helmet skins).
3.  **SWAT Soldier (Female)** (For tactical female variant and lightweight outfits).
4.  **Modern Insurgent 7** (For mercenary/insurgent gear and alternative tactical jackets).
5.  **Antiradiation Suit - OZK** (For special hazard/gasmask cosmetic variants).

### Character Swapping Architecture (Status: [x] C++ Foundation Implemented)
*   **Skeletal Mesh Merging / Modular Components:**
    *   [x] Defined `FModularCharacterParts` struct and added a replicated `CharacterParts` variable.
    *   [x] Implemented `UpdateCharacterMeshes()` and `OnRep_CharacterParts()` to apply skeletal mesh assets at runtime and synchronize animations via `LeaderPoseComponent`.
    *   [x] Created the authoritative server RPC `ServerSetCharacterParts()` allowing clients to request outfit changes.
    *   [ ] Integration & Configuration: Bind modular parts in the Unreal Editor inside `BP_NemesisCharacter` or a custom customization UI menu.

---

## 📁 5. Source Code Map

*   **Attributes:** [NemesisAttributeComponent.h](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisAttributeComponent.h) | [NemesisAttributeComponent.cpp](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisAttributeComponent.cpp)
*   **Player Character:** [NemesisCharacter.h](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisCharacter.h) | [NemesisCharacter.cpp](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisCharacter.cpp)
*   **Phantom Decoy:** [NemesisDecoy.h](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisDecoy.h) | [NemesisDecoy.cpp](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisDecoy.cpp)
*   **GameMode Rules:** [NemesisGameMode.h](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisGameMode.h) | [NemesisGameMode.cpp](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisGameMode.cpp)
*   **Weapon Base:** [NemesisWeapon.h](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisWeapon.h) | [NemesisWeapon.cpp](file:///d:/tmp/game/ProjectNemesis/Source/ProjectNemesis/NemesisWeapon.cpp)
