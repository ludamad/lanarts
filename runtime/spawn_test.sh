set -e
#./run.sh -g -b --sanitize
#./run.sh -O -g -b # -lj
#./run.sh -O -g -b -lj
./run.sh -g -b # -lj

RUNTIME_FOLDER=`pwd`
RUNTIME_TMP=`mktemp -d`
# Clean up on exit:
trap "rm -rf $RUNTIME_TMP" EXIT
# Set up a temporary lanarts run folder so that we can have a fresh saves folder:
cd $RUNTIME_TMP
for component in $RUNTIME_FOLDER/* ; do
    ln -s $component "$RUNTIME_TMP"
done
# Place the expected test configuration in the saves folder:
rm -f saves
rm -f settings.yaml
mkdir saves
echo "# See settings.yaml for details
ip: localhost
username: TestBot
steps_per_draw: 10
time_per_step: 0
frame_action_repeat: 0
port: 6112
regen_level_on_death: yes
autouse_health_potions: no
autouse_mana_potions: no
connection_type: none" > saves/saved_settings.yaml

echo "#Online settings
username: TestBot
lobby_server_url: http://putterson.homedns.org:8080

#Window settings
fullscreen: no
view_width: 640
view_height: 480
#Font settings
font: fonts/Gudea-Regular.ttf
menu_font: fonts/alagard_by_pix3m-d6awiwp.ttf

#Performance settings
steps_per_draw: 1 #More is almost guaranteed to make the game faster, 1 is ideal
free_memory_while_idle: no

#Debug settings
network_debug_mode: no
invincible: no #Feel like exploring ? -> yes
draw_diagnostics: no
verbose_output: no
keep_event_log: no" > settings.yaml

export LANARTS_TESTCASE=tests.OverworldExplore
export LANARTS_SMALL=1 
#export LANARTS_OVERWORLD_TEST=1
export LANARTS_SEED=123456732
#export LANARTS_2P="Stormcaller"
#export ARENA_ENEMIES="return {Ciribot=1}"
gdb -silent -ex="break lua_error" -ex="break exit" -ex="break _exit" -ex="break abort" -ex="catch throw" -ex=r --args "$RUNTIME_FOLDER/../build/src/lanarts" $args
#"$RUNTIME_FOLDER/../build/src/lanarts" $args
