#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "storage/fd.h"
#include "utils/builtins.h"
#include "utils/guc.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

PG_MODULE_MAGIC;

static char *debugger_command;

PG_FUNCTION_INFO_V1(attach_gdb);

static char *write_cstring_to_file(const char *content);


void
_PG_init()
{
	DefineCustomStringVariable("pg_gdb.command",
							   "Debugger command to run; a single %d is required for the pid.",
							   NULL,
							   &debugger_command,
							   "screen -X screen -t gdb_window gdb -p %d",
							   PGC_USERSET,
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
		}
		command++;
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

	/*
	 * Construct our basic debugger command; if no breakpoints are passed in,
	 * then this is what we run.  If breakpoints are passed in, then we
	 * construct a custom gdb file that contains the breakpoint(s) we want and
	 * automatically continue on execution.
	 */

	char *command = psprintf(debugger_command, MyProcPid);

	/* Add breakpoints if needed */
	if (!PG_ARGISNULL(0))
	{
		ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
		Datum *elements;
		bool *nulls;
		int num_elements;
		int i;
		StringInfoData debuggerFileCommands;
		int breakpointCount = 0;

		if (ARR_ELEMTYPE(array) != TEXTOID)
			ereport(ERROR,
					(errmsg("Expected text array")));


		initStringInfo(&debuggerFileCommands);

		/* Deconstruct array into individual elements */
		deconstruct_array(array, TEXTOID, -1, false, 'i', &elements, &nulls, &num_elements);

		/* Iterate over elements */
		for (i = 0; i < num_elements; i++)
		{
			if (!nulls[i])
			{
				char *breakpoint = TextDatumGetCString(elements[i]);
				breakpointCount++;
				appendStringInfo(&debuggerFileCommands, "break %s\n", breakpoint);
			}
		}

		if (breakpointCount > 0)
		{
			appendStringInfoString(&debuggerFileCommands, "continue\n");

			/* write out to temporary file */
			char *commandFile = write_cstring_to_file(debuggerFileCommands.data);

			/* append our new option */
			command = psprintf("%s -x %s", command, commandFile);
		}
	}

	if (fork() == 0) {
		/* launch debugger using /bin/sh */
		execl("/bin/sh", "sh", "-c", command, (char *)NULL);
		_exit(1);  // If execl fails, exit the child process
	}

	PG_RETURN_VOID();
}

/* Helper to write a cstring to a unique temporary file, returning the filename */
static char *
write_cstring_to_file(const char *content)
{
	File file;
	char *filename;
	int bytes_written;
	static int version = 0;

	/* Generate unique filename based on DataDir and process ID */
	filename = psprintf("/tmp/gdb_commands_%d_%d.tmp", MyProcPid, ++version);

	/* Open file for writing with read access for other processes */
	file = PathNameOpenFile(filename, O_CREAT | O_WRONLY | O_TRUNC);
	if (file < 0)
		ereport(ERROR, (errmsg("could not create file: %s", filename)));

	/* Write content */
	bytes_written = FileWrite(file, content, strlen(content), 0, 0);
	if (bytes_written != strlen(content))
	{
		FileClose(file);
		ereport(ERROR, (errmsg("could not write all data to file: %s", filename)));
	}

	/* Close file */
	FileClose(file);

	/* Return the filename */
	return filename;
}
