class InitialLifeCommand:ScrServerCommand
{
	override string GetKeyword()
	{
		return "initialLife";
	}

	override bool IsServerSide()
	{
		return true;
	}

	override int RequiredRCONPermission()
	{
		return ERCONPermissions.PERMISSIONS_ADMIN;
	}

	override int RequiredChatPermission()
	{
		return EPlayerRole.ADMINISTRATOR;
	}

	protected ScrServerCmdResult SetInitialLives(notnull array<string> argv)
	{
		if (argv.Count() < 2)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.PARAMETERS);

		int parsedLength;
		int initialLives = argv[1].ToInt(parsed: parsedLength);
		if (parsedLength == 0 || initialLives < 0)
			return ScrServerCmdResult("Initial life count must be a non-negative integer", EServerCmdResultType.ERR);

		CTA_RespawnSystemComponent respawnSystem = CTA_RespawnSystemComponent.GetInstance();
		if (!respawnSystem)
			return ScrServerCmdResult("Respawn management is not initialized", EServerCmdResultType.ERR);

		int oldInitialLives = respawnSystem.GetInitialLives();
		if (!respawnSystem.SetInitialLives(initialLives))
			return ScrServerCmdResult("Unable to set initial life count", EServerCmdResultType.ERR);

		PrintFormat("Set Initial Lives %1 -> %2", oldInitialLives, initialLives, level: LogLevel.NORMAL);
		return ScrServerCmdResult(string.Format("Set Initial Lives %1 -> %2", oldInitialLives, initialLives), EServerCmdResultType.OK);
	}

	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		return SetInitialLives(argv);
	}

	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}

	override ref ScrServerCmdResult OnRCONExecution(array<string> argv)
	{
		return SetInitialLives(argv);
	}

	override ref ScrServerCmdResult OnUpdate()
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
}
