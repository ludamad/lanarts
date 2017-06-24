export ARENA_ENEMIES='return {["'$ENEMY'"] = 1}'
export LANARTS_LEVEL=3
export LANARTS_GO="White Mage"
#export LANARTS_2P="White Mage"
#export LANARTS_3P=Necromancer
#export LANARTS_HARD_QUIT=1
cd ..
#./run.sh -O -lj $@
#./run.sh $@
./run.sh $@ -g # --sanitize
