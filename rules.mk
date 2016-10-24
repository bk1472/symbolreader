clean :
	rm -f $(OBJ_DIR)/*.o
	rm -f $(TGT)

clobber : clean
	rm -rf $(BUILD_DIR)

$(OBJ_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.d : %.c
	@echo "Making dependency $< ... $@"
	@test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	@(	$(CC) -MM $(CFLAGS) $<												\
	   | sed     -f $(SCRPT_DIR)/mkdep.sed									\
	   | grep    -v "^ \\\\"												\
	   | sed     -e "s\$(<:.c=.o)\$@ $(OBJ_DIR)/$(<:.c=.o)\g"				\
	) > $@

$(TGT) : $(OBJS)
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
	$(CC) -o $@ $(OBJS) -lz

ifeq ($(OBJS),)
INCLUDE_DEPEND	?= 0
else
INCLUDE_DEPEND	?= 1
endif

ifeq ($(MAKECMDGOALS),)
ifeq ($(INCLUDE_DEPEND),1)
-include $(OBJS:.o=.d)
$(TGT) : $(OBJS:.o=.d)
endif
endif
