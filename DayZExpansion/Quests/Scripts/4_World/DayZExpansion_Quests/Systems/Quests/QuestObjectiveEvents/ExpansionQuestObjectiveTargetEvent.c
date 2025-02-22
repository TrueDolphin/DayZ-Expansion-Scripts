/**
 * ExpansionQuestObjectiveTargetEvent.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionQuestObjectiveTargetEvent: ExpansionQuestObjectiveEventBase
{
	protected int m_Count;
	protected int m_Amount;
	protected ref ExpansionQuestObjectiveTargetConfig m_Config;
	
	override bool OnEventStart()
	{
		if (!super.OnEventStart())
			return false;
		
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;
		
		return true;
	}

	override bool OnContinue()
	{
		if (!super.OnContinue())
			return false;
		
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;

		m_Quest.QuestCompletionCheck();

		return true;
	}

	override void OnEntityKilled(EntityAI victim, EntityAI killer, Man killerPlayer = NULL)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		
		ExpansionQuestObjectiveTarget target = m_Config.GetTarget();
		if (!target)
			return;

		if (ExpansionStatic.IsAnyOf(victim, target.GetExcludedClassNames(), true))
			return;

		bool maxRangeCheck = false;

		PlayerBase victimPlayer;
		if (Class.CastTo(victimPlayer, victim))
		{
			//! Check if this was a self-kill
			if (m_Quest.GetPlayer() == victimPlayer && !target.CountSelfKill())
				return;

		#ifdef EXPANSIONMODAI
			if (victimPlayer.IsAI() && !target.CountAIPlayers())
				return;
		#endif

			//! PvP quest objective. Check if the victim is a quest player
			//! of this quest and if its a group quest make sure he was not in the involved party before.
			//! If he is in the related group or was in it we dont count the kill!
		#ifdef EXPANSIONMODGROUPS
			if (m_Quest.GetQuestConfig().IsGroupQuest() && victimPlayer.GetIdentity())
			{
				string victimPlayerUID = victimPlayer.GetIdentity().GetId();
				if (ExpansionQuestModule.GetModuleInstance().IsGroupPlayer(m_Quest.GetGroupID(), victimPlayerUID))
					return;
			}
		#endif
		}

		if (killerPlayer && !IsInMaxRange(killerPlayer.GetPosition()))
		{
			ObjectivePrint("Killer is out of legit kill range! Skip..");
			return;
		}

		//! If the target need to be killed with a special weapon check incoming killer class type
		if (target.NeedSpecialWeapon() && !ExpansionStatic.IsAnyOf(killer, target.GetAllowedWeapons(), true))
		{
			ObjectivePrint("Entity got not killed with any allowed weapon! Skip..");
			return;
		}

		bool found = ExpansionStatic.IsAnyOf(victim, target.GetClassNames(), true);
		if (found)
		{
			ObjectivePrint("Killed valid target " + victim + " " + ExpansionString.JoinStrings(target.GetClassNames()));
			if (m_Count < m_Amount)
			{
				m_Count++;
				m_Quest.QuestCompletionCheck(true);
			}
		}
	}

	override bool CanComplete()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		ObjectivePrint("m_Count: " + m_Count);
		ObjectivePrint("m_Amount: " + m_Amount);

		if (m_Amount == 0)
			return false;
		
		bool conditionsResult = (m_Count == m_Amount);
		if (!conditionsResult)
		{
			ObjectivePrint("End and return: FALSE");
			return false;
		}

		ObjectivePrint("End and return: TRUE");

		return super.CanComplete();
	}

	protected bool IsInMaxRange(vector playerPos)
	{
		vector position = m_Config.GetPosition();
		if (position == vector.Zero)
			return true;

		float maxDistance = m_Config.GetMaxDistance();
		if (maxDistance <= 0)
			return true;

		position[1] = GetGame().SurfaceY(position[0], position[2]);

		float currentDistanceSq = vector.DistanceSq(playerPos, position);

		if (currentDistanceSq <= maxDistance * maxDistance)
			return true;

		return false;
	}

	void SetCount(int count)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		
		m_Count = count;
		
		ObjectivePrint("Count: " + m_Count);		
		
	}

	int GetCount()
	{
		return m_Count;
	}

	void SetAmount(int amount)
	{
		m_Amount = amount;
	}

	int GetAmount()
	{
		return m_Amount;
	}

	override int GetObjectiveType()
	{
		return ExpansionQuestObjectiveType.TARGET;
	}
};