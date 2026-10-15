Tags: `[qol]` quality of life, small and self-contained · `[perf]` performance fix · `[balance]` needs a design decision before code · `[big]` multi-day feature · `[dream]` may never ship

- `[qol]` Have a linear list of systems to sort through in the interstellar map. Sort by distance, but maybe other sort modes.
    - `[perf]` Related: Fix search lag. The system search does a search for every keystroke. The first characters typed each take time to generate results in bulk that are usually unnecessary
- `[qol]` In the interstellar map, where it shows multiple stars in a single system (and allows targeting those stars for hyperspace jump) indicate which ones have starports and which ones don't. So that I don't have to go to the tab that shows all the stars and planets and starports in that system, to identify which star has the starports and is safe to hyperjump to without having to drift 2000 AUs to the correct star and wait real-time hours even when the game time is sped up to its max.
- `[balance]` Make military drive more OP, ideas:
    - `[big]` Military drive can target individual planets (or stars) in a system, and do same-system jumps
    - `[balance]` Idea: Military drives use *less fuel* the longer the jump. Must consider how that can be abused, though.
- `[balance]` Get rid of separate hyperdrive fuel tank. Take fuel out of the *main* fuel tank for hyperdrive. Use separate fuel tank for military drive
- `[qol]` Auto refuel-from-cargo toggle
- `[qol]` Scoop can also refuel to the primary fuel tank. Should also add auto-handling to that. Toggle scoop goes to primary, or to cargo by default, goes to the next option when the default is full.
- `[qol]` Upper left corner HUD dropdown for mission objectives with buttons for setting destination as nav target
- `[qol]` Sort commodities by value
- `[qol]` Sort nearby systems by major export or major import of specific commodities (maybe put features like this under an upgraded trading computer)
- `[balance]` Add other gases and compounds (carbon dioxide, methane, oxygen) as commodities that can be scooped from atmospheres and sold or bought
    - `[balance]` Some kind of recipe for military fuel, and a ship addon that converts that stuff into military fuel (have to establish exactly what military fuel is. it's obviously something that generates radioactive waste when expended)


Big ones:
- `[big]` Bullets instead of blasters. Bullet damage is a multiplication of the velocity of impact, bullets are affected by gravity, bullets have a limited amount of ammo and they also generate heat just like blasters do right now.
- `[big]` Atmospheres have a solid, black-and-white barrier. Travel fast enough, and you instantly transition from flying through vacuum to hitting a brick wall of atmosphere. We should find a way of getting rid of that solid boundary. Maybe calculate dynamic pressure based on altitude. Go fast enough, even if you're pretty high, you will meet noticable atmospheric resistance from trace atmosphere.
    - Note: `Planet::GetAtmosphericState` (`src/Planet.cpp:80`) sets density to exactly 0 above `m_atmosphereRadius`, but the adiabatic model should already be near 0 there. The wall may come from drag rising with speed squared instead. Measure density at the edge first.
- `[big]` Add autopilots:
    - `[big]` An autopilot for fuel scooping
- `[balance]` Thruster efficiency: consider making strafing thrusters high powered, but less efficient, for combat strafing. Keep retro and main thrusters at the same efficiency.
- `[big]` Simulate slopes on planets. Allow sliding of objects on slopes. Have the ship match the tilt of the slope, if it doesn't already.
    - Note: landing on slopes up to 30° now tilts the ship to match (upstream Pioneer issue 6420 fix). Terrain collision still samples one point under the ship and pushes straight up (`src/Space.cpp` `CollideWithTerrain`), so nothing slides yet.

Difficult, lofty, Doom Bible level near-fantasy ideas, cool on paper but may be too difficult to make it to the final product
- `[big]` Relativistic visual effects: a shader for ships at relativistic speeds (aberration bunching stars ahead, Doppler colour shift, brightening). What else do we emulate visually? Light lag from distant objects like stars or planets in the same system?
- `[dream]` Relativistic time dilation simulation: ships capable of accelerating to relativistic speeds, and the time the player experiences warps accordingly (ship clock vs. galaxy clock).
    - Possibly track NPC ship clocks too.
    - If we're really generous: time dilation on planets that orbit at high speeds or sit in strong gravity.
- `[dream]` Kerbal Space Program style build-your-own spacecraft, fuel tank staging, stuff like that
- `[big]` Docking (Pioneer developers may already be working on this)
- `[dream]` Tunnels and caves filled with enemies, Terminal Velocity/Fury3/Hellbender style missions but with Newtonian physics
