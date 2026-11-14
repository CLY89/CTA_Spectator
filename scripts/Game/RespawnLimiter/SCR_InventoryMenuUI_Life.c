modded class SCR_InventoryMenuUI{
	override void OnMenuOpen(){
		super.OnMenuOpen();
		CTA_RespawnLimiterComponent respawnLimiter = CTA_RespawnLimiterComponent.GetInstance();
		CTA_RespawnLimitType limitType = respawnLimiter.GetRespawnLimitType();
		string text;

		if(limitType == CTA_RespawnLimitType.Individual){
			int death = respawnLimiter.GetDeathSelf();
			int limit = respawnLimiter.GetRespawnLimit();
			text = string.Format("Respawns:%1/%2",death,limit);
		}else if(limitType == CTA_RespawnLimitType.Ticket){
			int death = respawnLimiter.GetDeathTotal();
			int limit = respawnLimiter.GetRespawnLimit();
			text = string.Format("Tickets:%1",limit - death);
		}else{
			text = "";
		}
		TextWidget wRespawn = TextWidget.Cast(m_widget.FindAnyWidget("Respawn"));
		wRespawn.SetText(text);
		return;

	}
}