pg_gdb
======

**NOTE: This is pre-release software; the SQL API will not be stable until version 1.0**

This extension makes it easy to launch a debugger attached to the current backend process.

```
postgres# CREATE EXTENSION pg_gdb;
postgres# SELECT gdb();
```

When you run the `gdb()` function, it will run the command `pg_gdb.command` with
the process id of the current backend.


Configuration
-------------

The default command run is:

```
screen -X screen -t gdb_window gdb -p %d
```

You can customize this using the `pg_gdb.command` GUC.  The command is scanned
to ensure that it has a single `%d` format specifier, which is replaced with the
pid of the current backend and then executed in a forked process.

The default command is appropriate if you are already running inside the
`screen` multiplexer; it will launch a new `screen` window for running `gdb`.  A
similar comamnd could be written if running inside `tmux` or another
multiplexer.

Why not just `gdb`?  While you could `SET pg_gdb.command = 'gdb -p %d';` and
this would pass the check for a valid `pg_gdb.command`, doing so does not work
since `psql` keeps control of the terminal meaning the gdb process immediately
exits due to being run non-interactively.


Breakpoints
-----------

You can automatically invoke `gdb` with breakpoints and continue by passing in a
`text` or `text[]` argument into the `gdb()` function.  This works by creating a
temporary file that `gdb` executes with the `-x` flag.

Example:

```
postgres# SELECT gdb('errfinish');
postgres# SELECT 1/0; /* breaks in debugger */
```

The breakpoint name is automatically interpolated in a `break %s` line in a
generated debugger commands file, so you can provide anything that `gdb` can
recognize after a `break` statement.


Symbols
-------

You can inspect the symbols in the current process using the `proc_sym()`
function.  This takes a pid (defaulting to this process) and returns a list of
symbols that it could read for this process image and its loaded shared
libraries.  This can be used with the `gdb()` functions to load breakpoints
based on a pattern, for instance:

```
postgres# select proc_sym from proc_sym() where proc_sym ~* 'tuple';
             proc_sym
-----------------------------------
 BuildColumnDefListForTupleDesc
 BuildTupleDescriptorForRelation
 BuildTupleDescriptorForTargetList
 ...
 TupleDescToProjectionList
postgres# select gdb(array_agg(proc_sym)) from proc_sym() where proc_sym ~* 'tuple';
```

This would launch `gdb` with all of the above breakpoints set.

This functionality currently only exists on Linux.


Signals
-------

We will probably add support for a signals argument to allow you to generate
`handle` statements in addition to the breakpoint argument.  (We may also end up
doing something more generic, like allow you to provide additional contents in
the template gdb file.)


Notes
-----

This currently assumes that `gdb` is in path; if not, customize the
`pg_gdb.command` variable.

It should be obvious, but this should only be installed on development machines,
with superuser access since you are interactively launching a debugger on the
same system.

It can however be used in situations where your terminal is attached, for
instance when debugging a complicated test in a `make check` pipeline, where
attaching to the dynamically-created database instance is inconvenient.  In this
case, if you run something like this in the setup:

```
postgres# CREATE EXTENSION IF NOT EXISTS pg_gdb;
postgres# GRANT EXECUTE ON FUNCTION gdb() TO myuser;
```

Your test user can then execute `SELECT gdb();` to break into a debugger in the
course of the test run.
