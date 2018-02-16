echo "going to make"
cd ~/ece344/os161/kern
cd conf/
./config ASST3
cd ../compile/ASST3
make depend
make clean
make
make install


cd ~/ece344/root

