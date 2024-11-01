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

