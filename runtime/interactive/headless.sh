SCRIPT_DIR=$(readlink -f `dirname $0`)
LANARTS_HEADLESS=1 "$SCRIPT_DIR/"lua.sh $@
