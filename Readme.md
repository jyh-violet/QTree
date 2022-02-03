gcc src/* -fPIC -shared -o qtree.so

```console

# install libconfig
wget https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.3.tar.gz --no-check-certificate
tar -zxvf libconfig-1.7.3.tar.gz
cd libconfig-1.7.3/
./configure
make
make check
make install DESTDIR=/home/jyh/libs

#install papi
git clone https://bitbucket.org/icl/papi.git
cd papi/src
./configure
make
make install DESTDIR=/home/jyh/libs

#install lost package
wget http://archive.ubuntu.com/ubuntu/pool/main/libp/libpfm4/libpfm4_4.9.0-2_amd64.deb
dpkg-deb -x libpfm4_4.9.0-2_amd64.deb  /home/jyh/libs/usr/local/lib/

#change .bashrc
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/jyh/libs/usr/local/lib
```
#install

mkdir build
cd build
cmake  -DCMAKE_INSTALL_PREFIX=/home/jyh/libs/usr/local ..
cmake --configure ..
cmake --build ..
cmake --build .. --target install 