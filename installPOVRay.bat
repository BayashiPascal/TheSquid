sudo apt-get -y update && \
sudo apt-get -y install libboost-all-dev && \
sudo apt-get -y install zlib1g-dev && \
sudo apt-get -y install libpng-dev && \
sudo apt-get -y install libjpeg8-dev && \
sudo apt-get -y install libopenexr-dev && \
sudo apt-get -y install libtiff5-dev libtiff5 libjbig-dev && \
sudo apt-get -y install autoconf && \
cd ~/GitHub && \
git clone https://github.com/POV-Ray/povray.git && \
cd ~/GitHub/povray/unix/ && \
git checkout 3.7-stable && \
./prebuild.sh && \
cd ../ && \
./configure COMPILED_BY="TheSquid <email@address>"  --with-boost-libdir=/usr/lib/arm-linux-gnueabihf  && \
make && \
sudo make install && \
make check
