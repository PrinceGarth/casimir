-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Event       = require 'Event'
local Commodities = require 'Commodities'
local Game        = require 'Game'
local Lang        = require 'Lang'

local lc = Lang.GetResource("core")

-- Utility functions to handle fuel and cargo scooping.
-- These callbacks are triggered from C++ or Lua and are responsible for
-- providing UI feedback about the scooping process.

-- The ship fills its own propellant tank as it scoops (see Ship::StaticUpdate).
-- This event brings the hydrogen the tank had no room for, in whole tonnes.
-- It goes into the hyperdrive's reservoir, if the drive burns hydrogen, and
-- then into the cargo hold. Hydrogen that fits nowhere is lost.
Event.Register("onShipScoopFuel", function(ship, body, amount)
	-- events are queued, so the ship may be gone by now
	if not ship:exists() then return end

	local drive = ship:GetInstalledHyperdrive()
	if drive and drive.fuel == Commodities.hydrogen then
		local added = math.min(amount, drive:GetMaxFuel() - drive.storedFuel)
		if added > 0 then
			drive:SetFuel(ship, drive.storedFuel + added)
			amount = amount - added
		end
	end

	---@type CargoManager
	local cargoMgr = ship:GetComponent('CargoManager')

	-- the hold takes whole tonnes only
	local stowed = math.min(math.floor(amount), cargoMgr:GetFreeSpace())
	if stowed <= 0 then return end

	cargoMgr:AddCommodity(Commodities.hydrogen, stowed)

	if ship == Game.player then
		Game.AddCommsLogLine(lc.FUEL_SCOOP_ACTIVE_N_TONNES_H_COLLECTED:gsub('%%quantity',
			cargoMgr:CountCommodity(Commodities.hydrogen)))
	end
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
