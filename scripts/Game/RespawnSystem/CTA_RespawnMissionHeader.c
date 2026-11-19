modded class SCR_MissionHeader
{
	[Attribute("2", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(CTA_RespawnType), category: "CTA_Spectator")]
	CTA_RespawnType m_eRespawnType;

	[Attribute("1", params: "0 inf", desc: "Initial lives per player", category: "CTA_Spectator")]
	int m_iInitialLives;

	[Attribute("1", params: "0 inf", desc: "Initial tickets per faction", category: "CTA_Spectator")]
	int m_iInitialFactionTickets;
}
