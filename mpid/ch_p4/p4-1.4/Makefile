# top level makefile for p4

#MPIARCH = @MPIARCH@
#MPIARCH = sun4
#P4ARCH=SUN4

SHELL=/bin/sh

MFLAGS = 
#MFLAGS = -i    # for alliant fx/8

P4_DIRS = util lib alog usc usc/usctest include bin servers 

EXAMPLES_DIRS = messages 

ALL_DIRS = $(P4_DIRS) $(EXAMPLES_DIRS) 

DIRS = $(ALL_DIRS)

RM = /bin/rm

all:
	@echo making p4 on architecture=$(P4ARCH)
	@make makefiles P4ARCH=$(P4ARCH)
	@make $(MFLAGS) links P4ARCH=$(P4ARCH)
	@for dir in $(DIRS) ;\
	  do \
	  echo " " ;\
	  echo making p4 in directory $$dir ;\
	  (cd $$dir; make $(MFLAGS) default) ;\
	  done 

p4:
	@echo making p4 on architecture=$(P4ARCH)
	@make makefiles P4ARCH=$(P4ARCH)
	@make $(MFLAGS) links P4ARCH=$(P4ARCH)
	@for dir in $(P4_DIRS) ;\
	  do \
	  echo " " ;\
	  echo making p4 in directory $$dir ;\
	  (cd $$dir; make $(MFLAGS) default) ;\
	  done 

p4inmpi: 
	@echo making p4 on architecture=$(P4ARCH)
	@make p4makefiles P4ARCH=$(P4ARCH)
	@make $(MFLAGS) p4links P4ARCH=$(P4ARCH)
	@for dir in $(P4_DIRS) ;\
	  do \
	  echo " " ;\
	  echo making p4 in directory $$dir ;\
	  (cd $$dir; make $(MFLAGS) default) ;\
	  done 
	(cd lib;  make $(MFLAGS) MPIARCH=$(MPIARCH) p4inmpi; cd ..)
	(cd alog; make $(MFLAGS) MPIARCH=$(MPIARCH) p4inmpi; cd ..)
	(cd usc;  make $(MFLAGS) MPIARCH=$(MPIARCH) p4inmpi; cd ..)

makefiles:
	@(cd util; make -f makefile.proto defs.MD P4ARCH=$(P4ARCH))
	@echo "making Makefiles in " $(DIRS)
	@for dir in $(DIRS) ;\
	  do \
	  cat util/defs.MD $$dir/makefile.proto > $$dir/Makefile ;\
	  done 

p4makefiles:
	@(cd util; make -f makefile.proto defs.MD P4ARCH=$(P4ARCH))
	@echo "making Makefiles in " $(P4_DIRS)
	@for dir in $(P4_DIRS) messages ;\
	  do \
	  cat util/defs.MD $$dir/makefile.proto > $$dir/Makefile ;\
	  done 

#  @echo "#define  P4_$_MACHINE_TYPE \"$(P4ARCH)\"" >> lib/p4_config.h
config_hdr:
	@echo "#define P4_COMPILED_TIME  \"`date`\"" > lib/p4_config.h
	@echo "#define  P4_MACHINE_TYPE \"$(P4ARCH)\"" >> lib/p4_config.h
	@echo "#ifndef  $(P4ARCH)" >> lib/p4_config.h
	@echo "#define  $(P4ARCH)" >> lib/p4_config.h
	@echo "#endif  "            >> lib/p4_config.h
	@cat OPTIONS >> lib/p4_config.h

links:
	@if [ -f lib/p4_config.h ] ;\
	then\
	    true ;\
	else\
	    make config_hdr P4ARCH=$(P4ARCH) ;\
	fi
	@if [ -f include/alog.h ] ;\
	then\
	    true ;\
	else\
	    (cd include ;\
	    ln -s ../alog/alog.h . ;\
	    ln -s ../alog/alog_evntdfs.h . ;\
	    cd ..) ;\
	fi
	@if [ -f include/usc.h ] ;\
	then\
	    true ;\
	else\
	    (cd include ;\
	    ln -s ../usc/usc.h . ;\
	    ln -s ../usc/usc_sys.h . ;\
	    cd ..) ;\
	fi
	@if [ -f include/p4.h ] ;\
	then\
	    true ;\
	else\
	    (cd include ;\
	    ln -s ../lib/p4.h . ;\
	    ln -s ../lib/p4_MD.h . ;\
	    ln -s ../lib/p4_config.h . ;\
	    ln -s ../lib/p4_defs.h . ;\
	    ln -s ../lib/p4_funcs.h . ;\
	    ln -s ../lib/p4_globals.h . ;\
	    ln -s ../lib/p4_macros.h . ;\
	    ln -s ../lib/p4_mon.h . ;\
	    ln -s ../lib/p4_patchlevel.h . ;\
	    ln -s ../lib/p4_sock_util.h . ;\
	    ln -s ../lib/p4_sr.h . ;\
	    ln -s ../lib/p4_sys.h . ;\
	    ln -s ../lib/p4_sys_funcs.h . ;\
	    ln -s ../lib/sfcdchi.h . ;\
	    cd ..) ;\
	fi
	@if [ -f messages_f/p4f.h ] ;\
	then\
		true ;\
	else\
		(cd messages_f; ln -s ../lib_f/p4f.h . ; cd ..) ;\
	fi
	@if [ -f contrib_f/p4f.h ] ;\
	then\
		true ;\
	else\
		(cd contrib_f; ln -s ../lib_f/p4f.h . ; cd ..) ;\
	fi

p4links:
	@if [ -f lib/p4_config.h ] ;\
	then\
	    true ;\
	else\
	    make config_hdr P4ARCH=$(P4ARCH) ;\
	fi
	@if [ -f include/alog.h ] ;\
	then\
	    true ;\
	else\
	    (cd include ;\
	    ln -s ../alog/alog.h . ;\
	    ln -s ../alog/alog_evntdfs.h . ;\
	    cd ..) ;\
	fi
	@if [ -f include/usc.h ] ;\
	then\
	    true ;\
	else\
	    (cd include ;\
	    ln -s ../usc/usc.h . ;\
	    ln -s ../usc/usc_sys.h . ;\
	    cd ..) ;\
	fi
	@if [ -f include/p4.h ] ;\
	then\
	    true ;\
	else\
	    (cd include ;\
	    ln -s ../lib/p4.h . ;\
	    ln -s ../lib/p4_MD.h . ;\
	    ln -s ../lib/p4_config.h . ;\
	    ln -s ../lib/p4_defs.h . ;\
	    ln -s ../lib/p4_funcs.h . ;\
	    ln -s ../lib/p4_globals.h . ;\
	    ln -s ../lib/p4_macros.h . ;\
	    ln -s ../lib/p4_mon.h . ;\
	    ln -s ../lib/p4_patchlevel.h . ;\
	    ln -s ../lib/p4_sock_util.h . ;\
	    ln -s ../lib/p4_sr.h . ;\
	    ln -s ../lib/p4_sys.h . ;\
	    ln -s ../lib/p4_sys_funcs.h . ;\
	    ln -s ../lib/sfcdchi.h . ;\
	    cd ..) ;\
	fi


clean:  
	@for dir in $(DIRS) ;\
	  do \
	  echo cleaning $$dir ;\
	  (cd $$dir; if [ -s Makefile ] ; then make clean ; fi ) ;\
	  done
	$(RM) -f *~ *.o *.bak

dist:	realclean   

realclean:
	$(RM) -f *~ *.o *.bak
	@for dir in $(DIRS) ;\
	  do \
	  echo cleaning $$dir ;\
	  (cd $$dir; if [ -s Makefile ] ; then make realclean ; fi ) ;\
	  done

Makefiles: makefiles

INSTALLDIR = ./$(P4ARCH)
install:
	@echo installing p4 in $(INSTALLDIR)
	@if [ -d $(INSTALLDIR) ] ;\
	then\
	    echo 'using existing directory $(INSTALLDIR)' ;\
	else\
	    mkdir $(INSTALLDIR) ;\
	fi
	@cp Makefile $(INSTALLDIR)
	@mkdir $(INSTALLDIR)/util
	@cp util/* $(INSTALLDIR)/util
	@mkdir $(INSTALLDIR)/include
	@cp include/*.h $(INSTALLDIR)/include
	@mkdir $(INSTALLDIR)/lib
	@cp lib/libp4.a $(INSTALLDIR)/lib
	@grep RANLIB util/defs.MD > $(INSTALLDIR)/lib/makeranlib
	@echo "doran:" >> $(INSTALLDIR)/lib/makeranlib
	@echo "	$$(RANLIB) libp4.a" >> $(INSTALLDIR)/lib/makeranlib
	@(cd  $(INSTALLDIR)/lib; make -f makeranlib doran; $(RM) makeranlib)
	@mkdir $(INSTALLDIR)/bin
	@cp bin/* $(INSTALLDIR)/bin
	@mkdir $(INSTALLDIR)/examples
	@cp messages/sr_test.c messages/sr_user.h  \
	  messages/systest.c messages/makefile.proto \
	  messages/run_ipsc messages/run_ncube messages/run_delta \
	  messages/sr_test.pg messages/systest.pg $(INSTALLDIR)/examples
	@sed -e "s@P4_HOME_DIR = ..@P4_HOME_DIR = $(INSTALLDIR)@" \
	  $(INSTALLDIR)/examples/makefile.proto >  $(INSTALLDIR)/temp
	@mv $(INSTALLDIR)/temp  $(INSTALLDIR)/examples/makefile.proto
	@make makefiles DIRS=$(INSTALLDIR)/examples
	@(cd $(INSTALLDIR)/examples; make sr_test systest)
	@mkdir $(INSTALLDIR)/lib_f
	@cp lib_f/libp4_f.a lib_f/*.h $(INSTALLDIR)/lib_f
	@grep RANLIB util/defs.MD > $(INSTALLDIR)/lib_f/makeranlib
	@echo "doran:" >> $(INSTALLDIR)/lib_f/makeranlib
	@echo "	$$(RANLIB) libp4_f.a" >> $(INSTALLDIR)/lib_f/makeranlib
	@(cd  $(INSTALLDIR)/lib_f; make -f makeranlib doran; $(RM) makeranlib)
	@mkdir $(INSTALLDIR)/examples_f
	@cp messages_f/sr_test.f messages_f/p4f.h  \
	  messages_f/makefile.proto \
	  messages_f/sr_test.pg $(INSTALLDIR)/examples_f
	@sed -e "s@P4_HOME_DIR = ..@P4_HOME_DIR = $(INSTALLDIR)@" \
	  $(INSTALLDIR)/examples_f/makefile.proto >  $(INSTALLDIR)/temp
	@mv $(INSTALLDIR)/temp  $(INSTALLDIR)/examples_f/makefile.proto
	@make makefiles DIRS=$(INSTALLDIR)/examples_f
	@(cd $(INSTALLDIR)/examples_f; make sr_test)
	@echo installed p4 in $(INSTALLDIR)
