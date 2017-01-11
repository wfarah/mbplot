#!/bin/csh -f

set utc="2016-04-17-04:21:15"
#Filering SN>12, 175<beam<200, 600<time<1000 (according to image), from the all_candidates file for the above utc. Output in list.dat 
cat /data/mopsr/archives/2016-04-17-04:21:15/all_candidates.dat | awk '{ if ( $1>12 && $13 <200 && $13>175 && $3 <1000 && $3>600) print }' > list.dat

sort -k 13 list.dat > list_sorted.dat #sort according to beam number (not necessary though)

set beam=`awk '{print $13}' list_sorted.dat` #beam number of events
set start_sample=`awk '{print $2}' list_sorted.dat` #start_sample of events
set dm=`awk '{print $6}' list_sorted.dat` #dm of events


set cc=1.34217728
@ n_lines=`more list_sorted.dat | wc -l`
set i=1
#Creating all the .fil files from extract command
while ($i <= $n_lines)
	@ s=($start_sample[$i] - 200)
	extract -f /data/mopsr/archives/$utc/BEAM_$beam[$i]/$utc.fil -start $s -nsamp 2048 > /home/wfarah/task1/files/$utc-$start_sample[$i].fil
	@ i++
end

set i=1
set l=`ls files`
#Creating the .ar files
while ($i <= $n_lines)
	dspsr files/$l[$i] -c $cc -b 1048 -D $dm[$i] -U0.005 -k MO -e $i.ar
	@ i++
end

# mkdir ar
# mv *.ar /ar

psrplot -pfreq+ -jDT ar/*.ar

