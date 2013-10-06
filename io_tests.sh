#!/bin/sh
for flagf in noflush flushfs;do echo -n "$flagf\t";
    for flagd in nodirect direct; do echo -n "$flagd\t";
	for flags in nosync sync; do echo -n "$flags\t";
	    for flaga in noalign align;do echo -n "$flaga\t";
		for flagr in norandom random; do echo  "$flagr\t ";
		    LogFile=tests_"$flagf"_"$flagr"_"$flaga"_"$flags"_"$flagd".log;
		    for size in `seq 8 8 64 `; 
		    do   ./io_test $((size*1024*1024)) $flagr $flaga $flags $flagd |tee -a $LogFile ;
		    done
		done
	    done
	done
    done
done