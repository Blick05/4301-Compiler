#!/bin/bash
rm -rf testing

mkdir testing

mkdir testing/dat
mkdir testing/motl
mkdir testing/motl/asm
mkdir testing/motl/lst

cp /usr/local/4301/data/stage0/*.dat testing/dat
cp /usr/local/4301/data/stage0/*.asm testing/motl/asm
cp /usr/local/4301/data/stage0/*.lst testing/motl/lst

