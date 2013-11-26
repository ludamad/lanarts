# Bash function to apply a color to a piece of text.
function colorify {
    local words;
    words=$(cat)
    echo -e "\e[$1m$words\e[0m"
}

# Bash function to check for a flag in 'args' and remove it.
# Treats 'args' as one long string.
# Returns true if flag was removed.
function handle_flag() {
    flag=$1
    if [[ "$args" == *"$flag"* ]] ; then
        args="${args/$flag/}"
        return 0 # True!
    fi
    return 1 # False!
}

