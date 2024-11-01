#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

PG_MODULE_MAGIC;

static char *debugger_command;

PG_FUNCTION_INFO_V1(attach_gdb);

void
_PG_init()
{
	DefineCustomStringVariable("pg_gdb.command",
							   "Debugger command to run; a single %d is required for the pid.",
							   NULL,
							   &debugger_command,
							   "screen -X screen -t gdb_window gdb -p %d",
							   PGC_SUSET,
							   0,
							   NULL,
							   NULL,
							   NULL);
}


/*
 * Check that our debugger command is valid
 */
static bool
is_valid_debugger_command(char *command)
{
	int pct_count = 0;

	if (!command)
		return false;

	/*
	 * We require a single %d for our process id and no other %-escapes (other
	 * than perhaps %%).
	 */

	while (*command)
	{
		if (*command == '%')
		{
			switch (*++command) {

			case 'd':
				pct_count++;
				break;
			case '%':
				/* allow double-percent for some kind of escaped commands? */
				break;
			default:
				return false;
			}
		command++;
		}
	}

	/* only valid if we have a single %d */
	return pct_count == 1;
}

Datum
attach_gdb(PG_FUNCTION_ARGS)
{
  if (!is_valid_debugger_command(debugger_command))
	  ereport(ERROR, (errmsg("invalid debugger provided in pg_gdb.command"),
					  errhint("command must have only a single %%d escape for process id")));

  char *interpolated_command = psprintf(debugger_command, MyProcPid);

  if (fork() == 0) {
	  /* launch debugger using /bin/sh */
	  execl("/bin/sh", "sh", "-c", command, (char *)NULL);
	  _exit(1);  // If execl fails, exit the child process
  }

  PG_RETURN_VOID();
}
