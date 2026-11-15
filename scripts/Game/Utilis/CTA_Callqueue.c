class CTA_CallQueue{
    protected static ref ScriptCallQueue m_Callqueue = new ScriptCallQueue();

    static ScriptCallQueue GetCallQueue(){
        return m_Callqueue;
    }
}