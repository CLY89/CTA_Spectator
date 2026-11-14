class RespawnLimitCommand:ScrServerCommand{
	//------------------------------------------------------------------------------------------------
	// Specify keyword of command
	override string GetKeyword()
	{
		return "respawnLimit";
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
	protected ScrServerCmdResult SetRespawnLimit(notnull array<string> argv, int playerId = 0)
	{
		if (argv.Count() < 2)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.PARAMETERS);

		int newRespawnLimit = argv[1].ToInt();
		if (newRespawnLimit < 0)
			return ScrServerCmdResult("Not Valid Respawn Limit Number", EServerCmdResultType.ERR);
		CTA_RespawnLimiterComponent respawnLimiter = CTA_RespawnLimiterComponent.GetInstance();
		int oldRespawnLimit = respawnLimiter.GetRespawnLimit();
		respawnLimiter.SetRespawnLimit(newRespawnLimit);
		PrintFormat("Set Respawn Limit %1 -> %2",oldRespawnLimit,newRespawnLimit, level: LogLevel.NORMAL);

		return ScrServerCmdResult(string.Format("Set Respawn Limit %1 -> %2",oldRespawnLimit,newRespawnLimit), EServerCmdResultType.OK);
	}

	//------------------------------------------------------------------------------------------------
	// Handle Chat command on server
	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		return SetRespawnLimit(argv, playerId);
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
		return SetRespawnLimit(argv);
	}

	//------------------------------------------------------------------------------------------------
	// Handle Pending command
	override ref ScrServerCmdResult OnUpdate()
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
}