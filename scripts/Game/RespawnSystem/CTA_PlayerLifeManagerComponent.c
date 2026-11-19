[ComponentEditorProps(category: "CTA", description: "Manages each player's remaining lives")]
class CTA_PlayerLifeManagerComponentClass : ScriptComponentClass {}

class CTA_PlayerLifeManagerComponent : ScriptComponent
{
	protected ref map<int, int> m_mPlayerLives = new map<int, int>();
	protected ref ScriptInvoker<int> m_OnPlayerLivesChanged = new ScriptInvoker();
	protected static CTA_PlayerLifeManagerComponent s_Instance;

	static CTA_PlayerLifeManagerComponent GetInstance()
	{
		return s_Instance;
	}

	ScriptInvoker<int> GetOnPlayerLivesChanged()
	{
		return m_OnPlayerLivesChanged;
	}

	int GetLives(int playerId)
	{
		if (!m_mPlayerLives.Contains(playerId))
			return -1;

		return m_mPlayerLives.Get(playerId);
	}

	void InitializePlayer(int playerId, int lives)
	{
		if (!Replication.IsServer() || playerId <= 0 || m_mPlayerLives.Contains(playerId))
			return;

		SetPlayerLives(playerId, lives);
	}

	int ConsumeLife(int playerId)
	{
		if (!Replication.IsServer() || !m_mPlayerLives.Contains(playerId))
			return -1;

		int lives = m_mPlayerLives.Get(playerId);
		if (lives > 0)
			lives--;

		SetPlayerLives(playerId, lives);
		return lives;
	}

	void SetAllPlayerLives(int lives)
	{
		if (!Replication.IsServer())
			return;

		array<int> playerIds = {};
		foreach (int playerId, int currentLives : m_mPlayerLives)
			playerIds.Insert(playerId);

		foreach (int playerId : playerIds)
			SetPlayerLives(playerId, lives);
	}

	void RemovePlayer(int playerId)
	{
		if (!Replication.IsServer() || !m_mPlayerLives.Contains(playerId))
			return;

		m_mPlayerLives.Remove(playerId);
		RPC_SetPlayerLives(playerId, 0, false);
		Rpc(RPC_SetPlayerLives, playerId, 0, false);
	}

	void SyncAllClients()
	{
		if (!Replication.IsServer())
			return;

		foreach (int playerId, int lives : m_mPlayerLives)
		{
			RPC_SetPlayerLives(playerId, lives, true);
			Rpc(RPC_SetPlayerLives, playerId, lives, true);
		}
	}

	protected void SetPlayerLives(int playerId, int lives)
	{
		if (lives < 0)
			lives = 0;

		m_mPlayerLives.Set(playerId, lives);
		RPC_SetPlayerLives(playerId, lives, true);
		Rpc(RPC_SetPlayerLives, playerId, lives, true);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetPlayerLives(int playerId, int lives, bool isPresent)
	{
		if (!isPresent)
		{
			m_mPlayerLives.Remove(playerId);
			m_OnPlayerLivesChanged.Invoke(playerId);
			return;
		}

		m_mPlayerLives.Set(playerId, lives);
		m_OnPlayerLivesChanged.Invoke(playerId);
	}

	override void OnPostInit(IEntity owner)
	{
		s_Instance = this;
	}
}
