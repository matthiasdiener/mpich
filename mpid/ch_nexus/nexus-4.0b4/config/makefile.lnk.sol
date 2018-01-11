$(LIB): $(OBJS)
	$(G_RM) -f $(LIB)
	$(G_MKSHR) -o $(LIB) $(OBJS) $(PORTS0_OBJS)

$(LIB_LITE): $(OBJS_LITE)
	$(G_RM) -f $(LIB_LITE)
	$(G_MKSHR) -o $(LIB_LITE) $(OBJS_LITE) $(PORTS0_OBJS)
