class RespawnTypeCommand:ScrServerCommand{
	//------------------------------------------------------------------------------------------------
	// Specify keyword of command
	override string GetKeyword()
	{
		return "respawnType";
	}

	//------------------------------------------------------------------------------------------------
	// Run command server-side
	override bool IsServerSide()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Set requirement to admin permission via RCON
	override int RequiredRCONPermission()
	{
		return ERCONPermissions.PERMISSIONS_ADMIN;
	}

	//------------------------------------------------------------------------------------------------
	// Set requirement to be logged in administrator for chat command
	override int RequiredChatPermission()
	{
		return EPlayerRole.ADMINISTRATOR;
	}

	//------------------------------------------------------------------------------------------------
	// Shared handle for kicking player for RCON and Chat command
	protected ScrServerCmdResult HandleCommand(notnull array<string> argv, int playerId = 0)
	{
		if (argv.Count() < 2)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.PARAMETERS);

		string argText = argv[1];
		argText.ToLower();
		CTA_RespawnLimitType newRespawnType;
		switch(argText){
			case "individual":
				newRespawnType = CTA_RespawnLimitType.Individual;	
				break;
			case "ticket":
				newRespawnType = CTA_RespawnLimitType.Ticket;	
				break;
			case "unlimited":
				newRespawnType = CTA_RespawnLimitType.Unlimited;	
				break;
			default:	
				return ScrServerCmdResult(string.Format("Invaild Arg Text:%1",argText), EServerCmdResultType.ERR);
		}
		CTA_RespawnLimiterComponent respawnLimiter = CTA_RespawnLimiterComponent.GetInstance();
		CTA_RespawnLimitType oldRespawnType = respawnLimiter.GetRespawnLimitType();
		respawnLimiter.SetRespawnType(newRespawnType);
		PrintFormat("Set Respawn Type: %1 -> %2",oldRespawnType,newRespawnType, level: LogLevel.NORMAL);

		return ScrServerCmdResult(
			string.Format("Set Respawn Type: %1 -> %2",
			typename.EnumToString(CTA_RespawnLimitType,oldRespawnType),
			typename.EnumToString(CTA_RespawnLimitType,newRespawnType)),
		 EServerCmdResultType.OK);
	}

	//------------------------------------------------------------------------------------------------
	// Handle Chat command on server
	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		return HandleCommand(argv, playerId);
	}

	//------------------------------------------------------------------------------------------------
	// Handle Chat command on client
	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}

	//------------------------------------------------------------------------------------------------
	// Handle RCON command on server
	override ref ScrServerCmdResult OnRCONExecution(array<string> argv)
	{
		return HandleCommand(argv);
	}

	//------------------------------------------------------------------------------------------------
	// Handle Pending command
	override ref ScrServerCmdResult OnUpdate()
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
}