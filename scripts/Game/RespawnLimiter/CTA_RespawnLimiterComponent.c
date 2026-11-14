[ ComponentEditorProps ( category :  "CTA" , description :  "TODO" ) ] 
class CTA_RespawnLimiterComponentClass  :  ScriptComponentClass{ 
	
} 
enum CTA_RespawnLimitType{
	Individual,
	Ticket,
	Unlimited
}
void CTA_OnAlivePlayerAdded(int playerId);
typedef func CTA_OnAlivePlayerAdded;

void CTA_OnAlivePlayerRemoved(int playerId);
typedef func CTA_OnAlivePlayerRemoved;

void CTA_OnAlivePlayerReset(array<int> ids);
typedef func CTA_OnAlivePlayerReset;

class CTA_AlivePlayerComponentClass: ScriptComponentClass{
}

class CTA_AlivePlayerComponent:ScriptComponent{
	protected ref set<int> m_sAlives = new set<int>();
	protected ref ScriptInvokerBase<CTA_OnAlivePlayerAdded> m_OnAlivePlayerAdded = new ScriptInvokerBase<CTA_OnAlivePlayerAdded>;
	protected ref ScriptInvokerBase<CTA_OnAlivePlayerRemoved> m_OnAlivePlayerRemoved = new ScriptInvokerBase<CTA_OnAlivePlayerRemoved>;
	protected ref ScriptInvokerBase<CTA_OnAlivePlayerReset> m_OnAlivePlayerReset = new ScriptInvokerBase<CTA_OnAlivePlayerReset>;
	protected static CTA_AlivePlayerComponent m_sInstance;
	protected CTA_RespawnLimiterComponent m_respawnLimiter;
	
	static CTA_AlivePlayerComponent GetInstance(){
		return m_sInstance;
	}
	ScriptInvokerBase<CTA_OnAlivePlayerAdded> GetOnAlivePlayerAdded(){
		return m_OnAlivePlayerAdded;
	}

	ScriptInvokerBase<CTA_OnAlivePlayerRemoved> GetOnAlivePlayerRemoved(){
		return m_OnAlivePlayerRemoved;
	}

	ScriptInvokerBase<CTA_OnAlivePlayerReset> GetOnAlivePlayerReset(){
		return m_OnAlivePlayerReset;
	}
	
	void GetAlivePlayer(out notnull array<int> ids){
		foreach(int id :m_sAlives){
			ids.Insert(id);
		}
	}
	
	void Clear(){
		array<int> ids = {};
		Reset(ids);
	}
	void SetAlive(int playerId,bool isAlive){
		RPC_SetAlive(playerId,isAlive);
		Rpc(RPC_SetAlive,playerId,isAlive);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetAlive(int playerId,bool isAlive){
		if(isAlive){
			if(m_sAlives.Contains(playerId)){
				return;
			}
			m_sAlives.Insert(playerId);
			m_OnAlivePlayerAdded.Invoke(playerId);
		} else {
			m_sAlives.RemoveItem(playerId);
			m_OnAlivePlayerRemoved.Invoke(playerId);
		}
	}
		
	void Reset(array<int> ids){
		RPC_Reset(ids);
		Rpc(RPC_Reset,ids);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_Reset(array<int> ids){
		m_sAlives.Clear();
		foreach(int id : ids){
			m_sAlives.Insert(id);
		}	
		m_OnAlivePlayerReset.Invoke(ids);
	}

	void OnRespawnLimitReachedHandler(int playerId){
		SetAlive(playerId,false);
	}
	
	void OnRespawnLimitTypeChangedHandler(CTA_RespawnLimitType type){
		CTA_RespawnLimiterComponent respawnLimiter = CTA_RespawnLimiterComponent.GetInstance();
		array<int> ids = {};
		if(type == CTA_RespawnLimitType.Ticket){
			GetGame().GetPlayerManager().GetPlayers(ids);
		} else{
			respawnLimiter.GetRespawnablePlayers(ids);	
		}
		Reset(ids);
	}
	
	void OnPlayerRespawnableHandler(int playerId){
		SetAlive(playerId,true);
	}	
	
	void OnPlayerAddedHandler(int playerId){
		SetAlive(playerId,true);
	}
	
	void OnPlayerRemovedHandler(int playerId){
		SetAlive(playerId,false);
	}
	
	void OnRespawnLimitChangedHandler(int newLimit){
		array<int> ids = {};
		m_respawnLimiter.GetRespawnablePlayers(ids);
		Reset(ids);
	}
	
	override void OnPostInit(IEntity owner){
		m_sInstance = this;
		SetEventMask(owner,EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner){
		m_respawnLimiter = CTA_RespawnLimiterComponent.Cast(owner.FindComponent(CTA_RespawnLimiterComponent));
		if(!m_respawnLimiter){
			Print("[CTA] CTA_RespawnLimiterComponent is not attached to entity of CTA_AlivePlayerComponent.",LogLevel.ERROR);
			return;
		}
		m_respawnLimiter.GetOnRespawnLimitReached().Insert(OnRespawnLimitReachedHandler);
		m_respawnLimiter.GetOnRespawnLimitChanged().Insert(OnRespawnLimitChangedHandler);
		m_respawnLimiter.GetOnRespawnLimitTypeChanged().Insert(OnRespawnLimitTypeChangedHandler);
		m_respawnLimiter.GetOnPlayerRespawnable().Insert(OnPlayerRespawnableHandler);
		m_respawnLimiter.GetOnPlayerAdded().Insert(OnPlayerAddedHandler);
		m_respawnLimiter.GetOnPlayerRemoved().Insert(OnPlayerRemovedHandler);

	}
	
	
}

void CTA_OnRespawnLimitTypeChanged(CTA_RespawnLimitType type);
typedef func CTA_OnRespawnLimitTypeChanged;

void CTA_OnAliveUpdated(int playerId,bool isAlive);
typedef func CTA_OnAliveUpdated;

void CTA_OnRespawnLimitReached(int playerId);
typedef func CTA_OnRespawnLimitReached;

void CTA_OnPlayerRespawnable(int playerId);
typedef func CTA_OnPlayerRespawnable;

void CTA_OnPlayerAdded(int playerId);
typedef func CTA_OnPlayerAdded;

void CTA_OnPlayerRemoved(int playerId);
typedef func CTA_OnPlayerRemoved;

void CTA_OnRespawnLimitChanged(int newLimit);
typedef func CTA_OnRespawnLimitChanged;

class CTA_RespawnLimiterComponent:ScriptComponent{
	protected ref map<int,int> m_mDeathCount = new map<int,int>();
	[RplProp()]
	protected int m_iTotalDeath = 0;
	[RplProp(onRplName:"OnRespawnLimitBroadcasted")]
	protected int m_iRespawnLimit = 0;
	protected int m_iDeathSelf = 0;
	
	[RplProp(onRplName:"OnRespawnLimitTypeBroadcasted")]
	protected CTA_RespawnLimitType m_eRespawnLimitType = CTA_RespawnLimitType.Unlimited;
	
	protected ref ScriptCallQueue m_Callqueue = new ScriptCallQueue();
	protected ref ScriptInvokerBase<CTA_OnRespawnLimitReached> m_OnRespawnLimitReached = new ScriptInvokerBase<CTA_OnRespawnLimitReached>();
	protected ref ScriptInvokerBase<CTA_OnRespawnLimitTypeChanged> m_OnRespawnLimitTypeChanged =  new ScriptInvokerBase<CTA_OnRespawnLimitTypeChanged>();
	protected ref ScriptInvokerBase<CTA_OnRespawnLimitChanged> m_OnRespawnLimitChanged =  new ScriptInvokerBase<CTA_OnRespawnLimitChanged>();

	protected ref ScriptInvokerBase<CTA_OnPlayerRespawnable> m_OnPlayerRespawnable =  new ScriptInvokerBase<CTA_OnPlayerRespawnable>();
	protected ref ScriptInvokerBase<CTA_OnPlayerAdded> m_OnPlayerAdded =  new ScriptInvokerBase<CTA_OnPlayerAdded>();
	protected ref ScriptInvokerBase<CTA_OnPlayerRemoved> m_OnPlayerRemoved =  new ScriptInvokerBase<CTA_OnPlayerRemoved>();

	
	protected static CTA_RespawnLimiterComponent s_Instance;
	
	static CTA_RespawnLimiterComponent GetInstance(){
		return s_Instance;
	}
	
	ScriptInvokerBase<CTA_OnRespawnLimitReached> GetOnRespawnLimitReached(){
		return m_OnRespawnLimitReached;
	}
	ScriptInvokerBase<CTA_OnPlayerRespawnable> GetOnPlayerRespawnable(){
		return m_OnPlayerRespawnable;
	}
	ScriptInvokerBase<CTA_OnRespawnLimitTypeChanged> GetOnRespawnLimitTypeChanged(){
		return m_OnRespawnLimitTypeChanged;
	}
	
	ScriptInvokerBase<CTA_OnPlayerAdded> GetOnPlayerAdded(){
		return m_OnPlayerAdded;
	}
	
	ScriptInvokerBase<CTA_OnPlayerRemoved> GetOnPlayerRemoved(){
		return m_OnPlayerRemoved;
	}
	
	ScriptInvokerBase<CTA_OnRespawnLimitChanged> GetOnRespawnLimitChanged(){
		return m_OnRespawnLimitChanged;
	}
	
	void OnRespawnLimitTypeBroadcasted(){ 
		m_OnRespawnLimitTypeChanged.Invoke(m_eRespawnLimitType);
	}
	
	void OnRespawnLimitBroadcasted(){ 
		m_OnRespawnLimitChanged.Invoke(m_iRespawnLimit);
	}
	// ServerOnly
	void SetRespawnLimit(int num){
		if(m_iRespawnLimit == num){
			return;
		}
		m_iRespawnLimit = num;
		/*
		map<int,bool> old = new map<int,bool>();
		foreach(int id,int death:m_mDeathCount){
			old.Insert(id,isRespawnLimited(id));
		}
		m_iRespawnLimit = i;
		map<int,bool> _new = new map<int,bool>();
		foreach(int id,int death:m_mDeathCount){
			_new.Insert(id,isRespawnLimited(id));
		}
		array<int> aLimited = {};
		array<int> aRespawnable = {};
		foreach(int id,bool isLimited: _new){
			if (isLimited != old.Get(id)){
				if(isLimited){
					aLimited.Insert(id);
				} else {
					aRespawnable.Insert(id);
				}
			}
		}
		RPC_OnRespawnLimitReached(aLimited);
		Rpc(RPC_OnRespawnLimitReached,aLimited);
		RPC_OnPlayerRespawnable(aRespawnable);
		Rpc(RPC_OnPlayerRespawnable,aRespawnable);
		*/
		OnRespawnLimitBroadcasted();
		Replication.BumpMe();
	}

	/*
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_OnRespawnLimitReached(array<int>aId){
		foreach(int id :aId){
			GetOnRespawnLimitReached().Invoke(id);	
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_OnPlayerRespawnable(array<int>aId){
		foreach(int id :aId){
			GetOnPlayerRespawnable().Invoke(id);	
		}
	}
	*/
	int GetRespawnLimit(){
		return m_iRespawnLimit;
	}

	int GetDeath(int playerId){
		return m_mDeathCount.Get(playerId);
	}
	int GetDeathTotal(){
		return m_iTotalDeath;
	}
	int GetDeathSelf(){
		return m_iDeathSelf;
	}
	void SetTotalDeath(int count){
	if(m_iTotalDeath == count){
			return;
		}
		m_iTotalDeath = count;
		Replication.BumpMe();
	}
	void SetDeathSelf(int playerId,int count){
		if(isLocalPlayer(playerId)){
			m_iDeathSelf = count;	
			return;	
		}
		Rpc(RPC_SetDeathSelf,playerId,count);
	}
	
	CTA_RespawnLimitType GetRespawnLimitType(){
		return m_eRespawnLimitType;
	}
	// ServerOnly
	void SetRespawnType(CTA_RespawnLimitType type){
		if(m_eRespawnLimitType == type){
			return;
		}
		m_eRespawnLimitType = type;
		OnRespawnLimitTypeBroadcasted();
		Replication.BumpMe();
	}
	override void EOnFixedFrame(IEntity owner, float timeSlice){
		super.EOnFixedFrame(owner.timeSlice);
		m_Callqueue.Tick(timeSlice);
	}
	
	override void OnPostInit(IEntity owner){
		s_Instance = this;
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if(!gameMode){
			PrintFormat("[CTA]No gamemode found.");
			return;
		}
		array<int> aPlayerId = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(aPlayerId);
		foreach(int playerId : aPlayerId){
			m_mDeathCount.Set(playerId,0);
		}
		
		gameMode.GetOnPlayerConnected().Insert(OnPlayerConnectedHandler);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilledHandler);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnectedHandler);
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedHandler);
		SetEventMask(GetOwner(),EntityEvent.FIXEDFRAME);
		m_Callqueue.CallLater(SetSFRespawnLimit,2000);
	}
	
	void SetSFRespawnLimit(){
		// Set Respawn Limit
		SCR_ScenarioFrameworkSystem sfSys = SCR_ScenarioFrameworkSystem.GetInstance();
		if(!sfSys){
			return;
		}
		string sSfRespawnLimit;
		if(!sfSys.GetVariable("respawnLimit",sSfRespawnLimit)){
			return;
		}
		int len;
		int iSfRespawnLimit = sSfRespawnLimit.ToInt(parsed:len);
		if(len == 0){
			PrintFormat("[CTA]Failed to parse the ScenarioFramework variable into an int. VariableName:respawnLimit Value:%1",sSfRespawnLimit);
			return;
		}
		SetRespawnLimit(iSfRespawnLimit);
	
	}
	protected bool isNoTicket(){
		return m_iTotalDeath > m_iRespawnLimit;

	}
	bool isRespawnLimited(int playerId){
		if(m_eRespawnLimitType == CTA_RespawnLimitType.Unlimited){
			return false;
		}
		if(m_eRespawnLimitType == CTA_RespawnLimitType.Ticket) {
			return isNoTicket();
		}
		//Individual
		if(!m_mDeathCount.Contains(playerId)){
			m_mDeathCount.Set(playerId,0);
		}
		int death = m_mDeathCount.Get(playerId);
		return death > m_iRespawnLimit;
	}
	
	void GetRespawnablePlayers(out notnull array<int> ids){
		array<int> plIds = {};
		GetGame().GetPlayerManager().GetPlayers(plIds);
		foreach(int id : plIds){
			if(!isRespawnLimited(id)){
				ids.Insert(id);
			}
		}
	}
	/*
	void GetNoRespawnPlayers(out notnull array<int> ids){
		if(m_eRespawnLimitType == CTA_RespawnLimitType.Unlimited){
			GetGame().GetPlayerManager().GetPlayers(ids);
			return;
		}
		array<int> plIds = {};
		if( ( m_eRespawnLimitType == CTA_RespawnLimitType.Ticket && isNoTicket() ) ||
			( m_eRespawnLimitType == CTA_RespawnLimitType.Individual ))
		{
			GetGame().GetPlayerManager().GetPlayers(plIds);
			foreach(int id :plIds){
				ids.Insert(id);
			}
		}
	}
	*/
	
	void OnPlayerKilledHandler(notnull SCR_InstigatorContextData instigatorContextData){
		int playerId = instigatorContextData.GetVictimPlayerID();
		if(!m_mDeathCount.Contains(playerId)){
			m_mDeathCount.Set(playerId,0);
		}
		int newDeathCount = m_mDeathCount.Get(playerId) + 1;
		m_mDeathCount.Set(playerId,newDeathCount);
		
		SetTotalDeath(m_iTotalDeath + 1);
		SetDeathSelf(playerId,newDeathCount);
		if(isRespawnLimited(playerId)){
			GetOnRespawnLimitReached().Invoke(playerId);	
		}
	}
	
	void OnPlayerConnectedHandler(int playerId){
		m_mDeathCount.Set(playerId,0);
		GetOnPlayerAdded().Invoke(playerId);
	}
	
	void OnPlayerDisconnectedHandler(int playerId, KickCauseCode cause = KickCauseCode.NONE, int timeout = -1){
		m_mDeathCount.Remove(playerId); 
		GetOnPlayerRemoved().Invoke(playerId);
	}
	
	void OnPlayerSpawnedHandler(int playerId, IEntity player){
		if(!isRespawnLimited(playerId)){
			return;
		}
		PlayerController pc = GetGame().GetPlayerController();
		if(!pc){
			Rpc(RPC_EnableSpaectator,playerId);
			return;
		}
		if(pc.GetPlayerId() == playerId){
			EnableSpectatorDelay(playerId);
			return;
		}
		Rpc(RPC_EnableSpaectator,playerId);
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetDeathSelf(int playerId,int count){
		if(isLocalPlayer(playerId)){
			m_iDeathSelf = count;		
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_EnableSpaectator(int playerId){
		EnableSpectatorDelay(playerId);
	}
	void EnableSpectatorDelay(int playerId){
		PlayerController pc = GetGame().GetPlayerController();
		if(!pc){
			return;
		}
		int id = pc.GetPlayerId();
		if(id != playerId){
			return;
		}
		/*
		if(System.IsConsoleApp()){
			return;
		}
		*/
		CTA_Spectator spectator = CTA_Spectator.GetInstance();
		if(!spectator){
			return;
		}
		m_Callqueue.CallLater(spectator.Enable,1000);
	}
	
	bool isLocalPlayer(int playerId){
		int id = GetGame().GetPlayerController().GetPlayerId();
		return id == playerId;
	}
}

