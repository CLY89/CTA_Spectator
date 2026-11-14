class CTA_EnableSpectatorAction: ScriptedUserAction{		
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CTA_Spectator spectator = CTA_Spectator.GetInstance();
		spectator.Enable();
	}
	override bool HasLocalEffectOnlyScript() { return true; };

}