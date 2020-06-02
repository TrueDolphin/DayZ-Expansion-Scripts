/**
 * Hatchback_02.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2020 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class Expansion_Hatchback_02_Door_1_1: Hatchback_02_Door_1_1
{
	override string ExpansionMainType()
	{
		return "Expansion_Hatchback_02_Door_1_1";
	}
}

class Expansion_Hatchback_02_Door_1_2: Hatchback_02_Door_1_2
{
	override string ExpansionMainType()
	{
		return "Expansion_Hatchback_02_Door_1_2";
	}
}

class Expansion_Hatchback_02_Door_2_1: Hatchback_02_Door_2_1
{
	override string ExpansionMainType()
	{
		return "Expansion_Hatchback_02_Door_2_1";
	}
}

class Expansion_Hatchback_02_Door_2_2: Hatchback_02_Door_2_2
{
	override string ExpansionMainType()
	{
		return "Expansion_Hatchback_02_Door_2_2";
	}
}

class Expansion_Hatchback_02_Hood: Hatchback_02_Hood
{
	override string ExpansionMainType()
	{
		return "Expansion_Hatchback_02_Hood";
	}
}

class Expansion_Hatchback_02_Trunk: Hatchback_02_Trunk
{
	override string ExpansionMainType()
	{
		return "Expansion_Hatchback_02_Trunk";
	}
}

modded class Hatchback_02
{
	override bool IsVitalCarBattery()
	{
		return true;
	}
}