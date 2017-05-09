#!/usr/local/bin/bash

FIRST_PLAYER=/tmp/pipe1
SECOND_PLAYER=/tmp/pipe2
OTHER_PLAYERS=/tmp/pipe3

# флаг для третьего и более игроков
NOT_ONLY_SHOW=true

term_rows=$(tput lines)
term_cols=$(tput cols)
size_field=$((term_cols / 5))
offset=$((size_field / 3))
offset_rows=$(( (term_rows - size_field) / 2 ))
offset_cols=$(( (term_cols - size_field) / 2 ))
cur_line=0
cur_steps_count=0

# Храним тут статус игры. Всего 9 клеток, нумерация с 0
# Игоровое поле выглядит так:
# 0 | 1 | 2
# - - - - -
# 3 | 4 | 5
# - - - - -
# 6 | 7 | 8
# GAME_STATUS будет состоять из 9 элементов
# Тогда, например, при ходе первого игрока 1 1, запишем в GAME_STATUS[1 * 3 + 1] = 1
GAME_STATUS=()

swipe_pipes() {
	local tmp=$FIRST_PLAYER
    FIRST_PLAYER=$SECOND_PLAYER
    SECOND_PLAYER=$tmp
}

init_game() {
    # При выходе удаляем все pipe
    trap "rm -f $FIRST_PLAYER && rm -f $SECOND_PLAYER && rm -f $OTHER_PLAYERS" exit

    if [[ ! -p $FIRST_PLAYER ]]; then
        waiting_enemy=1
        mkfifo $FIRST_PLAYER
    elif [[ ! -p $SECOND_PLAYER ]]; then
        # Как только появляется п=второй игрок - игра начинается
        waiting_enemy=0
        mkfifo $SECOND_PLAYER
        # Необходимо для второго игрока поменять местами pipe, чтобы код ниже работал корректно
        swipe_pipes
    elif [[ -p $FIRST_PLAYER ]] && [[ -p $SECOND_PLAYER ]]; then
        if [[ ! -p $OTHER_PLAYERS ]]; then
            mkfifo $OTHER_PLAYERS
        fi
        NOT_ONLY_SHOW=false
    fi
}

set_cursor() {
    tput cup $1 $2
}

draw_playing_field() {
    clear

    local local_offset_rows=$offset_rows
    set_cursor $local_offset_rows $offset_cols $size_field

    for (( i=1; i <= $size_field - 2; i++ )); do
        for (( j=1; j <= $size_field - 2; j++ )); do
            if [[ $i -eq 1 ]] || [[ $((i % offset)) -eq 0 ]]; then
                echo "-\c"
        	elif [[ $j -eq 1 ]] || [[ $((j % offset)) -eq 0 ]]; then
        		set_cursor $((local_offset_rows)) $((j + offset_cols - 1))
        		echo "*\c"
        	fi
        done

        ((local_offset_rows++))
        set_cursor $local_offset_rows $offset_cols
    done
}

check_win_combination() {
  if [[ ${GAME_STATUS[$1]} -eq $4 ]] && [[ ${GAME_STATUS[$2]} -eq $4 ]] && [[ ${GAME_STATUS[$3]} -eq $4 ]]; then
    return 0
  fi

  return 1
}

check_game_status() {
    if check_win_combination 0 3 6 1 || check_win_combination 1 4 7 1 || check_win_combination 2 5 8 1 || 
       check_win_combination 0 1 2 1 || check_win_combination 3 4 5 1 || check_win_combination 6 7 8 1 ||
       check_win_combination 0 4 8 1 || check_win_combination 2 4 6 1
    then
        echo "YOU WIN!"
        return 1
    fi

    if check_win_combination 0 3 6 2 || check_win_combination 1 4 7 2 || check_win_combination 2 5 8 2 ||
       check_win_combination 0 1 2 2 || check_win_combination 3 4 5 2 || check_win_combination 6 7 8 2 ||
       check_win_combination 0 4 8 2 || check_win_combination 2 4 6 2
    then
        echo "YOU LOST!"
        return 1
    fi

    return 0
}

draw_player_step() {
    local x=$1
    local y=$2
    local move_x=$((offset_rows + $offset * x + $offset / 2))
    local move_y=$((offset_cols + $offset * y + $offset / 2))
    set_cursor ${move_x} ${move_y}

    if [[ $3 -eq 1 ]]; then
        echo 'X'
        GAME_STATUS[$((x * 3 + y))]=1
    else
        echo 'O'
        GAME_STATUS[$((x * 3 + y))]=2
    fi
    ((cur_steps_count++))

    set_cursor $cur_line 0
}

game_cycle() {
    draw_player_step $a $b $1
    
    check_game_status
    
    # Если был выигрыш или проигрыш, выходим из игры
    if [[ $? -eq 1 ]]; then
    	sleep 7
    	clear
    	exit
    elif [[ $cur_steps_count -ge 9 ]]; then
        # Обрабатываем ничью
        echo "Dead heat"
        sleep 7
        clear
        exit
    fi
    
    waiting_enemy=$1
}

is_right_step() {
    # Проверяем введеные игроками ходы
    if ! [[ $a =~ ^[0-9]+$ ]] || ! [[ $b =~ ^[0-9]+$ ]] ||
       [[ $a -gt 2 ]] || [[ $a -lt 0 ]] ||
       [[ $b -gt 2 ]] || [[ $b -lt 0 ]]; then
           return 0
    fi

    return 1
}

init_game
draw_playing_field
set_cursor 0 0


while true
    do
        if $NOT_ONLY_SHOW; then
            echo $waiting_enemy
            if [[ $waiting_enemy -eq 1 ]]; then
                echo "Wait enemy step"
                ((cur_line++))
                stty -echo
                if read a b < $FIRST_PLAYER; then
                    game_cycle $((1 - waiting_enemy))
                fi
            else
                echo 'Do your step:'
                cur_line=$((cur_line + 2))
                stty echo
                if read a b; then
                    is_right_step
                    if [[ $? -eq 0 ]]; then
                        echo 'Wrong step ! Try again'
                        ((cur_line++))
                    else
                        echo $a $b > $SECOND_PLAYER
                        game_cycle $((1 - waiting_enemy))
                    fi
                fi
            fi
        else
            echo $FIRST_PLAYER $SECOND_PLAYER
            if read a b < $FIRST_PLAYER; then
                echo 1
                game_cycle 0
            elif read a b < $SECOND_PLAYER; then
                echo 1
                # game_cycle 0
            fi
        fi
    done