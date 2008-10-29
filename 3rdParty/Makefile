include $(ISISROOT)/make/isismake.macros

.PHONY: libs plugins

all:  install

install: libs plugins

libs:
	@for library in $(THIRDPARTYLIBS); do \
	  echo "  [Installing $$library...]"; \
	  $(INSTALL3P) $(INSTALL3POPTS) $$library $(ISISROOT)/3rdParty/lib/; \
	done;

plugins:
	@for plugs in $(THIRDPARTYPLUGINS); do \
	  echo "  [Installing $$plugs...]"; \
	  $(INSTALL3P) $(INSTALL3POPTS) $$plugs $(ISISROOT)/3rdParty/plugins/; \
	done;

clean:
	rm -f lib/lib*.so*
	@for plugs in plugins/*; do \
	  if [ -d $$plugs -a $$plugs != "plugins/CVS" ]; \
	  then \
	    $(RM) -rf $$plugs; \
	  fi \
	done;
	
