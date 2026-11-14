class CTA_NameTagZoneCache{
	int m_iStart = 0;
	int m_iEnd = 0;
	
	void CTA_NameTagZoneCache(int start,int end){
		m_iStart = start;
		m_iEnd = end;
	}

}
class CTA_NameTagZoneReplacer{	
	protected ref CTA_NameTagZoneCache m_zoneDataCacheNear;
	protected ref CTA_NameTagZoneCache m_zoneDataCacheFar;
	
	protected int m_iNearEnd;
	protected int m_iFarStart;
	protected int m_iFarEnd;
	
	void CTA_NameTagZoneReplacer(int nearEnd,int farStart,int farEnd){
		m_iNearEnd = nearEnd;
		m_iFarStart = farStart;
		m_iFarEnd = farEnd;
	}
	
	void Replace(){
		array<ref SCR_NameTagZone> nameTagZones;
		if(!GetZone(nameTagZones)){
			return;
		};
		
		foreach(SCR_NameTagZone zone : nameTagZones){
			if(zone.GetZoneName() == "Near"){
				m_zoneDataCacheNear = zone.CreateCache();
				zone.Update(0,150);
			}
			if(zone.GetZoneName() == "Far"){
				m_zoneDataCacheFar = zone.CreateCache();
				zone.Update(150,500);
			}
		}	
	}
	
	void Restore(){
		if(!m_zoneDataCacheNear || !m_zoneDataCacheFar){
			Print("[CTA] NameTagZone is not cached");
			return;
		}
		
		array<ref SCR_NameTagZone> nameTagZones;
		if(!GetZone(nameTagZones)){
			return;
		};
		foreach(SCR_NameTagZone zone : nameTagZones){
			if(zone.GetZoneName() == "Near"){
				zone.Update(m_zoneDataCacheNear.m_iStart,m_zoneDataCacheNear.m_iEnd);
			}
			if(zone.GetZoneName() == "Far"){
				zone.Update(m_zoneDataCacheFar.m_iStart,m_zoneDataCacheFar.m_iEnd);
			}
		}	
	}
	
	protected bool GetZone(out array<ref SCR_NameTagZone> nameTagZones){
		nameTagZones = SCR_NameTagDisplay.GetNametagZones();
		if(!nameTagZones){
			Print("[CTA] Failed to GetNametagZones");
			return false;
		}
		return true;
	}
}
class CTA_CameraManager{
	protected static ResourceName m_sManualCaemra = "{10AE54157CB61E7D}Prefabs/Editor/Camera/ManualCameraBase_Test.et";
	protected IEntity m_eSpawnedManualCamera;
	
	IEntity GetCamera(){
		return m_eSpawnedManualCamera;
	}
	
	bool DeleteCamera(){
		if(m_eSpawnedManualCamera){
			delete m_eSpawnedManualCamera;
			return true;
		}
		return false;
	}
	
	bool SpawnCamera(){
		if(m_eSpawnedManualCamera != null){
			return false;
		}
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
	
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;
		vector transform[4];
		world.GetCurrentCamera(transform);
		
		spawnParams.Transform = transform;
		
		Resource resource = Resource.Load(m_sManualCaemra);
		if(resource && resource.IsValid()){
			m_eSpawnedManualCamera = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		}
		return true;
	}
	
	void WatchPlayer(int playerID){
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		MoveToBehind(m_eSpawnedManualCamera,player);
		LookAt(m_eSpawnedManualCamera,player);
		m_eSpawnedManualCamera.Update();
	}
	
	void LookAt(IEntity ent,IEntity tag){
		vector mat[4];
		SCR_Math3D.LookAt(ent.GetOrigin(),tag.GetOrigin(),vector.Up,mat);
		mat[3] = ent.GetOrigin();
		ent.SetTransform(mat);
	}
	
	void MoveToBehind(IEntity ent,IEntity tag){
		vector backward = -10 * tag.GetLocalTransformAxis(2);
		vector up = 5 * tag.GetLocalTransformAxis(1);
		vector posEnt = tag.GetOrigin() + up + backward;
		ent.SetOrigin(posEnt);
	}
	
}
class CTA_Spectator{
	protected static ref CTA_Spectator s_Instance;
	protected ref CTA_NameTagZoneReplacer m_nameTagReplacer;
	protected ref CTA_CameraManager m_cameraManager;
	
	static CTA_Spectator GetInstance(){
		return s_Instance;
	}
	
	void CTA_Spectator(){
		s_Instance = this;
		m_nameTagReplacer = new CTA_NameTagZoneReplacer(100,100,500);
		m_cameraManager = new CTA_CameraManager();
	}
	
	void Enable(){
		if(!m_cameraManager.SpawnCamera()){
			return;
		}
		m_nameTagReplacer.Replace();
		m_cameraManager.WatchPlayer(GetGame().GetPlayerController().GetPlayerId());
		OpenSpectatorUI();

	}
	
	void Disable(){
		m_nameTagReplacer.Restore();
		m_cameraManager.DeleteCamera();
	}
	
	bool isEnable(){
		return m_cameraManager.GetCamera();
	}
	
	IEntity GetCamera(){
		return m_cameraManager.GetCamera();
	}

	bool OpenSpectatorUI(){
		MenuManager menuMngr = GetGame().GetMenuManager();
		if(menuMngr.FindMenuByPreset(ChimeraMenuPreset.CTA_SpectatorMenu)){
			return false;
		}
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CTA_SpectatorMenu);	
		return true;
	}
	
	void WatchPlayer(int playerId){
		m_cameraManager.WatchPlayer(playerId);
	}
}

[BaseContainerProps(), SCR_NameTagZoneTitle()]
modded class SCR_NameTagZone{
	CTA_NameTagZoneCache CreateCache(){
		return new CTA_NameTagZoneCache(m_iZoneStart,m_iZoneEnd);
	}
	
	void Update(int start,int end){
		m_iZoneStart = start;
		m_iZoneEnd = end;
		m_iZoneStartSq = m_iZoneStart * m_iZoneStart;
		m_iZoneEndSq = m_iZoneEnd * m_iZoneEnd;
	}

}

/* old
modded class SCR_NameTagDisplay{
	void SetNameTagConfig(SCR_NameTagConfig newCfg){
		s_NametagCfg = newCfg;
	}
	SCR_NameTagConfig GetNameTagConfig(){
		return s_NametagCfg;
	}
}

if(!s_NametagCfg){
	ResourceName path = "{9CBA25AE993E2F84}Configs/NameTags/NametagFriendlies_Spectator.conf";
	Resource container = BaseContainerTools.LoadContainer(path);
	if (!container)
		return;
	s_NametagCfg = SCR_NameTagConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer() ) );		
}
*/
/*
SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
if(!hudManager){
	Print("[CTA]HUD manager was not found");
	return false;
}
SCR_NameTagDisplay nameTagDisplay = SCR_NameTagDisplay.Cast(hudManager.FindInfoDisplay(SCR_NameTagDisplay));
if(!nameTagDisplay){
	Print("[CTA] SCR_NameTagDisplay was not found.");
	return false;
}*/