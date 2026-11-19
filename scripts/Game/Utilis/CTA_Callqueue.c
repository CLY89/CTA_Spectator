[ComponentEditorProps(category: "CTA", description: "Schedules delayed calls")]
class CTA_CallQueueClass : ScriptComponentClass {}

class CTA_CallQueue : ScriptComponent
{
	protected static CTA_CallQueue s_Instance;
	protected ref ScriptCallQueue m_Callqueue = new ScriptCallQueue();

	static ScriptCallQueue GetCallQueue()
	{
		if (!s_Instance)
		{
			Print("[CTA] CTA_CallQueue component is not initialized.", LogLevel.ERROR);
			return null;
		}

		return s_Instance.m_Callqueue;
	}

	override void OnPostInit(IEntity owner)
	{
		s_Instance = this;
		SetEventMask(owner, EntityEvent.FIXEDFRAME);
	}

	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		m_Callqueue.Tick(timeSlice);
	}

	override void OnDelete(IEntity owner)
	{
		if (s_Instance == this)
			s_Instance = null;

		super.OnDelete(owner);
	}
}
