class CTA_SpectatorUI : EditorMenuUI
{
	protected static const string TEXT_TITLE = "TextTitle";
	protected static const string BUTTON_CLOSE = "ButtonClose";
	protected ref CTA_SpectatorPlayerListView m_playerListView;
	protected CTA_PlayerLifeManagerComponent m_playerLifeManager;
	protected ref ScriptInvoker<int> m_OnPlayerLivesChanged;
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		Widget rootWidget = GetRootWidget();
		if (!rootWidget)
		{
			Print("Error in Layout Tutorial layout creation", LogLevel.ERROR);
			return;
		}
		
		SCR_ButtonTextComponent buttonClose = SCR_ButtonTextComponent.GetButtonText(BUTTON_CLOSE, rootWidget);
		if (buttonClose)
		{
			buttonClose.m_OnClicked.Insert(Close);
			buttonClose.SetText("Close");
		}else{
			Print("Button Close not found - won't be able to exit by button", LogLevel.WARNING);
		}
		
		TextWidget textTitle = TextWidget.Cast(rootWidget.FindWidget(TEXT_TITLE));
		textTitle.SetText("Spectator");
		
		/*
			ESC/Start listener
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			// this is for the menu/dialog to close when pressing ESC
			// an alternative is to have a button with the SCR_NavigationButtonComponent component
			// and its Action Name field set to MenuBack - this would activate the button on ESC press
			inputManager.AddActionListener("MenuOpen", EActionTrigger.DOWN, Close);
			inputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, Close);
		}
		else if (!buttonClose)
		{
			Print("Auto-closing the menu that has no exit path", LogLevel.WARNING);
			Close();
			return;
		}
		*/
		
		//PlayerList
		
		m_playerListView = new CTA_SpectatorPlayerListView(GetRootWidget().FindAnyWidget("PlayerList"));
		m_playerLifeManager = CTA_PlayerLifeManagerComponent.GetInstance();
		if (!m_playerLifeManager)
		{
			Print("[CTA_Spectator] Player life manager was not found.", LogLevel.ERROR);
			return;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
		{
			Print("[CTA_Spectator] Player manager was not found.", LogLevel.ERROR);
			return;
		}

		array<int> ids = {};
		playerManager.GetPlayers(ids);

		m_OnPlayerLivesChanged = m_playerLifeManager.GetOnPlayerLivesChanged();
		m_OnPlayerLivesChanged.Insert(OnPlayerLivesChangedHandler);

		foreach (int id : ids)
			RefreshPlayerEntry(id);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if(!gameMode){
			PrintFormat("[CTA_Spectator] Failed to downcast from BaseGameMode to SCR_BaseGameMode.");
			return;
		}
		gameMode.GetOnPlayerConnected().Insert(OnPlayerConnectedHandler);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnectedHandler);
		m_playerListView.GetOnPlayerButtonClicked().Insert(OnPlayerButtonClickedHandler);
		
	}

	protected void OnPlayerButtonClickedHandler(int id){
		CTA_Spectator.GetInstance().WatchPlayer(id);
	}
		
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuClose()
	{
		// here we clean action listeners added above as the good practice wants it
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("MenuOpen", EActionTrigger.DOWN, Close);
			inputManager.RemoveActionListener("MenuBack", EActionTrigger.DOWN, Close);
		}
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if(!gameMode){
			PrintFormat("[CTA_Spectator] Failed to downcast from BaseGameMode to SCR_BaseGameMode.");
		}else{
			gameMode.GetOnPlayerConnected().Remove(OnPlayerConnectedHandler);
			gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnectedHandler);
		}

		if (m_OnPlayerLivesChanged)
		{
			m_OnPlayerLivesChanged.Remove(OnPlayerLivesChangedHandler);
			m_OnPlayerLivesChanged = null;
		}
		
		CTA_Spectator.GetInstance().Disable();
	}
	
	void OnPlayerConnectedHandler(int playerId){
		RefreshPlayerEntry(playerId);
	}
	
	void OnPlayerDisconnectedHandler(int playerId, KickCauseCode cause = KickCauseCode.NONE, int timeout = -1){
		m_playerListView.RemovePlayerEntry(playerId);
	}

	protected void OnPlayerLivesChangedHandler(int playerId)
	{
		RefreshPlayerEntry(playerId);
	}

	protected void RefreshPlayerEntry(int playerId)
	{
		if (m_playerLifeManager.GetLives(playerId) < 1)
		{
			m_playerListView.RemovePlayerEntry(playerId);
			return;
		}

		string name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId);
		m_playerListView.AddPlayerEntry(playerId, name);
	}
}
