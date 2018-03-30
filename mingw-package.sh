./run.sh --build --mingw -O
mkdir package
cp build/src/lanarts.exe package/
#cp dependencies/binary/*.dll package/
cp -r runtime/* package
cd package
wine lanarts.exe
