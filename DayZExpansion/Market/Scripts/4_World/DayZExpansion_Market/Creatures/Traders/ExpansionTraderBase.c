/**
 * ExpansionTraderBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionTraderNPCBase
 * @brief		This class handles trader NPCs
 **/
class ExpansionTraderNPCBase: ExpansionNPCBase
{
	private static ref set<ExpansionTraderNPCBase> m_allTraders = new set<ExpansionTraderNPCBase>;

	private ref ExpansionTraderObjectBase m_TraderObject;
	
	// ------------------------------------------------------------
	// ExpansionTraderNPCBase Constructor
	// ------------------------------------------------------------
	void ExpansionTraderNPCBase()
	{
		if (IsMissionHost())
		{
			SetAllowDamage(false);
		}

		m_allTraders.Insert(this);

		if (GetGame() && GetGame().IsClient())
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(LoadTrader, 250);
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderNPCBase Destructor
	// ------------------------------------------------------------
	void ~ExpansionTraderNPCBase()
	{
		if (!GetGame())
			return;
		
		int idx = m_allTraders.Find(this);
		if (idx >= 0)
		{
			m_allTraders.Remove(idx);
		}
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderNPCBase GetAll
	// ------------------------------------------------------------
	static set<ExpansionTraderNPCBase> GetAll()
	{
		return m_allTraders;
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderNPCBase OnRPC
	// ------------------------------------------------------------
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (m_TraderObject)
			m_TraderObject.OnRPC(sender, rpc_type, ctx);
	}

	void SetTraderObject(ExpansionTraderObjectBase traderObject)
	{
		m_TraderObject = traderObject;
		EXPrint(ToString() + "::SetTraderObject " + m_TraderObject + " " + GetPosition());
	}

	ExpansionTraderObjectBase GetTraderObject()
	{
		return m_TraderObject;
	}

	void LoadTrader(string fileName = "")
	{
		SetTraderObject(new ExpansionTraderObjectBase(this, fileName));
	}

	bool Expansion_IsTrader()
	{
		return true;
	}
};

/**@class		ExpansionTraderObjectBase
 * @brief		This class handles all trader functionality
 **/
class ExpansionTraderObjectBase
{
	private static ref set<ExpansionTraderObjectBase> m_allTraderObjects = new set<ExpansionTraderObjectBase>;
	static ref map<string, vector> s_VehicleSizes = new map<string, vector>;

	protected ref ExpansionMarketTraderZone m_TraderZone;
	protected ref ExpansionMarketTrader m_Trader;

	private EntityAI m_TraderEntity;
	
	ref map<string, bool> m_TradingPlayers;

	// ------------------------------------------------------------
	// ExpansionTraderObjectBase Constructor
	// ------------------------------------------------------------
	void ExpansionTraderObjectBase(EntityAI traderEntity, string fileName = "")
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "ExpansionTraderObjectBase");
#endif
		
		m_allTraderObjects.Insert(this);

		m_TradingPlayers = new map<string, bool>;

		SetTraderEntity(traderEntity);
		LoadTrader(fileName);
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase Destructor
	// ------------------------------------------------------------
	void ~ExpansionTraderObjectBase()
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "~ExpansionTraderObjectBase");
#endif

		if (!GetGame())
			return;

		int idx = m_allTraderObjects.Find(this);
		if (idx >= 0)
		{
			m_allTraderObjects.Remove(idx);
		}
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase GetAll
	// ------------------------------------------------------------
	static set<ExpansionTraderObjectBase> GetAll()
	{
		return m_allTraderObjects;
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase LoadTrader
	// ------------------------------------------------------------
	void LoadTrader(string fileName)
	{
		if (IsMissionHost())
		{
			LoadTraderHost(fileName);
		} 
		else
		{
			RequestTraderObject();
		}
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase LoadTraderHost
	// ------------------------------------------------------------
	void LoadTraderHost(string fileName)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "LoadTraderHost");
#endif

		if (!m_TraderEntity)
		{
			Error(ToString() + "::LoadTraderHost - ERROR: Trader does not have entity!");
			return;
		}
		
		CF_Log.Debug(ToString() + "::LoadTraderHost - m_TraderEntity: " + m_TraderEntity + " " + m_TraderEntity.GetType());

		m_Trader = GetExpansionSettings().GetMarket().GetMarketTrader(fileName);
		if (!m_Trader)
		{
			if (fileName)
			{
				CF_Log.Error(ToString() + "::LoadTraderHost - ERROR: Trader does not exist: " + fileName);
			}
			return;
		}

		m_TraderZone = GetExpansionSettings().GetMarket().GetTraderZoneByPosition(m_TraderEntity.GetPosition());
		
		CF_Log.Debug(ToString() + "::LoadTraderHost - m_Trader: " + m_Trader);

		if (!m_TraderZone)
		{
			CF_Log.Error(ToString() + "::LoadTraderHost - ERROR: Trader is not within a trader zone: " + m_TraderEntity + " " + m_TraderEntity.GetPosition());
			return;
		}

		CF_Log.Debug(ToString() + "::LoadTraderHost - trader zone: " + m_TraderZone.m_FileName);

		UpdateTraderZone();
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase RequestTraderObject
	// ------------------------------------------------------------
	void RequestTraderObject()
	{
		if ( IsMissionOffline() )
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(this.GetTraderEntity(), ExpansionMarketRPC.TraderObject, true, NULL);
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase OnRPC
	// ------------------------------------------------------------
	void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		if (rpc_type <= ExpansionMarketRPC.INVALID)
			return;
		if (rpc_type >= ExpansionMarketRPC.COUNT)
			return;
		
		switch (rpc_type)
		{
		case ExpansionMarketRPC.TraderObject:
			RPC_TraderObject(ctx, sender);
			break;
		}
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase RPC_TraderObject
	// ------------------------------------------------------------
	private void RPC_TraderObject(ParamsReadContext ctx, PlayerIdentity sender)
	{		
		if (GetGame().IsServer())
		{
			if (!m_Trader)
				return;

			ScriptRPC rpc = new ScriptRPC();

			rpc.Write(m_Trader.m_FileName);
			rpc.Write(m_Trader.DisplayName);
			rpc.Write(m_Trader.TraderIcon);
			rpc.Write(m_Trader.Currencies);
			rpc.Write(m_Trader.Categories);

			rpc.Send(this.GetTraderEntity(), ExpansionMarketRPC.TraderObject, true, sender);
		}
		else
		{
			string fileName;
			if (!ctx.Read(fileName))
				return;
				
			m_Trader = GetExpansionSettings().GetMarket().GetMarketTrader(fileName);
			if (!m_Trader)
			{
				m_Trader = new ExpansionMarketTrader;
				m_Trader.m_FileName = fileName;
				GetExpansionSettings().GetMarket().AddMarketTrader(m_Trader);
			}

			if (!ctx.Read(m_Trader.DisplayName))
				return;

			if (!ctx.Read(m_Trader.TraderIcon))
				return;

			if (!ctx.Read(m_Trader.Currencies))
				return;

			if (!ctx.Read(m_Trader.Categories))
				return;
		}
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase UpdateTraderZone
	// ------------------------------------------------------------
	void UpdateTraderZone()
	{
		if (!m_TraderZone)
			return;

		#ifdef EXPANSIONMODMARKET_DEBUG
		EXPrint(ToString() + "::UpdateTraderZone - trader " + m_Trader.m_FileName + " - trader zone " + m_TraderZone.m_DisplayName + " - stock before update");
		m_TraderZone.DebugPrint();
		#endif

		bool updated;

		foreach (ExpansionMarketTraderItem item : m_Trader.m_Items)
		{
			if (!m_TraderZone.Stock.Contains(item.MarketItem.ClassName))
			{
				ExpansionMarketCategory cat = GetExpansionSettings().GetMarket().GetCategory(item.MarketItem.CategoryID);
				int newStock;
				if (item.MarketItem.IsStaticStock())
					newStock = 1;
				else
					newStock = item.MarketItem.MaxStockThreshold;
				if (cat)
					newStock = newStock * cat.InitStockPercent * 0.01;
				m_TraderZone.Stock.Insert(item.MarketItem.ClassName, newStock);
				updated = true;
			}
		}

		if (updated)
			m_TraderZone.Save();

		#ifdef EXPANSIONMODMARKET_DEBUG
		EXPrint(ToString() + "::UpdateTraderZone - trader " + m_Trader.m_FileName + " - trader zone " + m_TraderZone.m_DisplayName + " - stock after update");
		m_TraderZone.DebugPrint();
		#endif
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase GetNetworkSerialization
	// ------------------------------------------------------------
	int GetNetworkSerialization(out array<ref ExpansionMarketNetworkItem> list, int start, bool stockOnly, TIntArray itemIDs = NULL)
	{
		if (!m_Trader)
		{
			Error("ExpansionTraderObjectBase::GetNetworkSerialization - Error reading m_Trader");
			return -1;
		}
		
		if (!m_TraderZone)
		{
			Error("ExpansionTraderObjectBase::GetNetworkSerialization - Error reading m_TraderZone");
			return -1;
		}
		
		return m_TraderZone.GetNetworkSerialization(m_Trader, list, start, stockOnly, itemIDs);
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase GetTraderZone
	// ------------------------------------------------------------
	ExpansionMarketTraderZone GetTraderZone()
	{
		return m_TraderZone;
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase GetTraderMarket
	// ------------------------------------------------------------
	ExpansionMarketTrader GetTraderMarket()
	{
		return m_Trader;
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase HasVehicleSpawnPosition
	// ------------------------------------------------------------
	bool HasVehicleSpawnPosition(string className, out vector spawnPosition, out vector spawnOrientation, out ExpansionMarketVehicleSpawnType spawnType = 0, out ExpansionMarketResult result = ExpansionMarketResult.Success, out Object blockingObject = NULL, int amountNeeded = 1)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "HasVehicleSpawnPosition");
#endif

		//array<vector> positions;
		array<ref ExpansionMarketSpawnPosition> positions;

		string vehicleClass = GetGame().ConfigGetTextOut("CfgVehicles " + className + " vehicleClass");

		switch (vehicleClass)
		{
			case "Expansion_Boat":
			case "Expansion_Ship":
				positions = GetExpansionSettings().GetMarket().WaterSpawnPositions;
				spawnType = ExpansionMarketVehicleSpawnType.WATER;
				break;
			case "Expansion_Helicopter":
			case "Expansion_Plane":
				positions = GetExpansionSettings().GetMarket().AirSpawnPositions;
				spawnType = ExpansionMarketVehicleSpawnType.AIR;
				break;
			default:
				positions = GetExpansionSettings().GetMarket().LandSpawnPositions;
				spawnType = ExpansionMarketVehicleSpawnType.LAND;
		}

		if (!positions || !positions.Count())
		{
			result = ExpansionMarketResult.FailedNoVehicleSpawnPositions;
			return false;
		}

		map<ref ExpansionMarketSpawnPosition, float> foundPositions = new map<ref ExpansionMarketSpawnPosition, float>;

		float minDistance = GetExpansionSettings().GetMarket().GetMinVehicleDistanceToTrader(className);
		float maxDistance = GetExpansionSettings().GetMarket().GetMaxVehicleDistanceToTrader(className);

		ExpansionMarketSpawnPosition lastCheckedPos;

		Object tempBlockingObject;
		foreach (ExpansionMarketSpawnPosition position : positions)
		{
			float distance = vector.Distance( position.Position, m_TraderEntity.GetPosition() );

			if (distance < minDistance || distance > maxDistance)
				continue;
			
			lastCheckedPos = position;

			if (!VehicleSpawnPositionFree(position, className, tempBlockingObject))
			{
				blockingObject = tempBlockingObject;
				continue;
			}
			
			CF_Log.Debug(ToString() + "::HasVehicleSpawnPosition - found candidate " + position.Position + " (distance " + distance + ") for " + className);

			foundPositions.Insert(position, distance);
		}
		
		CF_Log.Debug(ToString() + "::HasVehicleSpawnPosition - found " + foundPositions.Count() + " candidate spawn positions for " + className + ", needed " + amountNeeded);
		
		if (foundPositions.Count() >= amountNeeded)
		{
			float closestDistance = int.MAX;

			//! Select closest one
			foreach (ExpansionMarketSpawnPosition candidatePosition, float candidateDistance : foundPositions)
			{
				if (candidateDistance < closestDistance)
				{
					closestDistance = candidateDistance;
					spawnPosition = candidatePosition.Position;
					spawnOrientation = candidatePosition.Orientation;
				}
			}
			
			CF_Log.Debug(ToString() + "::HasVehicleSpawnPosition - selected " + spawnPosition + " (distance " + closestDistance + ") for " + className);
			
			result = ExpansionMarketResult.Success;

			return true;
		}

		if (!lastCheckedPos)
			result = ExpansionMarketResult.FailedNotEnoughVehicleSpawnPositionsNear;
		else if (blockingObject)
			result = ExpansionMarketResult.FailedVehicleSpawnOccupied;

		return false;
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderObjectBase VehicleSpawnPositionFree
	// ------------------------------------------------------------	
	private bool VehicleSpawnPositionFree(ExpansionMarketSpawnPosition position, string className, out Object blockingObject = NULL)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "VehicleSpawnPositionFree");
#endif

		vector size;
		if (!s_VehicleSizes.Find(className, size))
		{
			vector minMax[2];
			if (ExpansionStatic.GetCollisionBox(className, minMax))
			{
				size = Vector(minMax[1][0] - minMax[0][0], minMax[1][1] - minMax[0][1], minMax[1][2] - minMax[0][2]);

				#ifdef EXPANSION_MARKET_VEHICLE_SPAWN_DEBUG
				Object debugBox;

				//! Bottom left
				debugBox = GetGame().CreateObjectEx("ExpansionDebugBox_Blue", position.Position, ECE_NOLIFETIME);
				debugBox.SetOrientation(position.Orientation);
				debugBox.SetPosition(debugBox.ModelToWorld(Vector(-size[0] / 2, 0, -size[2] / 2)));

				//! Bottom right
				debugBox = GetGame().CreateObjectEx("ExpansionDebugBox_Orange", position.Position, ECE_NOLIFETIME);
				debugBox.SetOrientation(position.Orientation);
				debugBox.SetPosition(debugBox.ModelToWorld(Vector(size[0] / 2, 0, -size[2] / 2)));

				//! Top right
				debugBox = GetGame().CreateObjectEx("ExpansionDebugBox_Red", position.Position, ECE_NOLIFETIME);
				debugBox.SetOrientation(position.Orientation);
				debugBox.SetPosition(debugBox.ModelToWorld(Vector(size[0] / 2, size[1], size[2] / 2)));

				//! Top left
				debugBox = GetGame().CreateObjectEx("ExpansionDebugBox_Purple", position.Position, ECE_NOLIFETIME);
				debugBox.SetOrientation(position.Orientation);
				debugBox.SetPosition(debugBox.ModelToWorld(Vector(-size[0] / 2, size[1], size[2] / 2)));
				#endif
			}
			else
			{
				//! Fallback just in case
				size = "5 5 10";
			}
			s_VehicleSizes.Insert(className, size);
		}

		blockingObject = ExpansionGetObjectBlockingPosition(position, size);

		CF_Log.Debug("VehicleSpawnPositionFree " + className + " size " + size + " pos " + position.Position + " ori " + position.Orientation + " blocking " + blockingObject);
		
		return blockingObject == NULL;
	}
	
	Object ExpansionGetObjectBlockingPosition(ExpansionMarketSpawnPosition position, vector size)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "ExpansionGetObjectBlockingPosition");
#endif

		array<Object> excluded_objects = new array<Object>;
		array<Object> objects = new array<Object>;
		
		if (!GetGame().IsBoxColliding( position.Position + Vector(0, size[1] / 2, 0), position.Orientation, size, excluded_objects, objects))
			return NULL;

		foreach (Object obj: objects)
		{
			bool match = obj.IsInherited(Man) || (obj.IsInherited(ItemBase) && obj.ConfigGetString("physLayer") == "item_large") || obj.IsInherited(CarScript);

			#ifdef EXPANSIONMODVEHICLE
			match |= obj.IsInherited(ExpansionVehicleBase);
			#endif

			if (match)
				return obj;
		}

		return NULL;
	}

	string GetDisplayName()
	{
		if (!GetTraderMarket())
			return string.Empty;

		string traderName = GetTraderMarket().DisplayName;
		if (traderName.Contains("#"))
		{
			StringLocaliser name = new StringLocaliser(traderName);
			traderName = name.Format();
		}

		if (!m_TraderEntity)
			return traderName;

		return m_TraderEntity.GetDisplayName() + " [" + traderName + "]";
	}

	void SetTraderEntity(EntityAI entity)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.MARKET, this, "SetTraderEntity").Add(entity);
#endif

		m_TraderEntity = entity;
	}

	EntityAI GetTraderEntity()
	{
		return m_TraderEntity;
	}
}

/**@class		ExpansionTraderStaticBase
 * @brief		This class handles static trader objects
 **/
class ExpansionTraderStaticBase extends BuildingSuper
{
	private static ref set<ExpansionTraderStaticBase> m_allTraders = new set<ExpansionTraderStaticBase>;

	private ref ExpansionTraderObjectBase m_TraderObject;

	// ------------------------------------------------------------
	// ExpansionTraderStaticBase Constructor
	// ------------------------------------------------------------
	void ExpansionTraderStaticBase()
	{
		if (IsMissionHost())
		{
			SetAllowDamage(false);
		}

		m_allTraders.Insert(this);

		m_Expansion_NameOverride = new ExpansionNameOverride(this);

		if (GetGame() && GetGame().IsClient())
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(LoadTrader, 250);
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderStaticBase Destructor
	// ------------------------------------------------------------
	void ~ExpansionTraderStaticBase()
	{
		if (!GetGame())
			return;

		int idx = m_allTraders.Find(this);
		if (idx >= 0)
		{
			m_allTraders.Remove(idx);
		}
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderStaticBase GetAll
	// ------------------------------------------------------------
	static set<ExpansionTraderStaticBase> GetAll()
	{
		return m_allTraders;
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderStaticBase OnRPC
	// ------------------------------------------------------------
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (m_TraderObject)
			m_TraderObject.OnRPC(sender, rpc_type, ctx);
	}

	void SetTraderObject(ExpansionTraderObjectBase traderObject)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "SetTraderObject");
#endif

		m_TraderObject = traderObject;
	}

	ExpansionTraderObjectBase GetTraderObject()
	{
		return m_TraderObject;
	}

	void LoadTrader(string fileName = "")
	{
		SetTraderObject(new ExpansionTraderObjectBase(this, fileName));
	}

	bool Expansion_IsTrader()
	{
		return true;
	}
}

/**@class		ExpansionTraderZombieBase
 * @brief		This class handles Infected traders
 **/
class ExpansionTraderZombieBase extends ZombieBase
{
	private static ref set<ExpansionTraderZombieBase> m_allTraders = new set<ExpansionTraderZombieBase>;

	private ref ExpansionTraderObjectBase m_TraderObject;

	// ------------------------------------------------------------
	// ExpansionTraderZombieBase Constructor
	// ------------------------------------------------------------
	void ExpansionTraderZombieBase()
	{
		if (IsMissionHost())
		{
			SetAllowDamage(false);
		}

		m_allTraders.Insert(this);

		m_Expansion_NameOverride = new ExpansionNameOverride(this);

		if (GetGame() && GetGame().IsClient())
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(LoadTrader, 250);
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderZombieBase Destructor
	// ------------------------------------------------------------
	void ~ExpansionTraderZombieBase()
	{
		if (!GetGame())
			return;

		int idx = m_allTraders.Find(this);
		if (idx >= 0)
		{
			m_allTraders.Remove(idx);
		}
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderZombieBase GetAll
	// ------------------------------------------------------------
	//static set<ExpansionTraderZombieBase> GetAll()
	//{
		//return m_allTraders;
	//}
	
	// ------------------------------------------------------------
	// ExpansionTraderZombieBase OnRPC
	// ------------------------------------------------------------
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (m_TraderObject)
			m_TraderObject.OnRPC(sender, rpc_type, ctx);
	}

	void SetTraderObject(ExpansionTraderObjectBase traderObject)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "SetTraderObject");
#endif

		m_TraderObject = traderObject;
	}

	ExpansionTraderObjectBase GetTraderObject()
	{
		return m_TraderObject;
	}

	void LoadTrader(string fileName = "")
	{
		SetTraderObject(new ExpansionTraderObjectBase(this, fileName));
	}

	override void OnEnterZone(ExpansionZoneType type)
	{
		if (type != ExpansionZoneType.SAFE) return;

		m_Expansion_IsInSafeZone = true;
	}

	override void OnExitZone(ExpansionZoneType type)
	{
		if (type != ExpansionZoneType.SAFE) return;

		m_Expansion_IsInSafeZone = false;
	}

	bool Expansion_IsTrader()
	{
		return true;
	}
}

#ifdef ENFUSION_AI_PROJECT
/**@class		ExpansionTraderAIBase
 * @brief		This class handles AI traders
 **/
#ifdef EXPANSIONMODAI
class ExpansionTraderAIBase extends eAINPCBase
#else
class ExpansionTraderAIBase extends eAIBase
#endif
{
	private ref ExpansionTraderObjectBase m_TraderObject;

	// ------------------------------------------------------------
	// ExpansionTraderAIBase Constructor
	// ------------------------------------------------------------
	void ExpansionTraderAIBase()
	{
		if (IsMissionHost())
		{
			SetAllowDamage(false);
		}

#ifdef EXPANSIONMODAI
		SetMovementSpeedLimit(1.0);  //! Always walk
#endif

		if (GetGame() && GetGame().IsClient())
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(LoadTrader, 250);
	}
	
	// ------------------------------------------------------------
	// ExpansionTraderAIBase OnRPC
	// ------------------------------------------------------------
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (m_TraderObject)
			m_TraderObject.OnRPC(sender, rpc_type, ctx);
	}

	void SetTraderObject(ExpansionTraderObjectBase traderObject)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "SetTraderObject");
#endif

		m_TraderObject = traderObject;
	}

	ExpansionTraderObjectBase GetTraderObject()
	{
		return m_TraderObject;
	}

	void LoadTrader(string fileName = "")
	{
		SetTraderObject(new ExpansionTraderObjectBase(this, fileName));
	}

#ifdef EXPANSIONMODAI
	override bool PlayerIsEnemy(EntityAI other)
#else
	override bool PlayerIsEnemy(PlayerBase other)
#endif
	{
		return false;
	}

#ifdef EXPANSIONMODAI
	override bool IsTrading()
	{
		return m_TraderObject && m_TraderObject.m_TradingPlayers.Count() > 0;
	}
#endif

	override bool Expansion_IsTrader()
	{
		return true;
	}
}

class ExpansionTraderAIMirek: ExpansionTraderAIBase {};
class ExpansionTraderAIDenis: ExpansionTraderAIBase {};
class ExpansionTraderAIBoris: ExpansionTraderAIBase {};
class ExpansionTraderAICyril: ExpansionTraderAIBase {};
class ExpansionTraderAIElias: ExpansionTraderAIBase {};
class ExpansionTraderAIFrancis: ExpansionTraderAIBase {};
class ExpansionTraderAIGuo: ExpansionTraderAIBase {};
class ExpansionTraderAIHassan: ExpansionTraderAIBase {};
class ExpansionTraderAIIndar: ExpansionTraderAIBase {};
class ExpansionTraderAIJose: ExpansionTraderAIBase {};
class ExpansionTraderAIKaito: ExpansionTraderAIBase {};
class ExpansionTraderAILewis: ExpansionTraderAIBase {};
class ExpansionTraderAIManua: ExpansionTraderAIBase {};
class ExpansionTraderAINiki: ExpansionTraderAIBase {};
class ExpansionTraderAIOliver: ExpansionTraderAIBase {};
class ExpansionTraderAIPeter: ExpansionTraderAIBase {};
class ExpansionTraderAIQuinn: ExpansionTraderAIBase {};
class ExpansionTraderAIRolf: ExpansionTraderAIBase {};
class ExpansionTraderAISeth: ExpansionTraderAIBase {};
class ExpansionTraderAITaiki: ExpansionTraderAIBase {};
class ExpansionTraderAILinda: ExpansionTraderAIBase {};
class ExpansionTraderAIMaria: ExpansionTraderAIBase {};
class ExpansionTraderAIFrida: ExpansionTraderAIBase {};
class ExpansionTraderAIGabi: ExpansionTraderAIBase {};
class ExpansionTraderAIHelga: ExpansionTraderAIBase {};
class ExpansionTraderAIIrena: ExpansionTraderAIBase {};
class ExpansionTraderAIJudy: ExpansionTraderAIBase {};
class ExpansionTraderAIKeiko: ExpansionTraderAIBase {};
class ExpansionTraderAIEva: ExpansionTraderAIBase {};
class ExpansionTraderAINaomi: ExpansionTraderAIBase {};
class ExpansionTraderAIBaty: ExpansionTraderAIBase {};
#endif

class ExpansionTraderMirek: ExpansionTraderNPCBase {};
class ExpansionTraderDenis: ExpansionTraderNPCBase {};
class ExpansionTraderBoris: ExpansionTraderNPCBase {};
class ExpansionTraderCyril: ExpansionTraderNPCBase {};
class ExpansionTraderElias: ExpansionTraderNPCBase {};
class ExpansionTraderFrancis: ExpansionTraderNPCBase {};
class ExpansionTraderGuo: ExpansionTraderNPCBase {};
class ExpansionTraderHassan: ExpansionTraderNPCBase {};
class ExpansionTraderIndar: ExpansionTraderNPCBase {};
class ExpansionTraderJose: ExpansionTraderNPCBase {};
class ExpansionTraderKaito: ExpansionTraderNPCBase {};
class ExpansionTraderLewis: ExpansionTraderNPCBase {};
class ExpansionTraderManua: ExpansionTraderNPCBase {};
class ExpansionTraderNiki: ExpansionTraderNPCBase {};
class ExpansionTraderOliver: ExpansionTraderNPCBase {};
class ExpansionTraderPeter: ExpansionTraderNPCBase {};
class ExpansionTraderQuinn: ExpansionTraderNPCBase {};
class ExpansionTraderRolf: ExpansionTraderNPCBase {};
class ExpansionTraderSeth: ExpansionTraderNPCBase {};
class ExpansionTraderTaiki: ExpansionTraderNPCBase {};
class ExpansionTraderLinda: ExpansionTraderNPCBase {};
class ExpansionTraderMaria: ExpansionTraderNPCBase {};
class ExpansionTraderFrida: ExpansionTraderNPCBase {};
class ExpansionTraderGabi: ExpansionTraderNPCBase {};
class ExpansionTraderHelga: ExpansionTraderNPCBase {};
class ExpansionTraderIrena: ExpansionTraderNPCBase {};
class ExpansionTraderJudy: ExpansionTraderNPCBase {};
class ExpansionTraderKeiko: ExpansionTraderNPCBase {};
class ExpansionTraderEva: ExpansionTraderNPCBase {};
class ExpansionTraderNaomi: ExpansionTraderNPCBase {};
class ExpansionTraderBaty: ExpansionTraderNPCBase {};

class ExpansionTraderZmbM_JournalistSkinny: ExpansionTraderZombieBase {};
