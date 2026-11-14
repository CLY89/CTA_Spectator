[BaseContainerProps()]
modded class SCR_NameTagRulesetBase{
	
	override bool TestVisibility(SCR_NameTagData data, float timeSlice)
	{
		// current players tag, update tag specific vars and return
		if (data == m_CurrentPlayerTag)
		{
			data.UpdateTagPos();
			return false;
		}

		// out of zone range
		data.m_fTimeSliceUpdate += timeSlice;
		
		if (data.m_fTimeSliceUpdate >= POSITION_UPDATE) // avoid updating every frame
		{
			data.m_fTimeSliceUpdate = 0;
			// Modded: Use camera position if spectator enable
			CTA_Spectator spectator = CTA_Spectator.GetInstance();
			vector pos;
			if(spectator && spectator.isEnable()){
				pos = spectator.GetCamera().GetOrigin();
			} else {
				pos = m_CurrentPlayerTag.m_vEntWorldPos;
			}
			data.m_fDistance = vector.DistanceSq(pos, data.m_Entity.GetOrigin()); // checked through origin here to avoid more expensive pos check bottom
		}
		
		bool foundZone;
		int distMin;
		int distMax;
		foreach (SCR_NameTagZone zone : m_ZoneCfg.m_aZones)
		{
			if (zone.GetZoneName() == ZONE_SCOPED)
			{
				if ((!SCR_2DPIPSightsComponent.IsPIPActive() && SCR_PlayerController.s_pLocalPlayerController && !SCR_PlayerController.s_pLocalPlayerController.GetIsBinocularsZoomed() && !SCR_2DSightsComponent.IsUsingSights()) || m_CurrentPlayerTag.m_CharController.IsFreeLookEnabled())
					continue;
			}
			
			distMin = zone.m_iZoneStartSq;
			distMax = zone.m_iZoneEndSq;
			
			if (data.m_fDistance < distMax && data.m_fDistance > distMin)
			{
				data.m_fTimeSliceCleanup = 0;
				foundZone = true;
				break;
			}
		}
		
		if (!foundZone)
		{
			// cleanup tags which are more than distance + 10m (10^2) away from base zone query 
			if (data.m_fDistance >= m_ZoneCfg.m_fFarthestZoneRangePow2 + 100)
				data.m_fTimeSliceCleanup += timeSlice;			
			
			return false;
		}
		
		// vehicle visibility 
		if (data.m_Flags & ENameTagFlags.VEHICLE)	// in vehicle
		{
			if (!data.m_VehicleParent.m_bIsControlledPresent)	// dont show vehicle crew tags until inside the same vehicle
				return false;
			else if (m_CurrentPlayerTag.m_CharController.IsInThirdPersonView())	// dont show vehicle crew tags if in third person
				return false;
		}
		
		if (data.m_eType == ENameTagEntityType.VEHICLE)	// is vehicle
		{	
			if (data.m_Flags & ENameTagFlags.VEHICLE_DISABLE || SCR_VehicleTagData.Cast(data).m_bIsControlledPresent)
				return false;
		}
								
		// update tag position
		if (data.m_eAttachedTo == data.m_eAttachedToLast)
			data.UpdateTagPos();
		else
		{
			data.m_fTimeSlicePosChange += timeSlice * POS_TRANSITION_MULT;
			if ( data.m_fTimeSlicePosChange >= 1.0 )
			{
				data.UpdateTagPos();
				data.m_eAttachedToLast = data.m_eAttachedTo;
			}
			else 
			{
				data.UpdateTagPos();
				data.m_vTagWorldPos = vector.Lerp( data.m_vTagWorldPosLast, data.m_vTagWorldPos, data.m_fTimeSlicePosChange );
			}
		}
					
		CalculateScreenPos(data);
		if (data.m_vTagScreenPos[2] < 0 || m_iRefResolutionY * 1.1 < data.m_vTagScreenPos[1] || m_iRefResolutionX * 1.1 < data.m_vTagScreenPos[0] || 0 > data.m_vTagScreenPos[1] || 0 > data.m_vTagScreenPos[0])
			return false;	
			
		return true;	
	} 
}