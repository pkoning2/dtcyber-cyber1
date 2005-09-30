#!/bin/bash

# This script scans for print files created by the "separate" option
# in cyber.ini.  In a typical setup, this script is invoked by
# a system crontab entry.
#
# Depending on what's in the file, it moves it to one of several 
# subdirectories.  PLATO prints that have a mailing address are sent
# to the specified address.
#
# The following subdirectories are used to save files:
#    dumps    for core dump printouts
#    misc     for stuff not recognizable as anything else
#    printed  for PLATO prints that don't appear to have a valid
#             email address on them.
#    sent     for PLATO prints that were sent to the specified address
#
# A printout that looks like a system job -- for example QREC -- is
# deleted outright.  System jobs are defined as jobs that have a blank
# user name on the burst page.
#
#    p. koning.    2005.03.07.     initial version.
#    p. koning.    2005.07.15.     added mail headers (to, from, subj).
#    p. koning.    2005.09.30.     send printout as attachment.

t=/tmp/sendprints.tmp

for f in LP5xx_20*; do
    if [[ ! -f $f ]] ; then break; fi
    if head -20 $f | fgrep -q "USER NAME =  " ; then
	rm -f $f
	echo $f " is a system job, deleted"
	continue
    fi
    tail -40 $f > $t
    if tail -1  $t | fgrep -q "UCLP," ; then
	if tail -2 $t | egrep -q "^ +[0-7]+ +[0-7]+" ; then
	    mv $f dumps/
	    echo $f " saved in dumps"
	    continue
	fi
	if fgrep -q "block,output." $t ; then
	    if fgrep -q 'note(printit,nr)/***** mail to *************************.' $t ; then
		lesn=`fgrep 'note(printit,nr)/tprint' $t`
		lesn=${lesn#*/tprint.}
		lesn=${lesn%%,*}
		m=`fgrep "@" $t | head -1`
		m=${m#*/?}
		m=${m%.}
		if [ -n "$m" ] ; then
		    echo "sending lesson $lesn (printout file $f) to $m"
		    echo "To: $m" > /tmp/printhdr
		    echo "From: \"Cyber1 printout server\" <postmaster@cyberserv.org>" >> /tmp/printhdr
		    timestamp="$(date +%s)"
		    echo "MIME-Version: 1.0" >> /tmp/printhdr
		    echo "Content-Type: multipart/mixed; boundary=\"$timestamp\"" >> /tmp/printhdr
		    echo "Content-Transfer-Encoding: 7bit" >> /tmp/printhdr
		    echo "Subject: Cyber1 printout of $lesn" >> /tmp/printhdr
		    echo "" >> /tmp/printhdr

		    echo "--$timestamp" >> /tmp/printhdr
		    echo "Content-Type: text/plain" >> /tmp/printhdr
		    echo "Content-Description: printout of $lesn" >> /tmp/printhdr
		    echo "Content-Disposition: inline;" >> /tmp/printhdr
		    echo "	filename=\"$lesn\"" >> /tmp/printhdr
		    echo "Content-Transfer-Encoding: 7bit" >> /tmp/printhdr
		    echo "" >> /tmp/printhdr
		    echo "Printout of lesson $lesn follows:" >> /tmp/printhdr
		    echo "" >> /tmp/printhdr
		    echo "--$timestamp" >> /tmp/printhdr
		    echo "Content-Type: text/plain" >> /tmp/printhdr
		    echo "Content-Description: printout of $lesn" >> /tmp/printhdr
		    echo "Content-Disposition: inline;" >> /tmp/printhdr
		    echo "	filename=\"$lesn\"" >> /tmp/printhdr
		    echo "Content-Transfer-Encoding: 7bit" >> /tmp/printhdr
		    echo "" >> /tmp/printhdr
		    echo "" > /tmp/printtrailer
		    echo "--$timestamp--" >> /tmp/printtrailer
		    cat /tmp/printhdr $f /tmp/printtrailer | /usr/sbin/sendmail $m
		    echo $f " sent to " $m
		    mv $f sent/
		    rm /tmp/printhdr /tmp/printtrailer
		    continue
		fi
	    fi
	    mv $f printed/
	    echo $f " does not have mailing info, saved in printed"
	    continue
	fi
	mv $f misc/
	echo $f " saved in misc"
    else
	mv $f misc/
	echo $f " appears to be incomplete"
    fi
done

rm -f $t
