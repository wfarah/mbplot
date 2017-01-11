#!/bin/csh -f

set utc=$argv[1]
#set utc="2016-06-01-16:28:24"
set nsamp=$argv[2]
set width=$argv[3]
set dm=$argv[4]
set beam=$argv[5]
set c_dir=`pwd`
set dir="/data/mopsr/archives/$utc"
@ s = $nsamp - 200
set cc=1.34217728
#set cc=`echo 600| awk '{ print $1*0.65536/1000}'`
mkdir tmp
cd tmp
extract -f $dir/BEAM_$beam/$utc.fil -start $s -nsamp 2048 > $c_dir/tmp/$utc.fil
echo -f $dir/BEAM_$beam/$utc.fil -start $s -nsamp 2048 > $c_dir/tmp/$utc.fil
dspsr $c_dir/tmp/$utc.fil -c $cc -b 2048 -D $dm -U0.005 -k MO -e .ar
echo $c_dir/tmp/$utc.fil -c $cc -b 2048 -D $dm -U0.005 -k MO -e .ar
pdmp  *.ar
echo pdmp *.ar
#bins 2048!!!
#echo "Save?(y/n)"
#set answer = $<

#if ( $answer == "y" ) then
#	cd $c_dir
#	if ( ! -e saved_fil ) then
#		mkdir saved_fil
#	endif
#	mv $c_dir/tmp/*.fil $c_dir/saved_fil/
#	rm -r tmp
#else
#	cd $c_dir
#	rm -r tmp
#endif

#cd $c_dir
#rm -r tmp
