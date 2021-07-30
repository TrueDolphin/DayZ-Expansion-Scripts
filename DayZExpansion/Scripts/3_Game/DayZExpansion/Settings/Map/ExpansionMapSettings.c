/**
 * ExpansionMapSettings.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2020 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionMapSettingsBase
 * @brief		Map settings base class
 **/
class ExpansionMapSettingsBase: ExpansionSettingBase
{
	bool EnableMap;										//! Enable Expansion Map
	bool UseMapOnMapItem;							//! Use Expansion Map On Map Item
	int ShowPlayerPosition;								//! 0 = Player position is not visible | 1 = Player position is visible | 2 = Player position is visible if the player have a compass
	bool ShowMapStats;									//! Show XYZ positions of markers
	bool NeedPenItemForCreateMarker;			//! Require Pen Item to create markers
	bool NeedGPSItemForCreateMarker;			//! Require GPS Item to create markers
	bool CanCreateMarker;								//! Allow player to create markers
	bool CanCreate3DMarker;							//! Allow player to create 3D markers
	bool CanOpenMapWithKeyBinding;				//! Allow player to use a keybind to open the map
	bool ShowDistanceOnPersonalMarkers;		//! Show the distance of personal markers
	bool EnableHUDGPS;									//! Enable HUD GPS (bottom right corner after holding N)
	bool NeedGPSItemForKeyBinding;				//! Require GPS Item to use the keybind
	bool NeedMapItemForKeyBinding;				//! Require Map Item to use the keybind
	bool EnableServerMarkers;							//! Show server markers
	bool ShowNameOnServerMarkers;				//! Show the name of server markers
	bool ShowDistanceOnServerMarkers;			//! Show the distance of server markers
	
	//! WARNING, Do not send over ExpansionMapSettings as a variable, use OnSend.
	//! Failure will result in ServerMarkers incurring a memory leak on the client.
	ref array< ref ExpansionMarkerData > ServerMarkers;
}

/**@class		ExpansionEventCategorys
 * @brief		Map settings class
 **/
class ExpansionMapSettings: ExpansionMapSettingsBase
{
	static const int VERSION = 2;
	
	bool EnableHUDCompass;							//! Allow player to use HUD Compass.
	bool NeedCompassItemForHUDCompass;		//! Requires Compass Item to show the hud compass.
	bool NeedGPSItemForHUDCompass;			//! Requires GPS Item to show the hud compass.		
	int CompassColor;										//! Color of the HUD Compass.
	
	[NonSerialized()]
	private ref map< string, ExpansionMarkerData > ServerMarkersMap;
	
	[NonSerialized()]
	private bool m_IsLoaded;
	
	// ------------------------------------------------------------
	void ExpansionMapSettings()
	{
		#ifdef EXPANSION_MARKER_MODULE_DEBUG
		EXPrint("ExpansionMapSettings::ExpansionMapSettings - Start");
		#endif
		
		ServerMarkers = new array< ref ExpansionMarkerData >;
		ServerMarkersMap = new map< string, ExpansionMarkerData >;
		
		#ifdef EXPANSION_MARKER_MODULE_DEBUG
		EXPrint("ExpansionMapSettings::ExpansionMapSettings - End");
		#endif
	}
	
	// ------------------------------------------------------------
	void ~ExpansionMapSettings()
	{
		delete ServerMarkers;
		delete ServerMarkersMap;
	}
	
	// ------------------------------------------------------------
	bool AddServerMarker(  ExpansionMarkerData marker )
	{
		if ( marker.GetUID() == "" )
		{
			string newUid = "" + Math.RandomInt( 0, int.MAX );
			while ( ServerMarkersMap.Get( newUid ) )
				newUid = "" + Math.RandomInt( 0, int.MAX );
			
			marker.SetUID( newUid );
		} else if ( ServerMarkersMap.Get( marker.GetUID() ) )
			return false;

		ServerMarkers.Insert( marker );
		ServerMarkersMap.Insert( marker.GetUID(), marker );

		return true;
	}
	
	// ------------------------------------------------------------
	bool RemoveServerMarker( string uid )
	{
		#ifdef EXPANSION_MARKER_MODULE_DEBUG
		EXPrint("ExpansionMapSettings::RemoveServerMarker - Start");
		#endif
		
		ExpansionMarkerData server_markerMap = ServerMarkersMap.Get( uid );
		if ( server_markerMap )
		{
			ServerMarkersMap.Remove( uid );
		
			for ( int index = 0; index < ServerMarkers.Count(); ++index )
			{
				ExpansionMarkerData server_marker = ServerMarkers[index];
				if ( server_marker.GetUID() == uid )
				{
					ServerMarkers.Remove( index );
				}
			}
			
			delete server_markerMap;
			
			return true;
		}
		
		return false;
	}

	// ------------------------------------------------------------
	// Client OnReceive
	// ------------------------------------------------------------
	override bool OnRecieve( ParamsReadContext ctx )
	{
		#ifdef EXPANSION_MARKER_MODULE_DEBUG
		EXPrint("ExpansionMapSettings::OnRecieve - Start");
		#endif

		if ( !ctx.Read( EnableMap ) )
			return false;
		if ( !ctx.Read( UseMapOnMapItem ) )
			return false;
		if ( !ctx.Read( ShowPlayerPosition ) )
			return false;
		if ( !ctx.Read( ShowMapStats ) )
			return false;
		if ( !ctx.Read( NeedPenItemForCreateMarker ) )
			return false;
		if ( !ctx.Read( NeedGPSItemForCreateMarker ) )
			return false;
		if ( !ctx.Read( CanCreateMarker ) )
			return false;
		if ( !ctx.Read( CanCreate3DMarker ) )
			return false;
		if ( !ctx.Read( ShowDistanceOnPersonalMarkers ) )
			return false;
		if ( !ctx.Read( CanOpenMapWithKeyBinding ) )
			return false;
		if ( !ctx.Read( EnableHUDGPS ) )
			return false;
		if ( !ctx.Read( NeedGPSItemForKeyBinding ) )
			return false;
		if ( !ctx.Read( NeedMapItemForKeyBinding ) )
			return false;
		if ( !ctx.Read( EnableServerMarkers ) )
			return false;
		if ( !ctx.Read( ShowNameOnServerMarkers ) )
			return false;
		if ( !ctx.Read( ShowDistanceOnServerMarkers ) )
			return false;
		if ( !ctx.Read( EnableHUDCompass ) )
			return false;
		if ( !ctx.Read( NeedCompassItemForHUDCompass ) )
			return false;
		if ( !ctx.Read( NeedGPSItemForHUDCompass ) )
			return false;
		if ( !ctx.Read( CompassColor ) )
			return false;

		int count = 0;
		int index = 0;
		int removeIndex = 0;
		string uid = "";
		
		if ( !ctx.Read( count ) )
			return false;
			
		ExpansionMarkerData marker;
		array< string > checkArr = ServerMarkersMap.GetKeyArray();
		for ( index = 0; index < count; ++index )
		{
			if ( !ctx.Read( uid ) )
				return false;

			removeIndex = checkArr.Find( uid );
			if ( removeIndex != -1 )
				checkArr.Remove( removeIndex );

			marker = ServerMarkersMap.Get( uid );
			if ( !marker )
			{
				marker = ExpansionMarkerData.Create( ExpansionMapMarkerType.SERVER, uid );
				ServerMarkersMap.Insert( uid, marker );
				ServerMarkers.Insert( marker );
			}

			if ( !marker.OnRecieve( ctx ) )
				return false;
		}

		// Remove server markers that have been deleted on server
		for ( index = 0; index < checkArr.Count(); ++index )
		{
			RemoveServerMarker( checkArr[index] );
		}

		m_IsLoaded = true;

		ExpansionSettings.SI_Map.Invoke();
		
		#ifdef EXPANSION_MARKER_MODULE_DEBUG
		EXPrint("ExpansionMapSettings::OnRecieve - End");
		#endif

		return true;
	}
	
	// ------------------------------------------------------------
	override void OnSend( ParamsWriteContext ctx )
	{
		ctx.Write( EnableMap );
		ctx.Write( UseMapOnMapItem );
		ctx.Write( ShowPlayerPosition );
		ctx.Write( ShowMapStats );
		ctx.Write( NeedPenItemForCreateMarker );
		ctx.Write( NeedGPSItemForCreateMarker );
		ctx.Write( CanCreateMarker );
		ctx.Write( CanCreate3DMarker );
		ctx.Write( ShowDistanceOnPersonalMarkers );		
		ctx.Write( CanOpenMapWithKeyBinding );
		ctx.Write( EnableHUDGPS );
		ctx.Write( NeedGPSItemForKeyBinding );
		ctx.Write( NeedMapItemForKeyBinding );
		ctx.Write( EnableServerMarkers );
		ctx.Write( ShowNameOnServerMarkers );
		ctx.Write( ShowDistanceOnServerMarkers );
		ctx.Write( EnableHUDCompass );
		ctx.Write( NeedCompassItemForHUDCompass );
		ctx.Write( NeedGPSItemForHUDCompass );
		ctx.Write( CompassColor );
		
		int count = 0;
		int index = 0;
		string uid = "";
		
		count = ServerMarkers.Count();
		ctx.Write( count );
		for ( index = 0; index < count; ++index )
		{
			ctx.Write( ServerMarkers[index].GetUID() );
			ServerMarkers[index].OnSend( ctx );
		}
	}

	// ------------------------------------------------------------
	override int Send( PlayerIdentity identity )
	{
		#ifdef EXPANSION_MARKER_MODULE_DEBUG
		EXPrint("ExpansionMapSettings::Send - Start");
		#endif
		
		if ( !IsMissionHost() )
		{
			return 0;
		}
		
		ScriptRPC rpc = new ScriptRPC;
		OnSend( rpc );
		rpc.Send( null, ExpansionSettingsRPC.Map, true, identity );
		
		#ifdef EXPANSION_MARKER_MODULE_DEBUG
		EXPrint("ExpansionMapSettings::Send - End and return");
		#endif
		return 0;
	}

	// ------------------------------------------------------------
	override bool Copy( ExpansionSettingBase setting )
	{
		ExpansionMapSettings s;
		if ( !Class.CastTo( s, setting ) )
			return false;

		CopyInternal( s );
		return true;
	}

	// ------------------------------------------------------------
	private void CopyInternal(  ExpansionMapSettings s, bool copyServerMarkers = true )
	{
		EnableHUDCompass = s.EnableHUDCompass;
		NeedCompassItemForHUDCompass = s.NeedCompassItemForHUDCompass;
		NeedGPSItemForHUDCompass = s.NeedGPSItemForHUDCompass;
		CompassColor = s.CompassColor;
		
		ExpansionMapSettingsBase sb = s;
		CopyInternal( sb, copyServerMarkers );
	}
	
	// ------------------------------------------------------------
	private void CopyInternal(  ExpansionMapSettingsBase s, bool copyServerMarkers = true )
	{
		if ( copyServerMarkers )
		{
			ServerMarkers.Clear();
			Print("CopyInternal::s.ServerMarkers"+s.ServerMarkers.Count());

			for (int i = 0; i < s.ServerMarkers.Count(); i++)
			{
				ServerMarkers.Insert( s.ServerMarkers[i] );
			}
		}

		EnableMap = s.EnableMap;
		UseMapOnMapItem = s.UseMapOnMapItem;
		NeedPenItemForCreateMarker = s.NeedPenItemForCreateMarker;
		NeedGPSItemForCreateMarker = s.NeedGPSItemForCreateMarker;
		CanCreateMarker = s.CanCreateMarker;
		ShowPlayerPosition = s.ShowPlayerPosition;
		ShowMapStats = s.ShowMapStats;
		CanCreate3DMarker = s.CanCreate3DMarker;
		ShowDistanceOnPersonalMarkers = s.ShowDistanceOnPersonalMarkers;
		EnableServerMarkers = s.EnableServerMarkers;
		CanOpenMapWithKeyBinding = s.CanOpenMapWithKeyBinding;
		ShowNameOnServerMarkers = s.ShowNameOnServerMarkers;
		ShowDistanceOnServerMarkers = s.ShowDistanceOnServerMarkers;
		EnableHUDGPS = s.EnableHUDGPS;
		NeedGPSItemForKeyBinding = s.NeedGPSItemForKeyBinding;
		NeedMapItemForKeyBinding = s.NeedMapItemForKeyBinding;
	}
	
	// ------------------------------------------------------------
	override bool IsLoaded()
	{
		return m_IsLoaded;
	}

	// ------------------------------------------------------------
	override void Unload()
	{
		m_IsLoaded = false;
	}
	
	// ------------------------------------------------------------
	override bool OnLoad()
	{
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionMapSettings::Load - Start");
		#endif

		m_IsLoaded = true;

		bool save;

		bool mapSettingsExist = FileExist(EXPANSION_MAP_SETTINGS);

		if (mapSettingsExist)
		{
			ExpansionMapSettings settingsDefault = new ExpansionMapSettings;
			settingsDefault.Defaults();

			ExpansionMapSettingsBase settingsBase;

			JsonFileLoader<ExpansionMapSettingsBase>.JsonLoadFile(EXPANSION_MAP_SETTINGS, settingsBase);

			if (settingsBase.m_Version < VERSION)
			{
				if (settingsBase.m_Version < 2)
				{
					EXPrint("[ExpansionMapSettings] Load - Converting v1 \"" + EXPANSION_MAP_SETTINGS + "\" to v" + VERSION);
					
					//! New with v2
					CopyInternal(settingsDefault);
				}
				//! Copy over old settings that haven't changed
				CopyInternal(settingsBase);

				m_Version = VERSION;
				save = true;
			}
			else
			{
				JsonFileLoader<ExpansionMapSettings>.JsonLoadFile(EXPANSION_MAP_SETTINGS, this);
			}

			for (int index = 0; index < ServerMarkers.Count(); ++index)
			{
				if (ServerMarkers[index])
					ServerMarkersMap.Insert(ServerMarkers[index].GetUID(), ServerMarkers[index]);
			}
		}
		else
		{
			Defaults();
			save = true;
		}
		
		if (save)
			Save();
		
		#ifdef EXPANSIONEXPRINT
		EXPrint("ExpansionMapSettings::Load - End - Loaded: " + mapSettingsExist);
		#endif
		
		return mapSettingsExist;
	}

	// ------------------------------------------------------------
	override bool OnSave()
	{
		Print("[ExpansionMapSettings] Saving settings");
		
		JsonFileLoader<ExpansionMapSettings>.JsonSaveFile( EXPANSION_MAP_SETTINGS, this );

		return true;
	}

	// ------------------------------------------------------------
	override void Defaults()
	{
		Print("[ExpansionMapSettings] Loading default settings");
		
		m_Version = VERSION;
		
		EnableMap = true;
		UseMapOnMapItem = true;
		NeedPenItemForCreateMarker = false;
		NeedGPSItemForCreateMarker = false;
		ShowPlayerPosition = 1;
		ShowMapStats = true;
		CanCreateMarker = true;
		CanCreate3DMarker = true;
		ShowDistanceOnPersonalMarkers = true;
		
		EnableServerMarkers = true;

		CanOpenMapWithKeyBinding = true;
		
		ShowNameOnServerMarkers = true;
		ShowDistanceOnServerMarkers = true;
	
		EnableHUDGPS = true;

		NeedGPSItemForKeyBinding = true;
		NeedMapItemForKeyBinding = false;
		
		EnableHUDCompass = true;
		NeedCompassItemForHUDCompass = true;
		NeedGPSItemForHUDCompass = true;
		CompassColor = ARGB(255,255,255,255);
		
	#ifdef EXPANSIONMODMARKET
		string worldName;
		GetGame().GetWorldName(worldName);
		worldName.ToLower();
		
		if (worldName == "chernarusplus" || worldName == "chernarusplusgloom")
		{
			DefaultChernarusMarkers();
		}
	#endif
		
		int index = 0;
		for (index = 0; index < ServerMarkers.Count(); ++index)
		{
			if (ServerMarkers[index])
				ServerMarkersMap.Insert(ServerMarkers[index].GetUID(), ServerMarkers[index]);
		}
	}
	
	void DefaultChernarusMarkers()
	{
		//! Krasnostrav Airsrip Trader
		ExpansionServerMarkerData marker = new ExpansionServerMarkerData("ServerMarker_Trader_Krasno");
		marker.Set3D(true);
		marker.SetName("Traders - Krasnostav Airstrip");
		marker.SetIconName("Trader");
		marker.SetColor(-13710223);
		marker.SetPosition(Vector(11882, 143, 12466));
		marker.SetVisibility(EXPANSION_MARKER_VIS_WORLD | EXPANSION_MARKER_VIS_MAP);
		
		ServerMarkers.Insert(marker);
		
		//! Kamenka Trader
		marker = new ExpansionServerMarkerData("ServerMarker_Trader_Kamenka");
		marker.Set3D(true);
		marker.SetName("Traders - Kamenka");
		marker.SetIconName("Trader");
		marker.SetColor(-13710223);
		marker.SetPosition(Vector(1101, 8, 2382));
		marker.SetVisibility(EXPANSION_MARKER_VIS_WORLD | EXPANSION_MARKER_VIS_MAP);
		
		ServerMarkers.Insert(marker);
		
		//! Boats Kamenka Trader
		marker = new ExpansionServerMarkerData("ServerMarker_Boats_Kamenka");
		marker.Set3D(true);
		marker.SetName("Boats - Kamenka");
		marker.SetIconName("Boat");
		marker.SetColor(-13710223);
		marker.SetPosition(Vector(1756, 4, 2027));
		marker.SetVisibility(EXPANSION_MARKER_VIS_WORLD | EXPANSION_MARKER_VIS_MAP);
		
		ServerMarkers.Insert(marker);
		
		//! Aircrafts Balota Trader
		marker = new ExpansionServerMarkerData("ServerMarker_Aircrafts_Balota");
		marker.Set3D(true);
		marker.SetName("Aircrafts - Balota");
		marker.SetIconName("Helicopter");
		marker.SetColor(-13710223);
		marker.SetPosition(Vector(4973, 12, 2436));
		marker.SetVisibility(EXPANSION_MARKER_VIS_WORLD | EXPANSION_MARKER_VIS_MAP);

		ServerMarkers.Insert(marker);
		
		//! Boats Svetloyarsk Trader
		marker = new ExpansionServerMarkerData("ServerMarker_Boats_Svetloyarsk");
		marker.Set3D(true);
		marker.SetName("Boats & Fishing - Svetloyarsk");
		marker.SetIconName("Boat");
		marker.SetColor(-13710223);
		marker.SetPosition(Vector(14379, 6, 13256));
		marker.SetVisibility(EXPANSION_MARKER_VIS_WORLD | EXPANSION_MARKER_VIS_MAP);

		ServerMarkers.Insert(marker);
		
		//! Green Montain Trader
		marker = new ExpansionServerMarkerData("ServerMarker_Trader_Green_Montain");
		marker.Set3D(true);
		marker.SetName("Trader - Green Montain");
		marker.SetIconName("Trader");
		marker.SetColor(-13710223);
		marker.SetPosition(Vector(3698, 405, 5988));
		marker.SetVisibility(EXPANSION_MARKER_VIS_WORLD | EXPANSION_MARKER_VIS_MAP);

		ServerMarkers.Insert(marker);
	}
	
	// ------------------------------------------------------------
	override string SettingName()
	{
		return "Map Settings";
	}
};