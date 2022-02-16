make config=release
./bin/packer
mkdir -p release
cp res.pck release
cp bin/openmv release
cp bin/libcore.so release
cp liblogic.so release
cp libdialogue.so release
