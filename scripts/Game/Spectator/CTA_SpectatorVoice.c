// Receive proximity VoN at the spectator camera, using the same engine bridge as GM.
// GMと同じエンジンの連携機能を使い、観戦カメラの位置で近距離ボイスを受信する。
// No audio range override and no change to SCR_VONController's transmission component.
// 可聴範囲とSCR_VONControllerの送信コンポーネントは変更しない。
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class CTA_SpectatorVoiceManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected SCR_PlayerController m_CTA_LocalController;
	protected bool m_CTA_SentActive;
	override bool EOnCameraInit()
	{
		if (!GetGame() || System.IsConsoleApp()) return false;
		m_CTA_LocalController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		CTA_CallQueue.GetCallQueue().CallLater(CTA_UpdateVoice, 500, true);
		return false; // Updated by the bounded-frequency callqueue, not every frame.
		// 毎フレームではなく、一定間隔のコールキューで更新する。
	}

	protected void CTA_UpdateVoice()
	{
		if (!GetGame()) return;
		if (m_CTA_LocalController != GetGame().GetPlayerController())
		{
			if (m_CTA_SentActive && m_CTA_LocalController)
				CTA_SpectatorVoiceProxyComponent.StopForPlayer(m_CTA_LocalController.GetPlayerId());
			m_CTA_SentActive = false;
			m_CTA_LocalController = null;
		}
		if (!m_CTA_LocalController)
			m_CTA_LocalController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!m_CTA_LocalController) return;
		SCR_ManualCamera spectatorCamera = GetCameraEntity();
		SCR_EditorManagerEntity editor = CTA_SpectatorVoiceProxyComponent.GetEditorForPlayer(m_CTA_LocalController.GetPlayerId());
		bool active = spectatorCamera && GetGame().GetCameraManager().CurrentCamera() == spectatorCamera;
		if (editor && editor.IsOpened()) active = false;
		if (active)
		{
			m_CTA_LocalController.CTA_UpdateSpectatorVoice(true, spectatorCamera.GetOrigin());
			m_CTA_SentActive = true;
		}
		else if (m_CTA_SentActive)
		{
			m_CTA_LocalController.CTA_UpdateSpectatorVoice(false, vector.Zero);
			m_CTA_SentActive = false;
		}
	}

	override void EOnCameraExit()
	{
		if (GetGame()) CTA_CallQueue.GetCallQueue().Remove(CTA_UpdateVoice);
		if (m_CTA_SentActive && m_CTA_LocalController)
			m_CTA_LocalController.CTA_UpdateSpectatorVoice(false, vector.Zero);
		m_CTA_SentActive = false;
		m_CTA_LocalController = null;
	}
}

// The existing player-controller replication node supplies an owned client -> server route.
// 既存のプレイヤーコントローラーの同期ノードを使い、所有クライアントからサーバーへ通知する。
// There is no player-id parameter that could target somebody else's listener.
// 他人の受信位置を指定できないよう、プレイヤーIDは引数で受け取らない。
modded class SCR_PlayerController
{
	protected IEntity m_CTA_SpectatorVoiceProxy;
	protected int m_CTA_LocalVoiceSequence;
	protected bool m_CTA_LocalVoiceActive;
	protected int m_CTA_ServerVoiceSequence;
	protected static const ResourceName CTA_VOICE_PROXY_PREFAB = "{7644283733C948F5}Prefabs/Systems/CTA_SpectatorVoiceProxy.et";

	void CTA_UpdateSpectatorVoice(bool enabled, vector cameraPosition)
	{
		if (!GetGame() || GetGame().GetPlayerController() != this) return;
		// A state transition gets a new sequence; position heartbeats reuse that sequence.
		// 開始・終了のたびに通し番号を進め、位置の定期通知では同じ番号を使う。
		if (enabled != m_CTA_LocalVoiceActive)
		{
			m_CTA_LocalVoiceSequence++;
			m_CTA_LocalVoiceActive = enabled;
		}
		// Stop local playback immediately; replicated deletion may arrive later.
		// 同期された削除通知を待たずに、ローカル側の受信を直ちに停止する。
		if (!enabled) CTA_SpectatorVoiceProxyComponent.StopForPlayer(GetPlayerId());
		if (Replication.IsServer()) CTA_ApplySpectatorVoice_S(enabled, cameraPosition, m_CTA_LocalVoiceSequence);
		else Rpc(CTA_RpcAskSpectatorVoice, enabled, cameraPosition, m_CTA_LocalVoiceSequence);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void CTA_RpcAskSpectatorVoice(bool enabled, vector cameraPosition, int sequence)
	{
		CTA_ApplySpectatorVoice_S(enabled, cameraPosition, sequence);
	}

	bool CTA_AcceptsSpectatorVoice(int sequence)
	{
		return m_CTA_LocalVoiceActive && sequence == m_CTA_LocalVoiceSequence;
	}

	protected void CTA_ApplySpectatorVoice_S(bool enabled, vector cameraPosition, int sequence)
	{
		if (!Replication.IsServer() || !GetGame() || !GetWorld() || GetPlayerId() <= 0) return;
		// Delayed commands must not disable or revive a newer activation.
		// 遅れて届いた命令で、新しい観戦接続を停止・再開しない。
		if (sequence <= 0 || sequence < m_CTA_ServerVoiceSequence) return;
		if (sequence > m_CTA_ServerVoiceSequence)
		{
			m_CTA_ServerVoiceSequence = sequence;
			CTA_SpectatorVoiceProxyComponent.StopForPlayer(GetPlayerId());
			if (m_CTA_SpectatorVoiceProxy) RplComponent.DeleteRplEntity(m_CTA_SpectatorVoiceProxy, false);
			m_CTA_SpectatorVoiceProxy = null;
		}
		SCR_EditorManagerEntity editor = CTA_SpectatorVoiceProxyComponent.GetEditorForPlayer(GetPlayerId());
		if (editor && editor.IsOpened()) enabled = false;
		if (!enabled)
		{
			CTA_SpectatorVoiceProxyComponent.StopForPlayer(GetPlayerId());
			if (m_CTA_SpectatorVoiceProxy)
				RplComponent.DeleteRplEntity(m_CTA_SpectatorVoiceProxy, false);
			m_CTA_SpectatorVoiceProxy = null;
			return;
		}
		if (m_CTA_SpectatorVoiceProxy && !m_CTA_SpectatorVoiceProxy.IsDeleted())
		{
			CTA_SpectatorVoiceProxyComponent existing = CTA_SpectatorVoiceProxyComponent.Cast(m_CTA_SpectatorVoiceProxy.FindComponent(CTA_SpectatorVoiceProxyComponent));
			if (existing && existing.IsStopped())
			{
				RplComponent.DeleteRplEntity(m_CTA_SpectatorVoiceProxy, false);
				m_CTA_SpectatorVoiceProxy = null;
			}
		}
		if (!m_CTA_SpectatorVoiceProxy || m_CTA_SpectatorVoiceProxy.IsDeleted())
		{
			Resource resource = Resource.Load(CTA_VOICE_PROXY_PREFAB);
			if (!resource || !resource.IsValid()) return;
			EntitySpawnParams parameters = new EntitySpawnParams();
			parameters.TransformMode = ETransformMode.WORLD;
			parameters.Transform[3] = cameraPosition;
			m_CTA_SpectatorVoiceProxy = GetGame().SpawnEntityPrefab(resource, GetWorld(), parameters);
			if (!m_CTA_SpectatorVoiceProxy) return;
			RplComponent voiceReplication = RplComponent.Cast(m_CTA_SpectatorVoiceProxy.FindComponent(RplComponent));
			if (!voiceReplication)
			{
				RplComponent.DeleteRplEntity(m_CTA_SpectatorVoiceProxy, false);
				m_CTA_SpectatorVoiceProxy = null;
				return;
			}
			// Match native EditorManager ownership so the receiver belongs to its client.
			// 標準EditorManagerと同じく、受信機の所有権を対象クライアントへ渡す。
			RplIdentity listenerIdentity = GetRplIdentity();
			if (listenerIdentity != RplIdentity.Local()) voiceReplication.Give(listenerIdentity);
		}
		if (!m_CTA_SpectatorVoiceProxy) return;
		CTA_SpectatorVoiceProxyComponent listener = CTA_SpectatorVoiceProxyComponent.Cast(m_CTA_SpectatorVoiceProxy.FindComponent(CTA_SpectatorVoiceProxyComponent));
		if (listener) listener.UpdateListener(GetPlayerId(), cameraPosition, sequence);
	}
}

[ComponentEditorProps(category: "CTA", description: "Temporary spectator-camera proximity voice receiver")]
class CTA_SpectatorVoiceProxyComponentClass : ScriptComponentClass {}

class CTA_SpectatorVoiceProxyComponent : ScriptComponent
{
	protected static ref map<int, CTA_SpectatorVoiceProxyComponent> s_CTA_Listeners = new map<int, CTA_SpectatorVoiceProxyComponent>();
	protected static ref map<int, int> s_CTA_LastGenerations = new map<int, int>();
	protected static BaseWorld s_CTA_RegistryWorld;
	protected static int s_CTA_NextGeneration;
	[RplProp(onRplName: "CTA_ApplyListenerState")]
	protected int m_CTA_Generation;
	[RplProp(onRplName: "CTA_ApplyListenerState")]
	protected int m_CTA_RequestSequence;
	[RplProp(onRplName: "CTA_ApplyListenerState")]
	protected int m_CTA_PlayerId;
	[RplProp(onRplName: "CTA_ApplyListenerState")]
	protected vector m_CTA_CameraPosition;
	protected SCR_VoNComponent m_CTA_VoN;
	protected bool m_CTA_Connected;
	protected bool m_CTA_Stopped;
	protected static const int CTA_UPDATE_TIMEOUT_MS = 15000;
	protected int m_CTA_LastUpdate;
	protected bool m_CTA_HasObserver;
	protected RplIdentity m_CTA_ObserverIdentity;
	// Keep generation tombstones until the world changes, including after receiver deletion.
	// 受信機の削除後も最終世代を記憶し、ワールドが変わったら登録情報を初期化する。
	protected static void CTA_EnsureRegistryWorld()
	{
		if (!GetGame()) return;
		BaseWorld world = GetGame().GetWorld();
		if (s_CTA_RegistryWorld == world) return;
		s_CTA_RegistryWorld = world;
		s_CTA_Listeners.Clear();
		s_CTA_LastGenerations.Clear();
	}

	// Resolve the editor through its core instance, as native VoN does.
	// 標準VoNと同じく、コアのインスタンス経由でエディターを取得する。
	static SCR_EditorManagerEntity GetEditorForPlayer(int playerId)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core) return null;
		return core.GetEditorManager(playerId);
	}
	bool IsStopped()
	{
		return m_CTA_Stopped;
	}

	// Release only this feature's registration before native GM starts its own connection.
	// 標準GMが接続を開始する前に、この機能が所有する登録だけを解除する。
	static void StopForPlayer(int playerId)
	{
		CTA_EnsureRegistryWorld();
		CTA_SpectatorVoiceProxyComponent listener = s_CTA_Listeners.Get(playerId);
		if (listener) listener.CTA_StopListener();
	}

	static IEntity GetListenerEntity(int playerId)
	{
		CTA_EnsureRegistryWorld();
		CTA_SpectatorVoiceProxyComponent listener = s_CTA_Listeners.Get(playerId);
		if (!listener || listener.m_CTA_Stopped || !listener.GetOwner() || listener.GetOwner().IsDeleted()) return null;
		return listener.GetOwner();
	}

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode() || !GetGame()) return;
		m_CTA_VoN = SCR_VoNComponent.Cast(owner.FindComponent(SCR_VoNComponent));
		CTA_ApplyListenerState();
		if (Replication.IsServer()) CTA_CallQueue.GetCallQueue().CallLater(CTA_ServerWatchdog, 1000, true);
		else CTA_CallQueue.GetCallQueue().CallLater(CTA_ApplyListenerState, 500, true);
	}

	void UpdateListener(int playerId, vector cameraPosition, int sequence)
	{
		if (m_CTA_Stopped || !Replication.IsServer() || !GetGame() || !GetOwner() || !GetOwner().GetWorld() || playerId <= 0) return;
		if (m_CTA_Generation == 0)
		{
			s_CTA_NextGeneration++;
			m_CTA_Generation = s_CTA_NextGeneration;
		}
		m_CTA_RequestSequence = sequence;
		m_CTA_PlayerId = playerId;
		m_CTA_CameraPosition = cameraPosition;
		m_CTA_LastUpdate = GetOwner().GetWorld().GetWorldTime();
		CTA_ApplyListenerState();
		CTA_AddObserver();
		Replication.BumpMe();
	}

	protected void CTA_ApplyListenerState()
	{
		if (m_CTA_Stopped || !GetGame() || !GetOwner() || m_CTA_PlayerId <= 0 || m_CTA_Generation <= 0 || m_CTA_RequestSequence <= 0) return;
		if (GetOwner().GetWorld() != GetGame().GetWorld()) return;
		CTA_EnsureRegistryWorld();
		SCR_EditorManagerEntity editor = GetEditorForPlayer(m_CTA_PlayerId);
		if (editor && editor.IsOpened()) return;
		PlayerController localController = GetGame().GetPlayerController();
		// Late replication must not restart playback after the local camera has closed.
		// 同期が遅れて到着しても、終了済みカメラの音声受信を再開しない。
		if (!Replication.IsServer() && localController && localController.GetPlayerId() == m_CTA_PlayerId)
		{
			SCR_PlayerController controller = SCR_PlayerController.Cast(localController);
			if (!controller || !controller.CTA_AcceptsSpectatorVoice(m_CTA_RequestSequence))
			{
				CTA_StopListener();
				return;
			}
			SCR_ManualCamera camera = SCR_ManualCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
			if (!camera || !camera.FindCameraComponent(CTA_SpectatorVoiceManualCameraComponent))
			{
				if (m_CTA_Connected) CTA_StopListener();
				return;
			}
		}
		CTA_SpectatorVoiceProxyComponent previous = s_CTA_Listeners.Get(m_CTA_PlayerId);
		int latestGeneration = s_CTA_LastGenerations.Get(m_CTA_PlayerId);
		// An older replica cannot replace a newer receiver or revive a closed generation.
		// 古い同期個体で新しい受信機を置き換えたり、終了済み世代を復活させたりしない。
		if (m_CTA_Generation < latestGeneration || (m_CTA_Generation == latestGeneration && previous != this))
		{
			CTA_StopListener();
			return;
		}
		s_CTA_LastGenerations.Set(m_CTA_PlayerId, m_CTA_Generation);
		GetOwner().SetOrigin(m_CTA_CameraPosition);
		// Detach the old receiver before registering its replacement.
		// 新しい受信機を登録する前に、以前の受信機を切り離す。
		if (previous && previous != this) previous.CTA_StopListener();
		s_CTA_Listeners.Set(m_CTA_PlayerId, this);
		if (!m_CTA_VoN) m_CTA_VoN = SCR_VoNComponent.Cast(GetOwner().FindComponent(SCR_VoNComponent));
		if (!m_CTA_VoN || m_CTA_Connected) return;
		// Server routes the voice, and only the receiver's own client registers playback.
		// サーバーで音声を転送し、受信者本人のクライアントだけが再生を登録する。
		if (Replication.IsServer() || (localController && localController.GetPlayerId() == m_CTA_PlayerId))
		{
			m_CTA_VoN.ConnectEditorToVoNSystem(m_CTA_PlayerId);
			m_CTA_Connected = true;

		}
	}

	protected void CTA_AddObserver()
	{
		if (m_CTA_Stopped || m_CTA_HasObserver || !Replication.IsServer() || !GetGame() || !GetOwner() || !GetOwner().GetWorld()) return;
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(m_CTA_PlayerId);
		// The listen host already has its local camera observer.
		// プレイヤー兼用ホストでは、ローカルカメラの監視位置がすでに登録されている。
		if (!controller || controller == GetGame().GetPlayerController()) return;
		World world = GetOwner().GetWorld();
		if (!world) return;
		ObserversSystem observers = ObserversSystem.Cast(world.FindSystem(ObserversSystem));
		if (!observers) return;
		m_CTA_ObserverIdentity = controller.GetRplIdentity();
		observers.InsertObserverMP(m_CTA_ObserverIdentity, 0, 0, GetOwner());
		m_CTA_HasObserver = true;
	}

	protected void CTA_ServerWatchdog()
	{
		if (!Replication.IsServer() || !GetGame() || !GetOwner() || !GetOwner().GetWorld()) return;
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(m_CTA_PlayerId);
		// Recover from a disconnected client or a camera that was removed without sending stop.
		// クライアント切断や、停止通知なしでカメラが削除された場合に受信処理を後始末する。
		if (m_CTA_Stopped || !controller || GetOwner().GetWorld().GetWorldTime() - m_CTA_LastUpdate > CTA_UPDATE_TIMEOUT_MS)
		{
			RplComponent.DeleteRplEntity(GetOwner(), false);
			return;
		}
		CTA_ApplyListenerState();
	}

	protected void CTA_StopListener()
	{
		if (m_CTA_Stopped) return;
		m_CTA_Stopped = true;
		if (m_CTA_Connected && m_CTA_VoN) m_CTA_VoN.DisconnectEditorFromVoNSystem();
		m_CTA_Connected = false;
		if (s_CTA_Listeners.Get(m_CTA_PlayerId) == this) s_CTA_Listeners.Remove(m_CTA_PlayerId);
		if (m_CTA_HasObserver && GetGame() && GetOwner() && GetOwner().GetWorld())
		{
			World world = GetOwner().GetWorld();
			ObserversSystem observers;
			if (world) observers = ObserversSystem.Cast(world.FindSystem(ObserversSystem));
			if (observers) observers.RemoveObserverMP(m_CTA_ObserverIdentity);
		}
		m_CTA_HasObserver = false;
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			CTA_CallQueue.GetCallQueue().Remove(CTA_ServerWatchdog);
			CTA_CallQueue.GetCallQueue().Remove(CTA_ApplyListenerState);
		}
		CTA_StopListener();
		super.OnDelete(owner);
	}
}

// Use normal GM handling unless this particular player has an active CTA receiver.
// 対象プレイヤーのCTA受信機が有効な場合以外は、通常のGM処理を使う。
// Real Game Master sessions take precedence. No GM permissions are granted by this feature.
// 実際のゲームマスター操作を優先する。この機能によってGM権限は付与しない。
modded class SCR_VoNComponent
{
	override protected event IEntity GetEditorEntity(int playerId)
	{
		IEntity editorEntity = super.GetEditorEntity(playerId);
		if (editorEntity && super.IsEntityActiveEditor(editorEntity)) return editorEntity;
		IEntity spectatorReceiver = CTA_SpectatorVoiceProxyComponent.GetListenerEntity(playerId);
		if (spectatorReceiver) return spectatorReceiver;
		return editorEntity;
	}

	override protected event vector GetEditorWorldLocation(int playerId)
	{
		IEntity editorEntity = GetEditorEntity(playerId);
		if (editorEntity && editorEntity.FindComponent(CTA_SpectatorVoiceProxyComponent)) return editorEntity.GetOrigin();
		return super.GetEditorWorldLocation(playerId);
	}

	override bool IsEntityActiveEditor(IEntity entity)
	{
		if (entity && entity.FindComponent(CTA_SpectatorVoiceProxyComponent)) return true;
		return super.IsEntityActiveEditor(entity);
	}
}

// Hand over before GM opens, rather than disconnecting it afterwards during deletion.
// 削除時にGM接続を巻き込まないよう、GMが開く前に受信処理を引き渡す。
modded class SCR_EditorManagerEntity
{
	override void Open(bool showErrorNotification = true)
	{
		if (!IsOpened() && IsAuthority() && CanOpen())
			CTA_SpectatorVoiceProxyComponent.StopForPlayer(GetPlayerID());
		super.Open(showErrorNotification);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void ToggleServer(bool open)
	{
		if (open && Replication.IsServer() && CanOpen())
			CTA_SpectatorVoiceProxyComponent.StopForPlayer(GetPlayerID());
		super.ToggleServer(open);
	}
}
