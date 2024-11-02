pg_gdb
======

This extension makes it easy to launch a debugger attached to the current backend process.

```
postgres= CREATE EXTENSION pg_gdb;
postgres= SELECT gdb();
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
postgres= SELECT gdb('errfinish');
postgres= SELECT 1/0; /* breaks in debugger */
```

The breakpoint name is automatically interpolated in a `break %s` line in a
generated debugger commands file, so you can provide anything that `gdb` can
recognize after a `break` statement.

