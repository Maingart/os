rm -rf ./dist
git fetch && cmake -B ./dist
cd ./dist && make
