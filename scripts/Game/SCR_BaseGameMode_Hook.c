modded class SCR_BaseGameMode{
	protected ref CTA_Spectator m_CTA_Spectator;
	
	override void EOnInit(IEntity owner){
		super.EOnInit(owner);
		m_CTA_Spectator = new CTA_Spectator();
	}
}