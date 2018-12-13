############################################################
#  Filename: cleanCase
#  purpose:  Clean the time folders in the present case. 
#
###########################################################

parallel=0
serial=1
echo "Cleaning $PWD case"
#check what parts of the code to run
if [ -d "processor0" ]; then
	parallel=1	
fi

#Delete the files
if [ $parallel -eq 1 ];then 
  	for f in processor*
		do rm -rf processor* >/dev/null 2>&1
  	done
	echo "parallel case is cleaned"
fi
if [ $serial -eq 1 ];then
	rm -rf ./[1-9]* ./0.[0-9]* >/dev/null 2>&1
	echo "serial case is cleaned"
fi

rm -rf constant/polyMesh
rm -rf postProcessing
rm -rf 0
