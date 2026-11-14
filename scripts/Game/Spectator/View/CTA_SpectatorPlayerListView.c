class CTA_SpectatorPlayerListView{
	protected ResourceName m_sPlayerListEntry = "{6959FD2516FDE200}UI/layouts/CTA_PlayerListEntry.layout";
	protected Widget m_wPlayerListWidget;
	protected ref map<int,Widget> m_wPlayerEntryWidget = new map<int,Widget>();
	ref ScriptInvoker<int> m_OnPlayerEntrySelected = new ScriptInvoker();
	protected int m_iLastButtonID = -1;
	
	
	void CTA_SpectatorPlayerListView(Widget playerListWidget){
		m_wPlayerListWidget = playerListWidget;
	}
	
	void ClearPlayerList(){
		foreach(int id,Widget w:m_wPlayerEntryWidget){
			RemovePlayerEntry(id);
		}
	}
	
	void AddPlayerEntry(int id,string name){
		if(m_wPlayerEntryWidget.Contains(id)){
			return;
		}
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sPlayerListEntry,m_wPlayerListWidget);
		TextWidget wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		wName.SetText(name);	
		m_wPlayerEntryWidget.Set(id,w);
		SCR_ButtonBaseComponent button = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));

		button.m_OnFocus.Insert(OnFocusHandler);
		button.m_OnClicked.Insert(OnButtonClickedHandler);
	}
	
	void RemovePlayerEntry(int id){
		if(!m_wPlayerEntryWidget.Contains(id)){
			return;
		}
		Widget w = m_wPlayerEntryWidget.Get(id);
		w.RemoveFromHierarchy();
		m_wPlayerEntryWidget.Remove(id);
	}
	
	ScriptInvoker<int> GetOnPlayerButtonClicked(){
		return m_OnPlayerEntrySelected;
	}
	protected void OnButtonClickedHandler(){
		m_OnPlayerEntrySelected.Invoke(m_iLastButtonID);
	}
	protected void OnFocusHandler(Widget widget){		
		m_iLastButtonID = SCR_MapHelper<int,Widget>.GetKeyByValue(m_wPlayerEntryWidget,widget);
	}
	 
}