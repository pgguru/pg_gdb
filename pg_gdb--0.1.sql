CREATE FUNCTION gdb(breakpoints text[]) RETURNS int LANGUAGE C
AS 'MODULE_PATHNAME', $function$attach_gdb$function$;

CREATE FUNCTION gdb() RETURNS int AS $$SELECT gdb(NULL)$$ language sql;
CREATE FUNCTION gdb(breakpoint text) RETURNS int AS $$SELECT gdb(array[breakpoint])$$ language sql;
