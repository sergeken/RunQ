mv runq runq.bak
./runq.bak collect -S10 -m3 >test.log 2>&1 &
for i in 1 2 3 4 6 7 8
do
	sleep 1
	make clean
	make
done
