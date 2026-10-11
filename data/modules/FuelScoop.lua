-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event       = require 'Event'
local Commodities = require 'Commodities'
local Game        = require 'Game'
local Lang        = require 'Lang'

local lc = Lang.GetResource("core")
local l = Lang.GetResource("casimir-ui")

-- Utility functions to handle fuel and cargo scooping.
-- These callbacks are triggered from C++ or Lua and are responsible for
-- providing UI feedback about the scooping process.

-- Hydrogen waiting to go into a ship's cargo hold, which takes whole tonnes
-- only: ship -> tonnes (always less than one)
local waitingForHold = {}

-- The last tenth of the player's propellant tank that was reported (0 to 10)
local reportedTenth = nil

-- Tell the player each time scooping takes the propellant tank past another
-- tenth. A mark is reported again only after the level has fallen well below
-- it: with the engines burning, a full tank dips under full and is topped up
-- again several times a second.
local function reportTank(ship)
	-- percent; nudged up because the engines have already burnt a little of a
	-- tank that was filled to the brim this frame
	local level = ship.fuel + 0.01
	local tenth = math.floor(level / 10)

	if not reportedTenth or level < reportedTenth * 10 - 5 then
		reportedTenth = tenth
	elseif tenth > reportedTenth then
		reportedTenth = tenth
		Game.AddCommsLogLine(l.FUEL_SCOOP_TANK_LEVEL:interp({ percent = tenth * 10 }))
	end
end

-- The ship fills its own propellant tank as it scoops (see Ship::StaticUpdate).
-- This event comes a few times a second while it does, and brings the hydrogen
-- the tank had no room for. That goes into the hyperdrive's own tank, if the
-- drive burns hydrogen, and then into the cargo hold. Hydrogen that fits
-- nowhere is lost.
Event.Register("onShipScoopFuel", function(ship, body, amount)
	-- events are queued, so the ship may be gone by now
	if not ship:exists() then return end

	local isPlayer = ship == Game.player
	if isPlayer then reportTank(ship) end

	if amount <= 0 then return end

	local drive = ship:GetInstalledHyperdrive()
	if drive and drive.fuel == Commodities.hydrogen then
		local before, max = drive.storedFuel, drive:GetMaxFuel()
		local room = max - before
		if room > 0 then
			-- near enough to full counts as full, so rounding can't leave a sliver
			local fillsDrive = amount >= room - 1e-9
			local after = fillsDrive and max or before + amount
			drive:SetFuel(ship, after)
			amount = fillsDrive and math.max(amount - room, 0) or 0

			-- report each tenth of the tank reached, as for the propellant tank
			local tenth = max / 10
			local reached = fillsDrive and 10 or math.min(9, math.floor(after / tenth + 1e-9))
			if isPlayer and reached > math.floor(before / tenth + 1e-9) then
				Game.AddCommsLogLine(l.FUEL_SCOOP_HYPERDRIVE_LEVEL:interp({
					quantity = string.format("%.1f", reached * tenth) }))
			end
		end
	end

	if amount <= 0 then return end

	---@type CargoManager
	local cargoMgr = ship:GetComponent('CargoManager')
	local freeSpace = math.floor(cargoMgr:GetFreeSpace())

	-- What is left of a tonne waits for the next event, unless the hold is full
	local waiting = (waitingForHold[ship] or 0) + amount
	local stowed = math.min(math.floor(waiting), freeSpace)
	waitingForHold[ship] = stowed < freeSpace and waiting - stowed or nil

	if stowed <= 0 then return end

	cargoMgr:AddCommodity(Commodities.hydrogen, stowed)

	if isPlayer then
		Game.AddCommsLogLine(lc.FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED:gsub('%%quantity',
			cargoMgr:CountCommodity(Commodities.hydrogen)))
	end
end)

Event.Register("onShipDestroyed", function (ship)
	waitingForHold[ship] = nil
end)

Event.Register("onGameEnd", function ()
	waitingForHold = {}
	reportedTenth = nil
end)

Event.Register("onShipScoopCargo", function (ship, success, cargoType)
	if ship ~= Game.player then return end

	if success then
		Game.AddCommsLogLine(lc.CARGO_SCOOP_ACTIVE_1_TONNE_X_COLLECTED:gsub('%%item',
			cargoType:GetName()))
	else
		Game.AddCommsLogLine(lc.CARGO_SCOOP_ATTEMPTED)
	end
end)
