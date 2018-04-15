export LD_LIBRARY_PATH=./.out/lib/

./.out/bin/emi_core -d
sleep 1
./test/stress_test/stress_receiver &
sleep 1
./test/stress_test/stress_sender
pkill emi_core
pkill stress_receiver
