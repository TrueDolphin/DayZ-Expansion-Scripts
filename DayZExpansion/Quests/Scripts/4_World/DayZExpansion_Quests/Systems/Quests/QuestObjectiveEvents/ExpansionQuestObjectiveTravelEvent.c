/**
 * ExpansionQuestObjectiveTravelEvent.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionQuestObjectiveTravelEvent: ExpansionQuestObjectiveEventBase
{
	protected ExpansionTravelObjectiveSphereTrigger m_ObjectiveTrigger;
	protected bool m_DestinationReached;
	protected vector m_Position;
	protected int m_PointSearchCount;
	protected ref ExpansionQuestObjectiveTravelConfig m_Config;

	override bool OnEventStart()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!super.OnEventStart())
			return false;
		
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;
		
		Init();

		return true;
	}

	void Init()
	{
		//! Set objective position.
		if (m_Position == vector.Zero)
		{
			if (m_Quest.GetPlayer())
			{
				vector playerPos = m_Quest.GetPlayer().GetPosition();
				m_Position = GetRandomPointInCircle(playerPos, m_Config.GetMaxDistance());
				if (m_Position == vector.Zero)
					m_Position = playerPos;
				else
					m_Position[1] = GetGame().SurfaceY(m_Position[0], m_Position[2]);
			}
		}
		
	#ifdef EXPANSIONMODNAVIGATION
		if (m_Config.GetMarkerName() != string.Empty)
			CreateMarkers();
	#endif
		
		//! Create objective trigger.
		if (!m_ObjectiveTrigger)
			CreateObjectiveTrigger(m_Position);
		
		DestinationCheck();
	}

	void DestinationCheck()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		
		vector position = m_Position;
		float currentDistance;

		if (!GetQuest().GetQuestConfig().IsGroupQuest())
		{
			vector playerPos = GetQuest().GetPlayer().GetPosition();
			currentDistance = vector.Distance(playerPos, position);
		}
	#ifdef EXPANSIONMODGROUPS
		else
		{
			//! Set the position of the group member that has the shortest distance to the target location
			//! as our current position if the quest is a group quest.
			array<vector> groupMemberPos = new array<vector>;
			set<string> memberUIDs = m_Quest.GetPlayerUIDs();
			foreach (string memberUID: memberUIDs)
			{
				PlayerBase groupPlayer = PlayerBase.GetPlayerByUID(memberUID);
				if (!groupPlayer)
					continue;

				groupMemberPos.Insert(groupPlayer.GetPosition());
			}

			float smallestDistance;
			int posIndex;
			bool firstSet = false;
			for (int p = 0; p < groupMemberPos.Count(); p++)
			{
				vector pos = groupMemberPos[p];
				float dist = vector.Distance(pos, position);
				if (!firstSet)
				{
					smallestDistance = dist;
					posIndex = p;
					firstSet = true;
				}
				else if (firstSet && dist < smallestDistance)
				{
					smallestDistance = dist;
					posIndex = p;
				}
			}

			currentDistance = vector.Distance(groupMemberPos[posIndex], position);
		}
	#endif

		float maxDistance = m_Config.GetMaxDistance();

		if (position != vector.Zero && currentDistance <= maxDistance)
		{
			ObjectivePrint("End and return TRUE");
			SetReachedLocation(true);
			return;
		}

		ObjectivePrint("End and return FALSE");
		SetReachedLocation(false);
	}

	override bool OnContinue()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);

		if (!super.OnContinue())
			return false;
		
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return false;
		
		EXTrace.Print(EXTrace.QUESTS, this, "OnContinue - quest state: " + typename.EnumToString(ExpansionQuestState, m_Quest.GetQuestState()) + " trigger on exit: " + m_Config.TriggerOnExit());

		//! Only create the trigger when not already completed!
		if (m_Quest.GetQuestState() == ExpansionQuestState.STARTED || m_Config.TriggerOnExit())
		{
			Init();
		}
		else if (m_Quest.GetQuestState() >= ExpansionQuestState.CAN_TURNIN)
		{
			SetReachedLocation(true);
		}

		return true;
	}

	override bool OnCleanup()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		
		if (!super.OnCleanup())
			return false;

		if (m_ObjectiveTrigger)
			GetGame().ObjectDelete(m_ObjectiveTrigger);

		return true;
	}

	override bool CanComplete()
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		ObjectivePrint("m_DestinationReached: " + m_DestinationReached);

		if (!super.CanComplete())
			return false;
		
		bool conditionsResult = m_DestinationReached;
		if (!conditionsResult)
		{
			ObjectivePrint("End and return: FALSE");
			return false;
		}

		ObjectivePrint("End and return: TRUE");

		return true;
	}

#ifdef EXPANSIONMODNAVIGATION
	override void CreateMarkers()
	{
		if (!Class.CastTo(m_Config, m_ObjectiveConfig))
			return;

		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		string markerName = m_Config.GetMarkerName();
		CreateObjectiveMarker(m_Position, markerName);
	}
#endif

	vector GetPosition()
	{
		return m_Position;
	}

	//! Used by the trigger
	void SetReachedLocation(bool state)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		ObjectivePrint("State: " + state);
		m_DestinationReached = state;
		m_Quest.QuestCompletionCheck();
	}

	void SetLocationPosition(vector pos)
	{
		auto trace = EXTrace.Start(EXTrace.QUESTS, this);
		ObjectivePrint("Position: " + pos.ToString());
		m_Position = pos;
	}

	bool GetLocationState()
	{
		return m_DestinationReached;
	}

	protected void CreateObjectiveTrigger(vector pos)
	{
		Class.CastTo(m_ObjectiveTrigger, GetGame().CreateObjectEx("ExpansionTravelObjectiveSphereTrigger", pos, ECE_NONE));
		m_ObjectiveTrigger.SetPosition(pos);
		m_ObjectiveTrigger.SetObjectiveData(this);
	}

	protected vector GetRandomPointInCircle(vector pos, float radius)
    {
		m_PointSearchCount++;
		if (m_PointSearchCount == 100)
		{
			m_PointSearchCount = 0;
			return vector.Zero;
		}
		else
		{
			ObjectivePrint("m_PointSearchCount: " + m_PointSearchCount);
		}
		
		vector position = ExpansionMath.GetRandomPointInCircle(pos, radius);
        if (GetGame().SurfaceIsSea(position[0], position[2]) || GetGame().SurfaceIsPond(position[0], position[2]))
            return GetRandomPointInCircle(pos, radius); 

        array<Object> position_objects = {};
        array<CargoBase> position_cargos = {};
        GetGame().GetObjectsAtPosition(position, 10, position_objects, position_cargos);
        if (position_objects.Count() > 0)
            return GetRandomPointInCircle(pos, radius);
        
        return position;
    }

	override bool HasDynamicState()
	{
		return true;
	}

	override int GetObjectiveType()
	{
		return ExpansionQuestObjectiveType.TRAVEL;
	}
};
