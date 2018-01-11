/*
 *  rdb_file_parser.c
 *
 *  This is for parsing database and startup files.  Since their syntax
 *  is identical, it makes no sense to put the logic that parses them in
 *  two locations.  This is more than a simple function call, though
 *  because the logic determining what to do with the values is
 *  completely different.  For instance, database_file_lookup() returns
 *  a char *, but _nx_database_add_startup_file() returns a
 *  nexus_list_t *.
 *
 *  Therefore, it has been decided to #include this file into the
 *  correct location in these two functions.  For function specific
 *  logic, #ifdef's have been used.  Currently, only
 *  database_file_lookup() and _nx_database_add_startup_file() use this
 *  file.  Convention is to #define the function name that includes this
 *  file.
 *
 *  Recognized #define's are:
 *   A.  _RDB_FILE_LOOKUP--causes it to look for only one node name, and
 *       when it finds that name, it parses the attribute list and adds
 *       that name to the hash table.
 *
 *       Calls database_file_name_found(),
 *       _nx_parse_attributes(), and
 *       _nx_flush_rest_of_line().
 *
 *   B.  _RDB_ADD_STARTUP_FILE--causes it to parse out all the node
 *       names and attributes, adds them all to the hash table, and
 *       returns a list of node specifiers.
 *
 *       Calls startup_add_to_cur_node(), startup_add_to_node_list(),
 *       _nx_parse_attributes(), and
 *       _nx_flush_rest_of_line().
 *
 *   C.  _RDB_STARTUP_INITIAL_NODES--causes it to parse out all the node
 *       names from startup_arg_list and return a list of node
 *       specifiers.
 *
 *       Calls startup_add_to_node_list().
 *
 *   D.  _RDB_GET_NAMES--causes it to parse out all the node names from
 *       a file and return a list of node specifiers.
 *
 *       Calls _nx_add_to_node_list().
 *
 *
 *  BNF of the database file is:
 *
 *  node_name       ::= string
 *  attribute_key   ::= string
 *  attribute_value ::= string
 *  end_of_line     ::= '\n'
 *  continue_line   ::= '\\' end_of_line
 *  whitespace_char ::= ' '  |
 *                      '\t' |
 *                      continue_line
 *  whitespace      ::= whitespace_char*
 *  node_specifer   ::= node_name
 *  node_list       ::= node_specifer[[whitespace] ':' [whitespace]node_specifier]*
 *  attribute       ::= attribute_key |
 *                      attribute_key '=' attribute_value
 *  file_line       ::= end_of_line                  |
 *                      '#' [character*] end_of_line | 
 *                      node_list [whitespace attribute]* end_of_line
 *  file_format     ::= file_line*
 *
 *  This translates to the following finite state automata used as the
 *  basis for parsing the file into names and attributes (done as best
 *  possible in ASCII):
 *
 *  next character here refers to all valid ASCII characters except for:
 *    a.  '#'
 *    b.  ','
 *    c.  ':'
 *    d.  ' '
 *    e.  '\t'
 *    f.  '\\'
 *    g.  '\n'
 *
 *  STATE 1: (starting point)
 *   get first character -> STATE 2
 *
 *  STATE 2:
 *   add current character to name
 *   get next character -> STATE 2
 *   get whitechar      -> STATE 5
 *   get ':'            -> STATE 6
 *
 *  STATE 5:
 *   set name done
 *   get whitechar      -> STATE 5
 *   get ':'            -> STATE 6
 *   get next character -> STATE 8
 *
 *  STATE 6: (node spec follows)
 *   process node spec
 *   set name undone
 *   get next character -> STATE 2
 *   get whitechar      -> STATE 7
 *
 *  STATE 7:
 *   get next character -> STATE 2
 *   get whitechar      -> STATE 7
 *
 *  STATE 8: (end--attribute list follows)
 *   process node spec
 *
 *  Yes, I know this has control here, too, but it helps better explain
 *  what happens below without having to redo it for yourself.
 */
/* 
 * rcsid = "$Header: /nfs/globus1/src/master/rdb/rdb_file_parser.c,v 1.9 1996/12/11 18:24:33 tuecke Exp $"
 */
{
#define S_LEN 4096
    char input_line[S_LEN];
#if defined(_RDB_ADD_STARTUP_FILE) || defined(_RDB_FILE_LOOKUP)
    nexus_list_t *attr;
#endif

    nexus_bool_t colon;
    nexus_bool_t name_started, name_done;
    nexus_bool_t done;

    int i;
    char *name_ptr, *real_name;
    char *eof_marker;

#if defined(_RDB_ADD_STARTUP_FILE) || \
    defined(_RDB_FILE_LOOKUP) 
    nexus_rdb_hash_entry_t node;
#endif
#ifdef _RDB_FILE_LOOKUP
    nexus_bool_t name_found;
    char *value;
#endif

    while(NEXUS_TRUE)
    {
        done = NEXUS_FALSE;
        colon = NEXUS_FALSE;
        name_done = name_started = NEXUS_FALSE;
        name_ptr = input_line;
        real_name = NULL;

	eof_marker = NULL;

#if defined(_RDB_ADD_STARTUP_FILE) || \
    defined(_RDB_FILE_LOOKUP)
        node.name = NULL;
        node.next = NULL;
        node.attr = NULL;
#endif
#ifdef _RDB_FILE_LOOKUP
	name_found = NEXUS_FALSE;
#endif

#ifndef _RDB_STARTUP_INITIAL_NODES
    next_line:
#endif

#ifdef _RDB_FILE_LOOKUP
        eof_marker = fgets(input_line, S_LEN, rdb_file->fp);
#endif
#ifdef _RDB_ADD_STARTUP_FILE
        eof_marker = fgets(input_line, S_LEN, startup_file);
#endif
#ifdef _RDB_STARTUP_INITIAL_NODES
	eof_marker = (char *) !eof_marker;
	strcpy(input_line, arg_nodes_list);
	/*
	 *  We are doing two things with this line:
	 *
	 *  1.  Ending the input_line so the program doesn't think it
	 *      has been overrun.
	 *  2.  Put a ':' at the end to fool the parser.  Since the
	 *      next_line label has been removed, after the parser gets
	 *      to the new_line, it jumps out of the loop without
	 *      adding a null node.  It is effectively using the colon
	 *      as an end of list marker.
	 */
	strcat(input_line, ":\n");
#endif
#ifdef _RDB_GET_NAMES
	eof_marker = fgets(input_line, S_LEN, fp);
#endif
        if (!eof_marker) break;

        if (input_line[0] == '#' || input_line[0] == '\n')
        {
#ifdef _RDB_STARTUP_INITIAL_NODES
	    nexus_fatal("rdb_file_parser(): Invalid -nodes argument\n");
#else
	    goto next_line;
#endif
        }
        if (input_line[strlen(input_line)-1] != '\n')
        {
	    input_line[S_LEN-1] = '\0';
	    nexus_fatal("rdb_file_parser():  Database line length too long.  Must be less than %d characters.  Bad line started: %s\n", S_LEN, input_line);
        }

        for (i = 0; !done && input_line[i]; i++)
        {
	    switch(input_line[i])
	    {
	      case ':':
	        if (colon)
	        {
		    nexus_fatal("db_file_parser(): Improper database format: 2 consecutive ':'s\n");
	        }
	        colon = NEXUS_TRUE;
	        input_line[i] = '\0';
	        /* node spec is complete */
#ifdef _RDB_FILE_LOOKUP
	        rdb_file_name_found(&name_found,
			            node_name, &real_name,
				    name_ptr, &node);
#endif
#if defined(_RDB_ADD_STARTUP_FILE) || \
    defined(_RDB_STARTUP_INITIAL_NODES) || \
    defined(_RDB_GET_NAMES)
	        if (!real_name)
	        {
		    real_name = _nx_copy_string(name_ptr);
	        }
#endif
#ifdef _RDB_ADD_STARTUP_FILE
	        startup_add_to_cur_node(&node, real_name);
#endif
	        /* 
	         * we are about to start the next name, so set
	         * name_started = NEXUS_FALSE; and
		 * name_done = NEXUS_FALSE
	         */
	        name_done = name_started = NEXUS_FALSE;
		/* SJT
	        name_ptr = (char *)((int)input_line + i + 1);
		*/
	        name_ptr = input_line + i + 1;
	        if (real_name)
	        {
#if defined(_RDB_ADD_STARTUP_FILE) || \
    defined(_RDB_STARTUP_INITIAL_NODES)
		    startup_add_to_node_list(&node_list, real_name);
#elif defined(_RDB_GET_NAMES)
		    rdb_add_to_node_list(&names, real_name);
#endif
	            NexusFree(real_name);
		    real_name = NULL;
	        }
	        break;
	      case ' ':
	      case '\t':
		if (name_started)
	        {
		    input_line[i] = '\0';
		    name_done = NEXUS_TRUE;
		    if (!real_name)
		    {
		        real_name = _nx_copy_string(name_ptr);
		    }
	        }
		/* SJT
	        name_ptr = (char *)((int)input_line + i + 1);
		*/
	        name_ptr = input_line + i + 1;
	        break;
	      case '\\':
		if (name_started)
	        {
		    input_line[i] = '\0';
		    name_done = NEXUS_TRUE;
		    if (!real_name)
		    {
		        real_name = _nx_copy_string(name_ptr);
		    }
	        }
		name_ptr = input_line;
#ifdef _RDB_STARTUP_INITIAL_NODES
		nexus_fatal("rdb_file_parser():  Invalid line continuation character in -nodes list\n");
#else
	        goto next_line;
#endif
	        break;
#ifndef JGG
	      case '\n':
		input_line[i] = '\0';
		name_done = NEXUS_TRUE;
		if (!real_name)
		{
		    real_name = _nx_copy_string(name_ptr);
		}
		/* let this fall through to set node_list done, too */
#endif /* JGG */
	      default:
	        if (!name_done)
	        {
		    /* add this char to name */
		    colon = NEXUS_FALSE;
		    name_started = NEXUS_TRUE;
	        }
	        else
	        {
		    /* node list is complete */
		    done = NEXUS_TRUE;
#ifdef _RDB_FILE_LOOKUP
	            rdb_file_name_found(&name_found,
			                node_name, &real_name,
				        name_ptr, &node);
#endif
#if defined(_RDB_ADD_STARTUP_FILE) || \
    defined(_RDB_STARTUP_INITIAL_NODES)
		    if (real_name)
		    {
		        startup_add_to_node_list(&node_list, real_name);
		    }
#endif
#ifdef _RDB_ADD_STARTUP_FILE
		    startup_add_to_cur_node(&node, real_name);
#endif
#ifdef _RDB_GET_NAMES
		    rdb_add_to_node_list(&names, real_name);
#endif
	        }
	        break;
	    }
        }
#ifdef _RDB_FILE_LOOKUP
        if (name_found)
        {
	    attr = _nx_rdb_parse_attributes(input_line, S_LEN,
					    name_ptr, rdb_file->fp);
	    _nx_rdb_hash_table_add(node.name, attr);
	    if (_nx_rdb_hash_table_lookup(node_name, key, &value))
	    {
	        /*
	         * If this didn't return it might be possible to make
		 * this file a true function instead of being #included
		 * in the right spot
	         */
	        nexus_stdio_unlock();
	        return (value);
	    }
        }
        else
        {
	    _nx_rdb_flush_rest_of_line(input_line, S_LEN,
				       name_ptr, rdb_file->fp);
        }
#endif
#ifdef _RDB_ADD_STARTUP_FILE
        attr = _nx_rdb_parse_attributes(input_line, S_LEN,
					name_ptr, startup_file);
        _nx_rdb_hash_table_add_nodes_with_attrs(&node, attr);
#endif
#ifdef _RDB_STARTUP_INITIAL_NODES
	break;
#endif
#ifdef _RDB_GET_NAMES
	_nx_rdb_flush_rest_of_line(input_line, S_LEN, name_ptr, fp);
#endif
    }
}
