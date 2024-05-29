MODULE=DVDStyler-3.3b4
CODE_DIR=dvdstyler-DVDStyler

rm -rf $CODE_DIR
git clone git://git.code.sf.net/p/dvdstyler/DVDStyler $CODE_DIR || exit 1
rm -f $CODE_DIR/docs/*.pdf
(cd $CODE_DIR && chmod -x src/*.h src/*.cpp buttons/*.xml buttons/deprecated/*.xml backgrounds/*.jpg)
(cd $CODE_DIR && ./autogen.sh || exit 1) || exit 1
(cd $CODE_DIR/src && bison -y -o dvdvmy.c -d -p dvdvm dvdvmy.y || exit 1) || exit 1 
(cd $CODE_DIR/src && flex -s -B -Cem -odvdvml.c -Pdvdvm dvdvml.l || exit 1) || exit 1
rm -rf $CODE_DIR/autom4te.cache/ $CODE_DIR/.git

mv "$CODE_DIR" "$MODULE"

tar cjvf $MODULE.tar.bz2 $MODULE
rm -rf $MODULE
