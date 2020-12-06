cd ~/
echo [Clonando commons...]
sleep 3
git clone https://github.com/sisoputnfrba/so-commons-library/
echo [Moviendose a carpeta...]
sleep 1
cd so-commons-library
echo [Instalando commons...]
sleep 3
sudo make install
echo [Moviendose a carpeta del repo...]
sleep 1
cd ~/workspace/tp-2020-2c-Los-Recursa2
echo [Library path shared...]
sleep 3
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspace/tp-2020-2c-Los-Recursa2/shared/Debug
echo Moviendose a carpeta shared...
sleep 1
cd shared/Debug
echo [Buildeando shared...]
sleep 3
make clean
make all
echo [Copiando compilado de las shared en /usr/lib...]
sleep 3
sudo cp libshared.so /usr/lib
echo [Volviendo a carpeta root del repo...]
sleep 1
cd ~/workspace/tp-2020-2c-Los-Recursa2