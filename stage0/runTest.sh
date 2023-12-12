#!/bin/bash

ls | grep -q "testing"
if [ $? -eq 1 ] ; then
    echo "running setupTest.sh script"
    ./setupTest.sh
else
    rm -rf testing/output testing/diffs
    mkdir testing/output
    mkdir testing/diffs
	mkdir testing/ldiffs
	
    mkdir testing/output/asm
    mkdir testing/output/lst

    if [ $# -eq 0 ] ; then
        echo "run all tests"
        count="$(ls -1 testing/dat)"
        for i in $count ; do
            cleaned=${i:0:3}
            ./stage0 testing/dat/$cleaned.dat testing/output/lst/$cleaned.lst testing/output/asm/$cleaned.asm
            ls testing/motl/asm | grep -q $cleaned
            if [ $? -eq 0 ] ; then
                diff testing/motl/asm/$cleaned.asm testing/output/asm/$cleaned.asm > testing/diffs/$cleaned.diff 2>/dev/null
				diff testing/motl/lst/$cleaned.lst testing/output/lst/$cleaned.lst > testing/ldiffs/$cleaned.diff 2>/dev/null
            fi
        done
    else 
        echo $#
        for i in $@ ; do
            echo $i
            ./stage0 testing/dat/$i.dat testing/output/lst/$i.lst testing/output/asm/$i.asm
            diff testing/motl/asm/$i.asm testing/output/asm/$i.asm > testing/diffs/$i.diff 2>/dev/null
			diff testing/motl/lst/$i.lst testing/output/lst/$i.lst > testing/ldiffs/$i.diff 2>/dev/null
        done
    fi
fi

rm output diffs ldiffs
ln -s testing/output output
ln -s testing/diffs diffs
ln -s testing/ldiffs ldiffs
