SUBDIRS = stack portings auto_app brlinkd

.PHONY:all
.PHONY:install
.PHONY:clean
.PHONY:distclean
.PHONY:subdirs $(SUBDIRS)
.PHONY:TAGS

clean:
	$(MAKE) -C $(SUBDIRS) clean

subdirs:$(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

