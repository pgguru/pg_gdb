pg_gdb
======

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

You can customize this using the `pg_gdb.command` GUC.


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
