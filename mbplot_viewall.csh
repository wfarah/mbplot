#!/bin/csh -f

set utc=$argv[1]
set dir=/data/mopsr/archives
set c_dir=`pwd`

if !( -e $dir) then
	echo "Directory $dir does not exist, please enter the valid directory for $utc"
	set dir = $<
endif

@ n_lines=`more tmp.dat | wc -l`
echo $n_lines
set start_sample = `awk '{print $1}' tmp.dat` 
set width = `awk '{print $2}' tmp.dat`
set dm = `awk '{print $3}' tmp.dat`
set beam = `awk '{print $4}' tmp.dat`
set cc=1.34217728

set i=1

if (-e tmp) then
        rm tmp -r
endif

mkdir tmp
cd tmp
while ($i <= $n_lines)
	@ s=($start_sample[$i] - 200)
	extract -f $dir/$utc/BEAM_$beam[$i]/$utc.fil -start $s -nsamp 2048 > $utc.$i.fil
	dspsr $utc.$i.fil -c $cc -b 2048 -D $dm[$i] -U0.005 -k MO -O $utc.$i
	@ i++
end

cd ../
rm tmp.dat

psrplot -pfreq+ -jDT -D /xs tmp/*.ar

echo "\nSave any (all/y/n)?\n"
set answer = $<

if ( $answer == "all" ) then
	if (! -e saved_fil) then
		mkdir saved_fil
	endif
	mv tmp/*.fil saved_fil
	rm tmp -r
else if ( $answer == "y" ) then
	if (! -e saved_fil) then
		mkdir saved_fil
	endif
	echo "\nWhich ones? (-1 to stop)\n"
	set answer1 = $<
	set l = ""
	while ( $answer1 != "-1" )
		set l = ( $l $answer1 )
		set answer1 = $<
	end
	foreach i ( $l )
		mv tmp/$utc.$i.fil saved_fil
	end
endif

rm tmp/ -r
