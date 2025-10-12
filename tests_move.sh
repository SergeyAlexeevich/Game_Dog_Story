#!/bin/bash

TOKENS()

for i in {1..100}; do
  username="Player_$i"
  response=$(curl -s -X POST http://127.0.0.1:8080/api/v1/game/join -H "Content-Type: application/json" -d "{\"userName\": \"$username\", \"mapId\": \"map1\"}")
  authToken=$(echo $response | grep -oP '"authToken":\s?"\K[^"]+')  
  echo "Player $username joined with token: $authToken"
  TOKENS+=($authToken)
done

# Сохраняем остальные аргументы
TICK=150
REPEATS=101
TIME=0

# Массив возможных направлений движения
MOVES=("U" "D" "L" "R")

# Функция для получения случайного направления
get_random_move() {
    echo "${MOVES[$RANDOM % ${#MOVES[@]}]}"
}

# Основной цикл
for ((i=1; i<=$REPEATS; i++)); do
    TIME=$((TIME + TICK))
    echo "--- Итерация $i ---"
    
    # Обрабатываем все токены
    for ((k=0; k<${#TOKENS[@]}; k++)); do
        TOKEN=${TOKENS[k]}
        echo "Обработка токена $TOKEN"
        
        # Выполняем движение
        echo "Отправка движения..."
        curl -s -X POST http://localhost:8080/api/v1/game/player/action \
            -H "Content-Type: application/json" \
            -H "Authorization: Bearer $TOKEN" \
            -d "{\"move\": \"$(get_random_move)\"}"
    done
    
    # Выполняем общий тик
    echo "Отправка тика..."
    curl -s -X POST http://localhost:8080/api/v1/game/tick \
        -H "Content-Type: application/json" \
        -d "{\"timeDelta\": $TICK}"
    
    echo "Прошло: $TIME миллисекунд"
    echo "------------------------"
    
    # Небольшая пауза между итерациями
    #sleep 1
done

echo "Все итерации выполнены."
