modded class SCR_BaseGameMode{
	protected ref CTA_Spectator m_CTA_Spectator;
	
	override void EOnInit(IEntity owner){
		super.EOnInit(owner);
		m_CTA_Spectator = new CTA_Spectator();
	}
	/*
	void OnPlayerSpawnedHandler(int playerId, IEntity player){
		SCR_ScoreInfo score = GetScoringSystemComponent().GetPlayerScoreInfo(playerId);
		if(score.m_iDeaths <= m_CTA_RespawnLimiter.GetRespawnLimit()) {
			return;
		}
		Rpc(RPC_Hoge,playerId);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_Hoge(int playerId){
		if(GetGame().GetPlayerController().GetID() != playerId){
			return;
		}
		m_CTA_Spectator.Enable();
	}
	*/
}