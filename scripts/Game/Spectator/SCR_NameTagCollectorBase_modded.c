[BaseContainerProps()]
modded class SCR_NameTagCollectorBase{
	override void RefreshEntities()
	{
		if (m_TagSystem)
		{
			vector pos;
			CTA_Spectator spectator = CTA_Spectator.GetInstance();
			if(spectator && spectator.isEnable()){
				pos = spectator.GetCamera().GetOrigin();
			} else{
				pos = m_ControlledTag.m_Entity.GetOrigin();
				
			}
			m_TagSystem.GetTagsInRange(m_aEntities, pos, m_Config.m_fFarthestZoneRange, ETagCategory.NameTag);
		}
	}

}