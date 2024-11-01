EXTENSION = pg_gdb
MODULE_big = pg_gdb
DATA = pg_gdb--0.1.sql
OBJS = pg_gdb.o 
PG_CONFIG ?= pg_config
PG_CFLAGS := -Wno-missing-prototypes -Wno-deprecated-declarations -Wno-unused-result
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
