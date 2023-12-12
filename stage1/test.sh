#!/bin/bash
stage="stage1"

rm -rf testing

mkdir testing

mkdir testing/dat
mkdir testing/motl
mkdir testing/motl/asm
mkdir testing/motl/lst

cp /usr/local/4301/data/$stage/*.dat testing/dat
cp /usr/local/4301/data/$stage/*.asm testing/motl/asm
cp /usr/local/4301/data/$stage/*.lst testing/motl/lst