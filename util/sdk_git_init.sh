git init
git submodule add https://github.com/veridisquot/openmv
git submodule update

rm -rf ./core/
rm -rf ./bootstrapper/

cp -r ./openmv/core ./
cp -r ./openmv/bootstrapper ./