class CTA_SpectatorUI : EditorMenuUI
{
	protected static const string TEXT_TITLE = "TextTitle";
	protected static const string BUTTON_CLOSE = "ButtonClose";
	protected ref CTA_SpectatorPlayerListView m_playerListView;
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
		CTA_AlivePlayerComponent alivePlayer = CTA_AlivePlayerComponent.GetInstance();
		array<int> ids = {};
		alivePlayer.GetAlivePlayer(ids);

		foreach (int id : ids)
		{
			string name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(id);
			m_playerListView.AddPlayerEntry(id,name);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if(!gameMode){
			PrintFormat("[CTA_Spectator] Failed to downcast from BaseGameMode to SCR_BaseGameMode.");
		}
		// Model -> View 
		alivePlayer.GetOnAlivePlayerAdded().Insert(OnAlivePlayerAddedHandler);
		alivePlayer.GetOnAlivePlayerRemoved().Insert(OnAlivePlayerRemoved);
		alivePlayer.GetOnAlivePlayerReset().Insert(OnAlivePlayerReset);
		// View -> Model
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
		}
		
		CTA_Spectator.GetInstance().Disable();
	}
	
	void OnAlivePlayerAddedHandler(int playerId){
		string name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId);
		m_playerListView.AddPlayerEntry(playerId,name);
	}
	
	void OnAlivePlayerRemoved(int playerId){
		m_playerListView.RemovePlayerEntry(playerId);
	}
	
	void OnAlivePlayerReset(array<int> ids){
		m_playerListView.ClearPlayerList();
		foreach(int id :ids){
			string name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(id);
			m_playerListView.AddPlayerEntry(id,name);
		}
	}
}
