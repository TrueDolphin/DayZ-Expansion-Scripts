/**
 * PlayerBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

modded class PlayerBase
{
	// ------------------------------------------------------------
	// PlayerBase SetActions
	// ------------------------------------------------------------
	override void SetActions( out TInputActionMap InputActionMap )
	{
		super.SetActions(InputActionMap);

		AddAction(ExpansionActionOpenQuestMenu, InputActionMap);
	}

	// ------------------------------------------------------------
	// PlayerBase EEKilled
	// ------------------------------------------------------------
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		EntityAI killSource = EntityAI.Cast(killer);

		if (!killSource)
			return;

		ExpansionQuestModule questModule = ExpansionQuestModule.Cast(CF_ModuleCoreManager.Get(ExpansionQuestModule));
		if (!questModule)
			return;

		PlayerBase player;

		if (Class.CastTo(player, killSource.GetHierarchyRootPlayer()))
		{
			if (player.GetIdentity())
				questModule.OnEntityKilled(this, killSource, player.GetIdentity().GetId());
		}
		else if (killSource.IsAnimal() || killSource.IsInherited(AnimalBase))
		{
			questModule.OnEntityKilled(this, killSource);
		}
		else if (killSource.IsZombie() || killSource.IsInherited(ZombieBase))
		{
			questModule.OnEntityKilled(this, killSource);
		}
	}
};
