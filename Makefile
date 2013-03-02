include ./common.mk
all: rgis_target rcommands_target rserver_target nccommands_target rgisPlot_target tfcommands_target

install: reinstall
	ln -s $(INSTALLDIR)/Scripts/rgis.sh $(INSTALLDIR)/bin/rgis

reinstall: $(INSTALLDIR)/bin $(INSTALLDIR)/XResources/bitmaps $(INSTALLDIR)/Messages $(INSTALLDIR)/html $(INSTALLDIR)/Scripts $(INSTALLDIR)/f $(INSTALLDIR)/include $(INSTALLDIR)/lib
	$(UNIXMAKE) -C rGIS       install
	$(UNIXMAKE) -C rCommands  install
	$(UNIXMAKE) -C rServer    install
	$(UNIXMAKE) -C rgisPlot   install
	$(UNIXMAKE) -C tfCommands install
	$(UNIXMAKE) -C CMlib      install
	$(UNIXMAKE) -C DBlib      install
	$(UNIXMAKE) -C Flib       install
	$(UNIXMAKE) -C MFlib      install
	$(UNIXMAKE) -C NClib      install
	$(UNIXMAKE) -C RGlib      install
	$(UNIXMAKE) -C UIlib      install
	cp XResources/bitmaps/* $(INSTALLDIR)/XResources/bitmaps/
	cp XResources/ghaas     $(INSTALLDIR)/XResources/
	cp -rp html/*           $(INSTALLDIR)/html/
	cp Messages/*.*         $(INSTALLDIR)/Messages/
	cp Scripts/*.sh         $(INSTALLDIR)/Scripts/
	cp f/*                  $(INSTALLDIR)/f/
	chmod -R ugo+rX         $(INSTALLDIR)

uninstall:
	$(UNIXMAKE) -C rGIS       uninstall
	$(UNIXMAKE) -C rCommands  uninstall
	$(UNIXMAKE) -C CMlib      uninstall
	$(UNIXMAKE) -C DBlib      uninstall
	$(UNIXMAKE) -C Flib       uninstall
	$(UNIXMAKE) -C MFlib      uninstall
	$(UNIXMAKE) -C NClib      uninstall
	$(UNIXMAKE) -C RGlib      uninstall
	$(UNIXMAKE) -C UIlib      uninstall
	rm -rf $(INSTALLDIR)/XResources
	rm -rf $(INSTALLDIR)/Messages
	rm -rf $(INSTALLDIR)/html
	rm -rf $(INSTALLDIR)/Scripts
	rm -rf $(INSTALLDIR)/f
	rmdir  $(INSTALLDIR)/bin
	rmdir  $(INSTALLDIR)/include
	rmdir  $(INSTALLDIR)/lib
	rmdir  $(INSTALLDIR)

clean:
	$(UNIXMAKE) -i -C rGIS       clean
	$(UNIXMAKE) -i -C rgisPlot   clean
	$(UNIXMAKE) -i -C ncCommands clean
	$(UNIXMAKE) -i -C rCommands  clean
	$(UNIXMAKE) -i -C rServer    clean
	$(UNIXMAKE) -i -C tfCommands clean
	$(UNIXMAKE) -i -C CMlib      clean
	$(UNIXMAKE) -i -C DBlib      clean
	$(UNIXMAKE) -i -C NClib      clean
	$(UNIXMAKE) -i -C RGlib      clean
	$(UNIXMAKE) -i -C UIlib      clean
	$(UNIXMAKE) -i -C MFlib      clean
	$(UNIXMAKE) -i -C Flib       clean

$(INSTALLDIR)/bin:
	mkdir -p $(INSTALLDIR)/bin
$(INSTALLDIR)/XResources/bitmaps:
	mkdir -p $(INSTALLDIR)/XResources/bitmaps
$(INSTALLDIR)/Messages:
	mkdir -p $(INSTALLDIR)/Messages
$(INSTALLDIR)/html:
	mkdir -p $(INSTALLDIR)/html
$(INSTALLDIR)/Scripts:
	mkdir -p $(INSTALLDIR)/Scripts
$(INSTALLDIR)/include:
	mkdir -p $(INSTALLDIR)/include
$(INSTALLDIR)/lib:
	mkdir -p $(INSTALLDIR)/lib
$(INSTALLDIR)/f:
	mkdir -p $(INSTALLDIR)/f

rgis_target: cmlib_target dblib_target rglib_target uilib_target
	$(UNIXMAKE) -C rGIS all
rcommands_target: cmlib_target rglib_target mflib_target
	$(UNIXMAKE) -C rCommands all
rserver_target: cmlib_target rglib_target mflib_target
	$(UNIXMAKE) -C rServer  all
nccommands_target: cmlib_target nclib_target
	$(UNIXMAKE) -C ncCommands all
tfcommands_target: cmlib_target dblib_target flib_target
	$(UNIXMAKE) -C tfCommands all
rgisPlot_target: cmlib_target dblib_target
	$(UNIXMAKE) -C rgisPlot all

cmlib_target:
	$(UNIXMAKE) -C CMlib all
dblib_target:
	$(UNIXMAKE) -C DBlib all
nclib_target:
	$(UNIXMAKE) -C NClib all
rglib_target:
	$(UNIXMAKE) -C RGlib all
uilib_target:
	$(UNIXMAKE) -C UIlib all
mflib_target:
	$(UNIXMAKE) -C MFlib all
flib_target:
	$(UNIXMAKE) -C Flib all
