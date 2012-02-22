CXX=g++-apple-4.2
CXXFLAGS=-O3 -g -Isparsehash-install/include -DNDEBUG

# To run plot.py, you need Python with matplotlib. Set the python executable to
# use below.
#
# If you have MacPorts, you can:
#     sudo port install py27-matplotlib
# This doesn't install matplotlib in your Mac's system python installation.
# Instead it installs a copy of python in /opt/local/bin (or wherever you've
# configured ports to install stuff) and that copy has matplotlib.
#
PYTHON=/opt/local/bin/python2.7

SPEED_IMAGES=\
  InsertSmallTest-speed.png \
  InsertLargeTest-speed.png \
  LookupHitTest-speed.png \
  LookupMissTest-speed.png

all: figure-1.png figure-2.png $(SPEED_IMAGES)

figure-1.png: figure-1-data.txt plot.py
	$(PYTHON) plot.py $< $@

figure-2.png: figure-2-data.txt plot.py
	$(PYTHON) plot.py $< $@

figure-1-data.txt: hashbench
	./hashbench -m > $@

figure-2-data.txt: hashbench
	./hashbench -w > $@

$(SPEED_IMAGES): hashbench-data.txt plot_speed.py
	$(PYTHON) plot_speed.py $<

hashbench-data.txt: hashbench
	./hashbench > $@

hashbench: hashbench.o tables.o
	$(CXX) -o $@ $^

hashbench.o: hashbench.cpp tables.h sparsehash-install/include/sparsehash/dense_hash_map
	$(CXX) $(CXXFLAGS) -o $@ -c $<

tables.o: tables.cpp tables.h sparsehash-install/include/sparsehash/dense_hash_map
	$(CXX) $(CXXFLAGS) -o $@ -c $<

sparsehash-sources/configure:
	svn checkout http://sparsehash.googlecode.com/svn/trunk/ sparsehash-sources

sparsehash-install/include/sparsehash/dense_hash_map: sparsehash-sources/configure
	cd sparsehash-sources && \
		./configure --prefix=$(PWD)/sparsehash-install && \
		make && \
		make install

