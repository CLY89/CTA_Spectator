modded class SCR_InventoryMenuUI{
	override void OnMenuOpen(){
		super.OnMenuOpen();
		CTA_RespawnSystemComponent respawnSystem = CTA_RespawnSystemComponent.GetInstance();
		if (!respawnSystem)
			return;

		CTA_PlayerLifeManagerComponent playerLifeManager = CTA_PlayerLifeManagerComponent.GetInstance();
		CTA_FactionTicketManagerComponent factionTicketManager = CTA_FactionTicketManagerComponent.GetInstance();
		CTA_RespawnType respawnType = respawnSystem.GetRespawnType();
		if (respawnType == CTA_RespawnType.Life && !playerLifeManager)
		{
			Print("[CTA] Player life manager is not initialized.", LogLevel.ERROR);
			return;
		}

		if (respawnType == CTA_RespawnType.Ticket && !factionTicketManager)
		{
			Print("[CTA] Faction ticket manager is not initialized.", LogLevel.ERROR);
			return;
		}

		string text;

		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;

		int playerId = controller.GetPlayerId();
		if(respawnType == CTA_RespawnType.Life){
			int lives = playerLifeManager.GetLives(playerId);
			if (lives < 0)
				text = "Lives:N/A";
			else
				text = string.Format("Lives:%1/%2",lives,respawnSystem.GetInitialLives());
		}else if(respawnType == CTA_RespawnType.Ticket){
			int tickets = factionTicketManager.GetTicketCountForPlayer(playerId);
			if (tickets < 0)
				text = "Tickets:N/A";
			else
				text = string.Format("Tickets:%1",tickets);
		}else{
			text = "";
		}
		TextWidget wRespawn = TextWidget.Cast(m_widget.FindAnyWidget("Respawn"));
		if (wRespawn)
			wRespawn.SetText(text);
		return;

	}
}