echo Compilando app...
sleep 3
cd app/Debug
make clean
make all
cd ../..
echo Compilando cliente...
sleep 3
cd cliente/Debug
make clean
make all
cd ../..
echo Compilando comanda...
sleep 3
cd comanda/Debug
make clean
make all
cd ../..
echo Compilando restaurante...
sleep 3
cd restaurante/Debug
make clean
make all
cd ../..
echo Compilando sindicato...
sleep 3
cd sindicato/Debug
make clean
make all
cd ../..

