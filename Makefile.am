SUBDIRS = wxVillaLib src locale backgrounds buttons objects data docs templates transitions
ACLOCAL_AMFLAGS=-I.
doc_DATA = AUTHORS COPYING INSTALL README ChangeLog

run: all
	$(MAKE) -C src run

DVDStyler.app: Info.plist src/dvdstyler docs/*.zip
	-mkdir DVDStyler.app
	-mkdir DVDStyler.app/Contents
	-mkdir DVDStyler.app/Contents/MacOS
	-mkdir DVDStyler.app/Contents/Resources
	-mkdir DVDStyler.app/Contents/Resources/English.lproj
	cp Info.plist DVDStyler.app/Contents/
	echo -n "APPL????" > DVDStyler.app/Contents/PkgInfo
	cp src/dvdstyler DVDStyler.app/Contents/MacOS/DVDStyler
	cp /opt/local/etc/fonts/fonts.conf DVDStyler.app/Contents/Resources/
	cp src/rc/dvdstyler.icns DVDStyler.app/Contents/Resources/DVDStyler.icns
	cp -r backgrounds DVDStyler.app/Contents/
	cp -r buttons DVDStyler.app/Contents/
	cp -r objects DVDStyler.app/Contents/
	cp -r templates DVDStyler.app/Contents/
	cp -r data DVDStyler.app/Contents/
	-mkdir DVDStyler.app/Contents/transitions
	cp -r transitions/*.xml DVDStyler.app/Contents/transitions/
	-mkdir DVDStyler.app/Contents/rc
	cp -r src/rc/*.cur DVDStyler.app/Contents/rc/
	-mkdir DVDStyler.app/Contents/docs
	cp docs/*.zip DVDStyler.app/Contents/docs/
	-mkdir DVDStyler.app/Contents/locale
	(cd locale && cp -r ar ca cs da de el es es_ar eu fi fr hu hy it ja ko mk nb nl pl pt pt_BR ro ru sk sl sr sv tr uk uz vi zh_CN zh_TW ../DVDStyler.app/Contents/locale/)
	cp /opt/local/bin/spumux DVDStyler.app/Contents/MacOS/
	cp /opt/local/bin/dvdauthor DVDStyler.app/Contents/MacOS/
	cp /opt/local/bin/mplex DVDStyler.app/Contents/MacOS/
	cp /opt/local/bin/mkisofs DVDStyler.app/Contents/MacOS/
	cp /opt/local/bin/ffmpeg DVDStyler.app/Contents/MacOS/
	strip DVDStyler.app/Contents/MacOS/DVDStyler
	strip DVDStyler.app/Contents/MacOS/spumux
	strip DVDStyler.app/Contents/MacOS/dvdauthor
	strip DVDStyler.app/Contents/MacOS/mplex
	strip DVDStyler.app/Contents/MacOS/mkisofs
	strip DVDStyler.app/Contents/MacOS/ffmpeg

libs:
	dylibbundler -od -b -x ./DVDStyler.app/Contents/MacOS/DVDStyler -d ./DVDStyler.app/Contents/libs/
	dylibbundler -of -b -x ./DVDStyler.app/Contents/MacOS/spumux -d ./DVDStyler.app/Contents/libs/
	dylibbundler -of -b -x ./DVDStyler.app/Contents/MacOS/dvdauthor -d ./DVDStyler.app/Contents/libs/
	dylibbundler -of -b -x ./DVDStyler.app/Contents/MacOS/mplex -d ./DVDStyler.app/Contents/libs/
	dylibbundler -of -b -x ./DVDStyler.app/Contents/MacOS/mkisofs -d ./DVDStyler.app/Contents/libs/
	dylibbundler -of -b -x ./DVDStyler.app/Contents/MacOS/ffmpeg -d ./DVDStyler.app/Contents/libs/
	(cd DVDStyler.app/Contents/libs/ && rm -f libwx_*.0.2.0.dylib &&\
		ln -s libwx_baseu-3.0.dylib libwx_baseu-3.0.0.2.0.dylib &&\
		ln -s libwx_baseu_net-3.0.dylib libwx_baseu_net-3.0.0.2.0.dylib &&\
		ln -s libwx_baseu_xml-3.0.dylib libwx_baseu_xml-3.0.0.2.0.dylib &&\
		ln -s libwx_osx_cocoau_adv-3.0.dylib libwx_osx_cocoau_adv-3.0.0.2.0.dylib &&\
		ln -s libwx_osx_cocoau_core-3.0.dylib libwx_osx_cocoau_core-3.0.0.2.0.dylib &&\
		ln -s libwx_osx_cocoau_html-3.0.dylib libwx_osx_cocoau_html-3.0.0.2.0.dylib &&\
		ln -s libwx_osx_cocoau_qa-3.0.dylib libwx_osx_cocoau_qa-3.0.0.2.0.dylib &&\
		ln -s libwx_osx_cocoau_xrc-3.0.dylib libwx_osx_cocoau_xrc-3.0.0.2.0.dylib)

dmg:
	rm -f DVDStyler.dmg DVDStyler-@VERSION@-MacOSX.dmg
	hdiutil eject /dev/disk1 || echo
	hdiutil create -megabytes 100 -layout NONE DVDStyler.dmg
	hdid -nomount DVDStyler.dmg
	newfs_hfs -v DVDStyler /dev/disk1
	hdiutil eject /dev/disk1
	hdid DVDStyler.dmg
	rsync -rlv --exclude "CVS" --exclude "Makefile*" --exclude ".*" DVDStyler.app /Volumes/DVDStyler/
	cp ChangeLog /Volumes/DVDStyler/
	cp "installer/Important notes.rtf" /Volumes/DVDStyler/
	ln -s /Applications/ /Volumes/DVDStyler/Applications
	hdiutil detach /Volumes/DVDStyler/
	hdiutil convert DVDStyler.dmg -format UDZO -imagekey zlib-level=9 -o DVDStyler-@VERSION@-MacOSX.dmg
	rm -f DVDStyler.dmg
