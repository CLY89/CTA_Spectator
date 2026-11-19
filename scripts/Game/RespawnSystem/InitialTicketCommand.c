class InitialTicketCommand:ScrServerCommand
{
	override string GetKeyword()
	{
		return "initialTicket";
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

	protected ScrServerCmdResult SetInitialTickets(notnull array<string> argv)
	{
		if (argv.Count() < 2)
			return ScrServerCmdResult(string.Empty, EServerCmdResultType.PARAMETERS);

		int parsedLength;
		int initialTickets = argv[1].ToInt(parsed: parsedLength);
		if (parsedLength == 0 || initialTickets < 0)
			return ScrServerCmdResult("Initial ticket count must be a non-negative integer", EServerCmdResultType.ERR);

		CTA_RespawnSystemComponent respawnSystem = CTA_RespawnSystemComponent.GetInstance();
		if (!respawnSystem)
			return ScrServerCmdResult("Respawn management is not initialized", EServerCmdResultType.ERR);

		int oldInitialTickets = respawnSystem.GetInitialFactionTickets();
		if (!respawnSystem.SetInitialFactionTickets(initialTickets))
			return ScrServerCmdResult("Unable to set initial ticket count", EServerCmdResultType.ERR);

		PrintFormat("Set Initial Tickets %1 -> %2", oldInitialTickets, initialTickets, level: LogLevel.NORMAL);
		return ScrServerCmdResult(string.Format("Set Initial Tickets %1 -> %2", oldInitialTickets, initialTickets), EServerCmdResultType.OK);
	}

	override ref ScrServerCmdResult OnChatServerExecution(array<string> argv, int playerId)
	{
		return SetInitialTickets(argv);
	}

	override ref ScrServerCmdResult OnChatClientExecution(array<string> argv, int playerId)
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}

	override ref ScrServerCmdResult OnRCONExecution(array<string> argv)
	{
		return SetInitialTickets(argv);
	}

	override ref ScrServerCmdResult OnUpdate()
	{
		return ScrServerCmdResult(string.Empty, EServerCmdResultType.OK);
	}
}
