/**
 * AnimalBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

modded class AnimalBase
{
	protected autoptr ExpansionZoneActor m_Expansion_SafeZoneInstance = new ExpansionZoneEntity<AnimalBase>(this);

	protected bool m_Expansion_IsInSafeZone;
	protected bool m_Expansion_IsInSafeZone_DeprecationWarning;

	void OnEnterZone(ExpansionZoneType type)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.ZONES, this, "OnEnterZone");
#endif

		if (type == ExpansionZoneType.SAFE)
		{
			m_Expansion_IsInSafeZone = true;

			if (IsDanger())
			{
				SetHealth(0);
				GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( GetGame().ObjectDelete, 5000, false, this );
			}
		}
	}

	void OnExitZone(ExpansionZoneType type)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.ZONES, this, "OnExitZone");
#endif

		if (type == ExpansionZoneType.SAFE)
		{
			m_Expansion_IsInSafeZone = false;
		}
	}

	bool IsInSafeZone()
	{
		Expansion_Error("DEPRECATED: Please use Expansion_IsInSafeZone", m_Expansion_IsInSafeZone_DeprecationWarning);
		return Expansion_IsInSafeZone();
	}

	bool Expansion_IsInSafeZone()
	{
		return m_Expansion_IsInSafeZone;
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (damageType == DT_EXPLOSION && ExpansionDamageSystem.IsEnabledForExplosionTarget(this))
			ExpansionDamageSystem.OnExplosionHit(source, this, ammo, true, damageResult);
	}
}
