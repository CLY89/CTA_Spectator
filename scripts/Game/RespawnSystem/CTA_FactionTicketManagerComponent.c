[ComponentEditorProps(category: "CTA", description: "Manages remaining tickets for each faction")]
class CTA_FactionTicketManagerComponentClass : ScriptComponentClass {}

class CTA_FactionTicketManagerComponent : ScriptComponent
{
	protected ref map<int, string> m_mPlayerFactions = new map<int, string>();
	protected ref map<string, int> m_mFactionTickets = new map<string, int>();
	protected static CTA_FactionTicketManagerComponent s_Instance;

	static CTA_FactionTicketManagerComponent GetInstance()
	{
		return s_Instance;
	}

	int GetTicketCountForPlayer(int playerId)
	{
		if (!m_mPlayerFactions.Contains(playerId))
			return -1;

		string factionKey = m_mPlayerFactions.Get(playerId);
		if (!m_mFactionTickets.Contains(factionKey))
			return 0;

		return m_mFactionTickets.Get(factionKey);
	}

	void RegisterPlayer(int playerId, int initialTickets)
	{
		if (!Replication.IsServer())
			return;

		string factionKey;
		if (!GetPlayerFactionKey(playerId, factionKey))
			return;

		SetPlayerFaction(playerId, factionKey);
		EnsureFaction(factionKey, initialTickets);
	}

	int ConsumeTicketForPlayer(int playerId, int initialTickets)
	{
		if (!Replication.IsServer())
			return -1;

		string factionKey;
		if (!GetPlayerFactionKey(playerId, factionKey))
		{
			PrintFormat("[CTA] Cannot consume a respawn ticket: player %1 has no faction.", playerId, level: LogLevel.WARNING);
			return -1;
		}

		SetPlayerFaction(playerId, factionKey);
		EnsureFaction(factionKey, initialTickets);
		int tickets = m_mFactionTickets.Get(factionKey);
		if (tickets > 0)
			tickets--;

		SetFactionTickets(factionKey, tickets);
		return tickets;
	}

	void SetAllFactionTickets(int tickets)
	{
		if (!Replication.IsServer())
			return;

		array<string> factionKeys = {};
		foreach (string factionKey, int currentTickets : m_mFactionTickets)
			factionKeys.Insert(factionKey);

		foreach (string factionKey : factionKeys)
			SetFactionTickets(factionKey, tickets);
	}

	void RemovePlayer(int playerId)
	{
		if (!Replication.IsServer() || !m_mPlayerFactions.Contains(playerId))
			return;

		m_mPlayerFactions.Remove(playerId);
		RPC_SetPlayerFaction(playerId, string.Empty, false);
		Rpc(RPC_SetPlayerFaction, playerId, string.Empty, false);
	}

	void SyncAllClients()
	{
		if (!Replication.IsServer())
			return;

		foreach (int playerId, string factionKey : m_mPlayerFactions)
		{
			RPC_SetPlayerFaction(playerId, factionKey, true);
			Rpc(RPC_SetPlayerFaction, playerId, factionKey, true);
		}

		foreach (string factionKey, int tickets : m_mFactionTickets)
		{
			RPC_SetFactionTickets(factionKey, tickets);
			Rpc(RPC_SetFactionTickets, factionKey, tickets);
		}
	}

	protected void EnsureFaction(string factionKey, int initialTickets)
	{
		if (!m_mFactionTickets.Contains(factionKey))
			SetFactionTickets(factionKey, initialTickets);
	}

	protected void SetFactionTickets(string factionKey, int tickets)
	{
		if (tickets < 0)
			tickets = 0;

		m_mFactionTickets.Set(factionKey, tickets);
		RPC_SetFactionTickets(factionKey, tickets);
		Rpc(RPC_SetFactionTickets, factionKey, tickets);
	}

	protected void SetPlayerFaction(int playerId, string factionKey)
	{
		m_mPlayerFactions.Set(playerId, factionKey);
		RPC_SetPlayerFaction(playerId, factionKey, true);
		Rpc(RPC_SetPlayerFaction, playerId, factionKey, true);
	}

	protected bool GetPlayerFactionKey(int playerId, out string factionKey)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;

		Faction faction = factionManager.GetPlayerFaction(playerId);
		if (!faction)
			return false;

		factionKey = faction.GetFactionKey();
		return !factionKey.IsEmpty();
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetPlayerFaction(int playerId, string factionKey, bool isPresent)
	{
		if (isPresent)
			m_mPlayerFactions.Set(playerId, factionKey);
		else
			m_mPlayerFactions.Remove(playerId);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetFactionTickets(string factionKey, int tickets)
	{
		m_mFactionTickets.Set(factionKey, tickets);
	}

	override void OnPostInit(IEntity owner)
	{
		s_Instance = this;
	}
}
