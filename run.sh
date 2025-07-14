cmake -Bbuild -H.
cmake --build build -j4
cd build
make

sudo slcand -o -c -s8 /dev/ttyACM0 can0
sudo ip link set up can0

# Запуск программы с гарантированным завершением
trap 'kill $PID' SIGINT SIGTERM
./ServerPi &
PID=$!
wait $PID

# Остановка CAN-интерфейса после программы
sudo ip link set down can0
sudo pkill -f 'slcand .* /dev/ttyACM0'
