#! /bin/bash

echo "Looking for any time directories without results (e.g. stray functionObjectProperties files, see issue #26 on GitHub)..."

for case_dir in fluid heater1 heater2 heater3 heater4; do
    cd $case_dir
    for f in [0-9]* [0-9]*.[0-9]*; do
    	if ! [ -f $f/U ] && ! [ -f $f/T ]; then
    		rm -rfv $f
    	fi
    done
    if [ -d processor0 ]; then
    	for g in processor*; do
    		cd $g
    		for f in [0-9]* [0-9]*.[0-9]*; do
    			if ! [ -f $f/U ] && ! [ -f $f/T ]; then
    				rm -rfv $f
    			fi
    		done
    		cd ..
    	done
    fi
    cd ..
done

