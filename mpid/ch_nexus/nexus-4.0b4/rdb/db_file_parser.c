/*
 *  file_parser.c
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
 *   A.  _NX_DB_FILE_LOOKUP--causes it to look for only one node name, and
 *       when it finds that name, it parses the attribute list and adds
 *       that name to the hash table.
 *
 *       Calls database_file_name_found(),
 *       _nx_database_parse_attributes(), and
 *       _nx_database_flush_rest_of_line().
 *
 *   B.  _NX_DB_ADD_STARTUP_FILE--causes it to parse out all the node
 *       names and attributes, adds them all to the hash table, and
 *       returns a list of node specifiers.
 *
 *       Calls startup_add_to_cur_node(), startup_add_to_node_list(),
 *       _nx_database_parse_attributes(), and
 *       _nx_database_flush_rest_of_line().
 *
 *   C.  _NX_STARTUP_INITIAL_NODES--causes it to parse out all the node
 *       names and return a list of node specifiers.
 *
 *       Calls startup_add_to_node_list().
 *
 *
 *  BNF of the database file is:
 *
 *  node_name       ::= string
 *  node_number     ::= integer
 *  node_count      ::= integer
 *  attribute_key   ::= string
 *  attribute_value ::= string
 *  end_of_line     ::= '\n'
 *  continue_line   ::= '\\' end_of_line
 *  whitespace_char ::= ' '  |
 *                      '\t' |
 *                      continue_line
 *  whitespace      ::= whitespace_char*
 *  node_specifer   ::= node_name['#' node_number][',' node_count]
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
 *   get '#'            -> STATE 3
 *   get ','            -> STATE 4
 *   get whitechar      -> STATE 5
 *   get ':'            -> STATE 6
 *
 *  STATE 3:
 *   set name done
 *   get next digits and set number = atoi(digits)
 *   set number done
 *   get ','            -> STATE 4
 *   get whitechar      -> STATE 5
 *   get ':'            -> STATE 6
 *
 *  STATE 4:
 *   set name done
 *   get next digits and set count = atoi(digits)
 *   set count done
 *   get whitechar      -> STATE 5
 *   get ':'            -> STATE 6
 *
 *  STATE 5:
 *   set name done
 *   set number done
 *   set count done
 *   get whitechar      -> STATE 5
 *   get ':'            -> STATE 6
 *   get next character -> STATE 8
 *
 *  STATE 6: (node spec follows)
 *   process node spec
 *   set name undone
 *   set number undone
 *   set count undone
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
 * rcsid = ""
 */
{
#define S_LEN 4096
    char input_line[S_LEN];
    
#if defined(_NX_DB_FILE_LOOKUP) || defined(_NX_DB_ADD_STARTUP_FILE)
    nexus_list_t *attr;
#endif

    nexus_bool_t pound, comma, colon;
    nexus_bool_t name_done;
    nexus_bool_t done;

    int i;
    int number, count;
    char *name_ptr, *real_name;
    char *eof_marker;

#if defined(_NX_DB_ADD_STARTUP_FILE) || \
    defined(_NX_DB_FILE_LOOKUP)
    nexus_db_hash_entry_t node;
#endif
#ifdef _NX_DB_FILE_LOOKUP
    nexus_bool_t name_found;
    char *value;
#endif

    while(NEXUS_TRUE)
    {
        done = NEXUS_FALSE;
        pound = comma = colon = NEXUS_FALSE;
        name_done = NEXUS_FALSE;
        name_ptr = input_line;
        real_name = NULL;
        count = number = NEXUS_NO_NODE_NUM;

	eof_marker = NULL;

#if defined(_NX_DB_ADD_STARTUP_FILE) || \
    defined(_NX_DB_FILE_LOOKUP)
        node.name = NULL;
        node.number = NEXUS_NO_NODE_NUM;
        node.count = NEXUS_NO_NODE_NUM;
        node.next = NULL;
        node.attr = NULL;
#endif
#ifdef _NX_DB_FILE_LOOKUP
	name_found = NEXUS_FALSE;
#endif

#ifndef _NX_STARTUP_INITIAL_NODES
    next_line:
#endif

#ifdef _NX_DB_FILE_LOOKUP
        eof_marker = fgets(input_line, S_LEN, db_file->fp);
#endif
#ifdef _NX_DB_ADD_STARTUP_FILE
        eof_marker = fgets(input_line, S_LEN, startup_file);
#endif
#ifdef _NX_STARTUP_INITIAL_NODES
	eof_marker = (char *) !eof_marker;
	strcpy(input_line, arg_nodes_list);
	/*
	 *  We are doing two things with this line:
	 *
	 *  1.  Ending the input_line so the program doesn't think it
	 *      has been overrun.
	 *  2.  Put a '\n' at the end to fool the parser.  Since the
	 *      next_line label has been removed, after the parser gets
	 *      to the new_line, it jumps out of the loop without
	 *      adding a null node.  It is effectively using the colon
	 *      as an end of list marker.
	 */
	strcat(input_line, "\n");
#endif
        if (!eof_marker) break;

        if (input_line[0] == '#' || input_line[0] == '\n')
        {
#ifdef _NX_STARTUP_INITIAL_NODES
	    nexus_fatal("db_file_parser(): Invalid -nodes argument\n");
#else
	    goto next_line;
#endif
        }
        if (input_line[strlen(input_line)-1] != '\n')
        {
	    nexus_fatal("db_file_parser():  Database line length too long.  Must be less than %d characters\nLine was:'%s'", S_LEN,input_line);
        }

        for (i = 0; !done && input_line[i]; i++)
        {
	    switch(input_line[i])
	    {
	      case '#':
	        if (pound)
	        {
		    nexus_fatal("db_file_parser(): Improper database format:  2 consecutive '#'s, or '#' follows ','\n");
	        }
	        input_line[i++] = '\0';
	        number = atoi(((char *)(input_line)) + i);
	        /* skip past number */
	        while (isdigit(input_line[i])) i++; 
	        /* Go back to the last digit */
	        i--;
	        pound = NEXUS_TRUE;
	        name_done = NEXUS_TRUE;
	        real_name = _nx_copy_string(name_ptr);
	        break;
	      case ',':
	        if (comma)
	        {
		    nexus_fatal("db_file_parser(): Improper database format: 2 consecutive ','s\n");
	        }
	        input_line[i++] = '\0';
	        count = atoi(((char *)(input_line)) + i);
	        /* skip past number */
	        while (isdigit(input_line[i])) i++;
	        /* Go back to the last digit */
	        i--;
	        comma = NEXUS_TRUE;
	        /* pound cannot follow comman, so: */
	        pound = NEXUS_TRUE;
	        name_done = NEXUS_TRUE;
	        if (!real_name)
	        {
		    real_name = _nx_copy_string(name_ptr);
	        }
	        break;
	      case ':':
	        if (colon)
	        {
		    nexus_fatal("db_file_parser(): Improper database format: 2 consecutive ':'s\n");
	        }
	        colon = NEXUS_TRUE;
	        input_line[i] = '\0';
	        /* # and , can't follow :, so */
	        pound = comma = NEXUS_TRUE;
	        /* node spec is complete */
#ifdef _NX_DB_FILE_LOOKUP
	        database_file_name_found(&name_found,
			                 node_name, &real_name,
					 name_ptr, &node,
					 number, count);
#endif
#if defined(_NX_DB_ADD_STARTUP_FILE) || \
    defined(_NX_STARTUP_INITIAL_NODES)
	        if (!real_name)
	        {
		    real_name = _nx_copy_string(name_ptr);
	        }
#endif
#ifdef _NX_DB_ADD_STARTUP_FILE
	        startup_add_to_cur_node(&node, real_name,
				        number, count);
#endif
	        /* 
	         * we are about to start the next name, so don't set
	         * name_done = NEXUS_TRUE;
	         */
	        name_done = NEXUS_FALSE;
	        name_ptr = ((char *)(input_line)) + i + 1;
	        if (real_name)
	        {
#if defined(_NX_DB_ADD_STARTUP_FILE) || \
    defined(_NX_STARTUP_INITIAL_NODES)
		    startup_add_to_node_list(&node_list,
		    			     real_name,
					     number,
					     count);
#endif
	            NexusFree(real_name);
		    real_name = NULL;
	        }
	        count = number = NEXUS_NO_NODE_NUM;
	        break;
	      case ' ':
	      case '\t':
	        if (!pound)
	        {
		    input_line[i] = '\0';
		    name_done = NEXUS_TRUE;
		    if (!real_name)
		    {
		        real_name = _nx_copy_string(name_ptr);
		    }
		    /* name cannot have number or count, so: */
		    comma = pound = NEXUS_TRUE;
	        }
	        name_ptr = ((char *)(input_line)) + i + 1;
	        break;
	      case '\\':
	        if (!pound)
	        {
		    input_line[i] = '\0';
		    name_done = NEXUS_TRUE;
		    if (!real_name)
		    {
		        real_name = _nx_copy_string(name_ptr);
		    }
		    /* name cannot have number or count, so: */
		    comma = pound = NEXUS_TRUE;
	        }
#ifdef _NX_STARTUP_INITIAL_NODES
		nexus_fatal("db_file_parser():  Invalid line continuation character in -nodes list\n");
#else
	        goto next_line;
#endif
	        break;
	      case '\n':
		input_line[i] = '\0';
		name_done = NEXUS_TRUE;
		if (!real_name)
		{
		    real_name = _nx_copy_string(name_ptr);
		}
		/* name cannot have number or count, so: */
		comma = pound = NEXUS_TRUE;
		/* let this fall through to set node_list done, too */
	      default:
	        if (!name_done)
	        {
		    /* add this char to name */
		    colon = comma = pound = NEXUS_FALSE;
	        }
	        else
	        {
		    /* node list is complete */
		    done = NEXUS_TRUE;
#ifdef _NX_DB_FILE_LOOKUP
	        database_file_name_found(&name_found,
			                 node_name, &real_name,
					 name_ptr, &node,
					 number, count);
#endif
#if defined(_NX_DB_ADD_STARTUP_FILE) || \
    defined(_NX_STARTUP_INITIAL_NODES)
		    if (real_name)
		    {
		        startup_add_to_node_list(&node_list,
						 real_name,
						 number,
						 count);
		    }
#endif
#ifdef _NX_DB_ADD_STARTUP_FILE
		    startup_add_to_cur_node(&node, real_name,
					    number, count);
#endif
	        }
	        break;
	    }
        }
#ifdef _NX_DB_FILE_LOOKUP
        if (name_found)
        {
	    attr = _nx_database_parse_attributes(input_line, S_LEN,
					         name_ptr, db_file->fp);
	    _nx_database_hash_table_add(node.name,
				        node.number, node.count,
				        attr);
	    if ((value = _nx_database_hash_table_lookup(node_name,
							node_number,
							key)))
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
	    _nx_database_flush_rest_of_line(input_line, S_LEN,
				            name_ptr, db_file->fp);
        }
#endif
#ifdef _NX_DB_ADD_STARTUP_FILE
        attr = _nx_database_parse_attributes(input_line, S_LEN,
				             name_ptr, startup_file);
        _nx_database_hash_table_add_nodes_with_attrs(&node, attr);
#endif
#ifdef _NX_STARTUP_INITIAL_NODES
	break;
#endif
    }
}
