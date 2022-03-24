git submodule update --remote

rm -rf ./core/
rm -rf ./bootstrapper/

cp -r ./openmv/core ./
cp -r ./openmv/bootstrapper ./
