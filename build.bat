cl /c /O2 /Oy /DNDEBUG /Fotables.obj tables.cpp
cl /c /O2 /Oy /DNDEBUG /Fohashbench.obj hashbench.cpp
link /OUT:hashbench.exe hashbench.obj tables.obj

.\hashbench -m > figure-1-data.txt
python plot.py figure-1-data.txt figure-1.png
.\hashbench -w > figure-2-data.txt
python plot.py figure-2-data.txt figure-2.png

.\hashbench > hashbench-data.txt
python plot_speed.py hashbench-data.txt
