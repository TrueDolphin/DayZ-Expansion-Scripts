/**
 * InfectedSteakMeat.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class Expansion_KeyCard_NA_Antenna extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();

		AddAction(ExpansionActionUseSatelliteControl);
	}

	void OnCardUsed()
	{
		AddHealthLevel(1);
	}
};