SAN="-fsanitize=address -fsanitize=undefined -g"
FLAGS="$SAN -Wall -Wextra"
gcc main.c $FLAGS
