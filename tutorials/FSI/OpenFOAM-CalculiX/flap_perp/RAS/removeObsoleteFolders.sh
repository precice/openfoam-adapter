#! /bin/bash
cd Fluid
for f in *[0-9].*; do
	if ! [ -f $f/U ]; then
		rm -rf $f >/dev/null 2>&1
	fi
done
if [ -d processor0 ]; then
	for g in processor*; do
		cd $g
		for f in *[0-9].*; do
			if ! [ -f $f/U ]; then
				rm -rf $f >/dev/null 2>&1
			fi
		done
		cd ..
	done
fi
cd ..

