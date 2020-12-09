echo Compilando app...
sleep 1
cd app/Debug
make clean
make all
cd ../..
echo Compilando cliente...
sleep 1
cd cliente/Debug
make clean
make all
cd ../..
echo Compilando comanda...
sleep 1
cd comanda/Debug
make clean
make all
cd ../..
echo Compilando restaurante...
sleep 1
cd restaurante/Debug
make clean
make all
cd ../..
echo Compilando sindicato...
sleep 1
cd sindicato/Debug
make clean
make all
cd ../..

