/**
 * ExpansionBookMenuTabParty.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

#ifdef EXPANSIONMODGROUPS
class ExpansionBookMenuTabParty: ExpansionBookMenuTabBase
{
	ref ExpansionPartyModule m_PartyModule;
	ref ExpansionDialog_CreateParty m_CreatePartyDialog;
	ref ExpansionBookMenuTabPartyController m_PartyTabController;
	ref ExpansionPartyData m_Party;
	ref ExpansionDialog_DeleteParty m_DeleteDialog;
	ref ExpansionDialog_LeaveParty m_LeaveDialog;
	ref ExpansionDialog_InviteParty m_InviteDialog;
	ref ExpansionBookMenuTabPartyMemberEdit m_PartyTabMemberEdit;
	
	private ButtonWidget party_delete_button;
	private TextWidget party_delete_button_label;
	
	private EditBoxWidget player_list_filter_editbox;
	private ButtonWidget player_list_filter_clear;
	private ImageWidget player_list_filter_clear_icon;
	
	private Widget party_delete_content;
	private Widget party_leave_content;
	private Widget book_menu_content_left;
		
	private int m_PlayerSearchRadius = 25;
	ref SyncPlayer m_Player;
	
	void ExpansionBookMenuTabParty(ExpansionBookMenu book_menu)
	{
		m_BookMenu = book_menu;
		
		if (!m_PartyModule)
			m_PartyModule = ExpansionPartyModule.Cast(CF_ModuleCoreManager.Get(ExpansionPartyModule));
		
		if (!m_PartyTabController)
			m_PartyTabController = ExpansionBookMenuTabPartyController.Cast(GetController());
		
		RegisterPartyTabs();
	}
	
	void ~ExpansionBookMenuTabParty()
	{
		m_Party = NULL;
	}
	
	void RegisterPartyTabs()
	{
		if (!m_PartyTabMemberEdit)
		{
			m_PartyTabMemberEdit = new ExpansionBookMenuTabPartyMemberEdit(m_BookMenu);
			m_PartyTabMemberEdit.SetParentTab(this);
			m_BookMenu.RegisterTab(m_PartyTabMemberEdit);
		}
	}
	
	ExpansionBookMenuTabPartyMemberEdit GetMemberEditTab()
	{
		return m_PartyTabMemberEdit;
	}
	
	void UnregisterPartyTabs()
	{
		if (m_PartyTabMemberEdit)
		{
			m_BookMenu.UnregisterTab(m_PartyTabMemberEdit);
			delete m_PartyTabMemberEdit;
		}
	}
	
	void GetParty()
	{
		m_Party = m_PartyModule.GetParty();
	}
	
	void SetView()
	{
		if (!m_Party)
			return;
		
		m_PartyTabController.PartyName = m_Party.GetPartyName() + " #STR_EXPANSION_BOOK_MEMBERS";
		m_PartyTabController.NotifyPropertyChanged("PartyName");
	
		#ifdef EXPANSIONMODGROUPS_DEBUG
		EXLogPrint("SetView - CanDelete(): " + GetPlayerPartyData().CanDelete());
		EXLogPrint("SetView - CanInvite(): " + GetPlayerPartyData().CanInvite());
		EXLogPrint("SetView - CanKick(): " + GetPlayerPartyData().CanKick());
		EXLogPrint("SetView - CanEdit(): " + GetPlayerPartyData().CanEdit());
		#ifdef EXPANSIONMODMARKET
		EXLogPrint("SetView - CanWithdrawMoney(): " + GetPlayerPartyData().CanWithdrawMoney());
		#endif
		#endif	
		
		if (GetPlayerPartyData().CanDelete())
		{
			party_delete_content.Show(true);
		}
		else
		{
			party_delete_content.Show(false);
		}
		
		if (GetPlayerPartyData().GetID() == m_Party.GetOwnerUID())
		{
			party_leave_content.Show(false);
		}
		else
		{
			party_leave_content.Show(true);			
		}
		
		if (GetPlayerPartyData().CanInvite())
		{
			book_menu_content_left.Show(true);
		}
		else
		{
			book_menu_content_left.Show(false);
		}
	}
	
	void LoadMembers()
	{
		ClearMermbers();
		
		if (m_Party && m_Party.GetPlayers().Count() > 0)
		{
			array<ref ExpansionPartyPlayerData> members = m_Party.GetPlayers();
			for (int i = 0; i < members.Count(); ++i)
			{
				ExpansionPartyPlayerData memberData = members[i];
				ExpansionPartyPlayerData playerData = m_Party.GetPlayer(GetGame().GetPlayer().GetIdentity().GetId());
				
				//! Dont add a entry for the player itself, only his party members
				if (memberData.GetID() == playerData.GetID())
					continue;
				
				ExpansionBookMenuTabPartyMemberEntry entry;
				bool hasEntry = false;
				if (m_PartyTabController.PartyMemberEntrys.Count() > 0)
				{
					for (int e = 0; e < m_PartyTabController.PartyMemberEntrys.Count(); ++e)
					{
						ExpansionBookMenuTabPartyMemberEntry existingEntry = m_PartyTabController.PartyMemberEntrys[e];
						
						if (existingEntry.m_Member.GetID() == memberData.GetID())
							hasEntry = true;
						
						//! If the entry is related to a old member that is no longer the party then remove the entry
						if (members.Find(existingEntry.m_Member) == -1)
							m_PartyTabController.PartyMemberEntrys.Remove(e);
					}
				}
				//! If the party member has a entry already then skip the entry creation
				if (!hasEntry)
				{
					entry = new ExpansionBookMenuTabPartyMemberEntry(this, memberData);
					m_PartyTabController.PartyMemberEntrys.Insert(entry);
				}
			}
		}
	}
	
	void LoadPlayers(string filter)
	{
		if (!m_Party) 
			return;
		
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player) 
			return;
	
		ClearPlayers();
		
		string filterNormal = filter;
		filter.ToLower();
			
		int nmbPlayer = 0;
		ExpansionBookMenuTabPartyPlayerEntry entry;
		string playerName;
		
		set<ref SyncPlayer> players;
		if (GetExpansionSettings().GetParty().UseWholeMapForInviteList)
		{
			players = SyncPlayer.Expansion_GetAll();
		}
		else
		{
			players = SyncPlayer.Expansion_GetInSphere(player.GetPosition(), m_PlayerSearchRadius);
		}

		foreach (SyncPlayer playerSync: players)
		{
			if (playerSync.m_RUID == player.GetIdentity().GetId())
				continue;

			if (IsMember(playerSync.m_RUID) || HasInvite(playerSync.m_RUID))
			{
				continue;
			}

			if (filter != "")
			{
				playerName = playerSync.m_PlayerName;
				playerName.ToLower();
	
				if (playerName.IndexOf(filter) == -1)
					continue;
			}

			nmbPlayer++;

			AddPlayerEntry(playerSync);
		}
	}
	
	void AddPlayerEntry(SyncPlayer playerSync)
	{
		bool isInList = false;
		for (int i = 0; i < m_PartyTabController.PartyPlayerEntrys.Count(); ++i)
		{
			ExpansionBookMenuTabPartyPlayerEntry entry = m_PartyTabController.PartyPlayerEntrys[i];
			if (entry.m_Player.m_RUID == playerSync.m_RUID)
			{
				isInList = true;
				break;
			}
		}
		
		if (!isInList)
		{
			ExpansionBookMenuTabPartyPlayerEntry newEntry = new ExpansionBookMenuTabPartyPlayerEntry(this, playerSync);
			m_PartyTabController.PartyPlayerEntrys.Insert(newEntry);
		}
	}
	
	void ClearPlayers()
	{
		for (int i = 0; i < m_PartyTabController.PartyPlayerEntrys.Count(); ++i)
		{
			ExpansionBookMenuTabPartyPlayerEntry entry = m_PartyTabController.PartyPlayerEntrys[i];
			entry.Hide();
			entry = NULL;
		}
		
		m_PartyTabController.PartyPlayerEntrys.Clear();
	}
	
	void ClearMermbers()
	{
		for (int i = 0; i < m_PartyTabController.PartyMemberEntrys.Count(); ++i)
		{
			ExpansionBookMenuTabPartyMemberEntry entry = m_PartyTabController.PartyMemberEntrys[i];
			entry.Hide();
			entry = NULL;
		}

		m_PartyTabController.PartyMemberEntrys.Clear();
	}
	
	void InvitePlayer(SyncPlayer player)
	{
		m_Player = player;
		
		if (!m_InviteDialog)
			m_InviteDialog = new ExpansionDialog_InviteParty(this);
		
		m_InviteDialog.Show();
	}
	
	SyncPlayer GetSelectedPlayer()
	{
		return m_Player;
	}
	
	bool IsMember(string uid)
	{
		return m_Party.IsMember(uid);
	}
	
	bool HasInvite(string uid)
	{
		if (m_Party && m_Party.GetPartyInvites().Count() > 0)
		{
			foreach (ExpansionPartyInviteData invite: m_Party.GetPartyInvites())
			{
				if (invite.UID == uid)
					return true;
			}
		}
		
		return false;
	}
	
	ExpansionPartyPlayerData GetPlayerPartyData()
	{
		return m_Party.GetPlayer(GetGame().GetPlayer().GetIdentity().GetId());
	}
	
	void OnDeleteButtonClick()
	{
		if (GetPlayerPartyData().CanDelete())
		{
			if (!m_DeleteDialog)
				m_DeleteDialog = new ExpansionDialog_DeleteParty(this);
				
			m_DeleteDialog.Show();
		}
	}
	
	void OnLeaveButtonClick()
	{
		if (!m_LeaveDialog)
			m_LeaveDialog = new ExpansionDialog_LeaveParty(this);
		
		m_LeaveDialog.Show();
	}
	
	void OnRefreshButtonClick()
	{
		m_PartyTabController.SearchFilter = "#STR_USRACT_SEARCH";
		m_PartyTabController.NotifyPropertyChanged("SearchFilter");
		
		if (GetPlayerPartyData().CanInvite())
			LoadPlayers("");
	}
	
	override string GetLayoutFile() 
	{
		return "DayZExpansion/Book/GUI/layouts/tabs/party/expansion_book_tab_party.layout";
	}
	
	override typename GetControllerType() 
	{
		return ExpansionBookMenuTabPartyController;
	}
	
	override string GetTabIconName()
	{
		return "Group";
	}
	
	override string GetTabName()
	{
		return "#STR_EXPANSION_BOOK_TAB_PARTY_MANAGER";
	}
	
	override int GetTabColor()
	{
		return ARGB(255,0,0,0);
	}
	
	override bool CanShow()
	{
		return GetExpansionSettings().GetBook().EnablePartyTab;
	}
	
	override void OnShow()
	{
		super.OnShow();
		
		if (!m_PartyModule.HasParty())
		{
			if (!m_CreatePartyDialog)
				m_CreatePartyDialog = new ExpansionDialog_CreateParty(this);
			
			m_CreatePartyDialog.Show();
			GetLayoutRoot().Show(false);
		}
		else
		{
			GetParty();
			SetView();
			
			LoadMembers();
			
			if (GetPlayerPartyData().CanInvite())
				LoadPlayers("");
		}
		
		SwitchMovementLockState(true);
	}
	
	override void OnHide()
	{
		super.OnHide();
		
		if (m_CreatePartyDialog && m_CreatePartyDialog.IsVisible())
			m_CreatePartyDialog.Hide();
		
		if (m_InviteDialog && m_InviteDialog.IsVisible())
			m_InviteDialog.Hide();
		
		if (m_DeleteDialog && m_DeleteDialog.IsVisible())
			m_DeleteDialog.Hide();
		
		SwitchMovementLockState(false);
		
		m_Party = NULL;
	}
	
	override bool IsParentTab()
	{
		return true;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		switch (w)
		{
		case party_delete_button:
			party_delete_button_label.SetColor(ARGB(255, 220, 220, 220));
			break;
		case player_list_filter_clear:
			player_list_filter_clear_icon.SetColor(ARGB(255, 220, 220, 220));
			break;
		}
		
		return super.OnMouseEnter(w, x, y);;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		switch (w)
		{
		case party_delete_button:
			party_delete_button_label.SetColor(ARGB(255, 0, 0, 0));
			break;
		case player_list_filter_clear:
			player_list_filter_clear_icon.SetColor(ARGB(255, 0, 0, 0));
			break;
		}
		
		return super.OnMouseLeave(w, enterW, x, y);
	}
	
	override void OnBackButtonClick()
	{
		super.OnBackButtonClick();
		
		Hide();
	}
	
	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (w == player_list_filter_editbox)
		{
			if (GetPlayerPartyData().CanInvite())
			{
				LoadPlayers(player_list_filter_editbox.GetText());
				return true;
			}
		}
		
		return false;
	}
	
	override void Refresh()
	{
		if (!m_PartyModule.HasParty())
		{
			m_Party = NULL;
			Hide();
		}
		else
		{
			GetParty();
			SetView();
			
			LoadMembers();
			
			if (GetPlayerPartyData().CanInvite())
				LoadPlayers("");
		}
	}
};

class ExpansionBookMenuTabPartyController: ExpansionViewController 
{
	ref ObservableCollection<ref ExpansionBookMenuTabPartyMemberEntry> PartyMemberEntrys = new ObservableCollection<ref ExpansionBookMenuTabPartyMemberEntry>(this);
	ref ObservableCollection<ref ExpansionBookMenuTabPartyPlayerEntry> PartyPlayerEntrys  = new ObservableCollection<ref ExpansionBookMenuTabPartyPlayerEntry>(this);
	string PartyName;
	string SearchFilter;
};
#endif
