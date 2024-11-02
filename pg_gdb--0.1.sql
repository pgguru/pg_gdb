CREATE FUNCTION gdb(breakpoints text[]) RETURNS int LANGUAGE C
AS 'MODULE_PATHNAME', $function$attach_gdb$function$;

CREATE FUNCTION gdb() RETURNS int AS $$SELECT gdb(NULL)$$ language sql;
CREATE FUNCTION gdb(breakpoint text) RETURNS int AS $$SELECT gdb(array[breakpoint])$$ language sql;


CREATE FUNCTION proc_sym(pid int) RETURNS SETOF text LANGUAGE C
AS 'MODULE_PATHNAME', $function$process_symbols$function$;

CREATE FUNCTION proc_sym() RETURNS SETOF text AS $$SELECT proc_sym(pg_backend_pid())$$ LANGUAGE SQL;
