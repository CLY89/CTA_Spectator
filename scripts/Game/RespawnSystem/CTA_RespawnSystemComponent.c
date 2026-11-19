enum CTA_RespawnType
{
	Life,
	Ticket,
	Unlimited
}


[ComponentEditorProps(category: "CTA", description: "Controls respawn type, initial resources, and spectator transitions")]
class CTA_RespawnSystemComponentClass : ScriptComponentClass {}

class CTA_RespawnSystemComponent : ScriptComponent
{
	[RplProp()]
	protected CTA_RespawnType m_eRespawnType = CTA_RespawnType.Unlimited;
	[RplProp()]
	protected int m_iInitialLives = 1;
	[RplProp()]
	protected int m_iInitialFactionTickets = 1;
	protected ref ScriptCallQueue m_Callqueue = new ScriptCallQueue();
	protected static CTA_RespawnSystemComponent s_Instance;
	protected CTA_PlayerLifeManagerComponent m_PlayerLifeManager;
	protected CTA_FactionTicketManagerComponent m_FactionTicketManager;

	static CTA_RespawnSystemComponent GetInstance()
	{
		return s_Instance;
	}

	override void OnPostInit(IEntity owner)
	{
		s_Instance = this;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
		{
			Print("[CTA] No SCR_BaseGameMode found for respawn management.", LogLevel.ERROR);
			return;
		}

		m_PlayerLifeManager = CTA_PlayerLifeManagerComponent.Cast(owner.FindComponent(CTA_PlayerLifeManagerComponent));
		m_FactionTicketManager = CTA_FactionTicketManagerComponent.Cast(owner.FindComponent(CTA_FactionTicketManagerComponent));
		if (!m_PlayerLifeManager || !m_FactionTicketManager)
		{
			Print("[CTA] RespawnSystem requires PlayerLifeManager and FactionTicketManager components.", LogLevel.ERROR);
			return;
		}

		if (Replication.IsServer())
		{
			SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
			if (missionHeader)
			{
				if (m_eRespawnType != missionHeader.m_eRespawnType || m_iInitialLives != missionHeader.m_iInitialLives || m_iInitialFactionTickets != missionHeader.m_iInitialFactionTickets)
				{
					m_eRespawnType = missionHeader.m_eRespawnType;
					m_iInitialLives = missionHeader.m_iInitialLives;
					m_iInitialFactionTickets = missionHeader.m_iInitialFactionTickets;
					Replication.BumpMe();
				}
			}
		}

		SetEventMask(owner, EntityEvent.FIXEDFRAME);

		if (!Replication.IsServer())
			return;

		gameMode.GetOnPlayerConnected().Insert(OnPlayerConnectedHandler);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilledHandler);
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnectedHandler);
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedHandler);
		m_Callqueue.CallLater(ApplyScenarioFrameworkRespawnType, 1000);

		InitializeCurrentPlayers();
	}

	protected void InitializeCurrentPlayers()
	{
		if (!Replication.IsServer())
			return;

		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		foreach (int playerId : playerIds)
			InitializePlayerForCurrentType(playerId);

		SyncCurrentManager();
	}

	protected void InitializePlayerForCurrentType(int playerId)
	{
		switch (m_eRespawnType)
		{
			case CTA_RespawnType.Life:
				m_PlayerLifeManager.InitializePlayer(playerId, m_iInitialLives);
				return;
			case CTA_RespawnType.Ticket:
				m_PlayerLifeManager.InitializePlayer(playerId, 1);
				m_FactionTicketManager.RegisterPlayer(playerId, m_iInitialFactionTickets);
				return;
		}
	}

	protected void SyncCurrentManager()
	{
		switch (m_eRespawnType)
		{
			case CTA_RespawnType.Life:
				m_PlayerLifeManager.SyncAllClients();
				return;
			case CTA_RespawnType.Ticket:
				m_PlayerLifeManager.SyncAllClients();
				m_FactionTicketManager.SyncAllClients();
				return;
		}
	}

	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		super.EOnFixedFrame(owner, timeSlice);
		m_Callqueue.Tick(timeSlice);
	}

	CTA_RespawnType GetRespawnType()
	{
		return m_eRespawnType;
	}

	void SetRespawnType(CTA_RespawnType type)
	{
		if (!Replication.IsServer() || m_eRespawnType == type)
			return;

		if (type == CTA_RespawnType.Ticket)
			m_PlayerLifeManager.SetAllPlayerLives(1);
		else if (type == CTA_RespawnType.Life)
			m_PlayerLifeManager.SetAllPlayerLives(m_iInitialLives);

		m_eRespawnType = type;
		Replication.BumpMe();
		InitializeCurrentPlayers();
	}

	int GetInitialLives()
	{
		return m_iInitialLives;
	}

	bool SetInitialLives(int lives)
	{
		if (!Replication.IsServer() || lives < 0 || !m_PlayerLifeManager)
			return false;

		if (m_iInitialLives != lives)
		{
			m_iInitialLives = lives;
			Replication.BumpMe();
		}

		if (GetRespawnType() == CTA_RespawnType.Ticket)
			m_PlayerLifeManager.SetAllPlayerLives(1);
		else
			m_PlayerLifeManager.SetAllPlayerLives(lives);

		return true;
	}

	int GetInitialFactionTickets()
	{
		return m_iInitialFactionTickets;
	}

	bool SetInitialFactionTickets(int tickets)
	{
		if (!Replication.IsServer() || tickets < 0 || !m_FactionTicketManager)
			return false;

		if (m_iInitialFactionTickets != tickets)
		{
			m_iInitialFactionTickets = tickets;
			Replication.BumpMe();
		}

		m_FactionTicketManager.SetAllFactionTickets(tickets);
		return true;
	}

	protected bool IsRespawnAllowed(int playerId)
	{
		switch (GetRespawnType())
		{
			case CTA_RespawnType.Life:
				return m_PlayerLifeManager.GetLives(playerId) > 0;
			case CTA_RespawnType.Ticket:
				int tickets = m_FactionTicketManager.GetTicketCountForPlayer(playerId);
				if (tickets > 0)
					return true;

				return tickets == 0 && m_PlayerLifeManager.GetLives(playerId) > 0;
			default:
				return true;
		}

		return true;
	}

	protected void ConsumeActiveRespawnResource(int playerId)
	{
		switch (GetRespawnType())
		{
			case CTA_RespawnType.Life:
				InitializePlayerForCurrentType(playerId);
				m_PlayerLifeManager.ConsumeLife(playerId);
				return;
			case CTA_RespawnType.Ticket:
				if (m_FactionTicketManager.GetTicketCountForPlayer(playerId) == 0)
				{
					m_PlayerLifeManager.InitializePlayer(playerId, 1);
					m_PlayerLifeManager.ConsumeLife(playerId);
					return;
				}

				m_FactionTicketManager.ConsumeTicketForPlayer(playerId, m_iInitialFactionTickets);
				return;
		}
	}

	protected void OnPlayerConnectedHandler(int playerId)
	{
		if (!Replication.IsServer())
			return;

		InitializePlayerForCurrentType(playerId);
		SyncCurrentManager();
	}

	protected void OnPlayerKilledHandler(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (!Replication.IsServer())
			return;

		int playerId = instigatorContextData.GetVictimPlayerID();
		if (playerId <= 0)
			return;

		ConsumeActiveRespawnResource(playerId);
	}

	protected void OnPlayerDisconnectedHandler(int playerId, KickCauseCode cause = KickCauseCode.NONE, int timeout = -1)
	{
		if (!Replication.IsServer())
			return;

		m_PlayerLifeManager.RemovePlayer(playerId);
		m_FactionTicketManager.RemovePlayer(playerId);
	}

	protected void OnPlayerSpawnedHandler(int playerId, IEntity player)
	{
		if (!Replication.IsServer())
			return;

		InitializePlayerForCurrentType(playerId);
		if (IsRespawnAllowed(playerId))
			return;

		RPC_EnableSpectator(playerId);
		Rpc(RPC_EnableSpectator, playerId);
	}

	protected void ApplyScenarioFrameworkRespawnType()
	{
		SCR_ScenarioFrameworkSystem scenarioFramework = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFramework)
			return;

		string value;
		if (!scenarioFramework.GetVariable("respawnType", value))
			return;

		value.ToLower();
		switch (value)
		{
			case "life":
				SetRespawnType(CTA_RespawnType.Life);
				return;
			case "individual":
				SetRespawnType(CTA_RespawnType.Life);
				return;
			case "ticket":
				SetRespawnType(CTA_RespawnType.Ticket);
				return;
			case "unlimited":
				SetRespawnType(CTA_RespawnType.Unlimited);
				return;
		}

		PrintFormat("[CTA] Unknown Scenario Framework respawnType value: %1", value, level: LogLevel.ERROR);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_EnableSpectator(int playerId)
	{
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller || controller.GetPlayerId() != playerId)
			return;

		CTA_Spectator spectator = CTA_Spectator.GetInstance();
		if (spectator)
			m_Callqueue.CallLater(spectator.Enable, 1000);
	}
}
