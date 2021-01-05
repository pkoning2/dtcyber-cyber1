#!/usr/bin/env python3

"""This script scans for print files created by the "separate" option
in cyber.ini.  In a typical setup, this script is invoked by a system
crontab entry.  The scan is done in the directory named by the command
line argument, if any, or the working directory if no argument is
given.

Depending on what's in the file, it moves it to one of several
subdirectories.  PLATO prints that have a mailing address are sent to
the specified address.

The following subdirectories are used to save files:
   dumps    for core dump printouts
   misc     for stuff not recognizable as anything else
   printed  for PLATO prints that don't appear to have a valid
            email address on them.
   sent     for PLATO prints that were sent to the specified address

A printout that looks like a system job -- for example QREC -- is
deleted outright.  System jobs are defined as jobs that have a blank
user name on the burst page.

   p. koning.    2005.03.07.     initial version.
   p. koning.    2005.07.15.     added mail headers (to, from, subj).
   p. koning.    2005.09.30.     send printout as attachment.
   p. koning.    2006.09.14.     converted to Python.
                                 use real MIME machinery.
   p. resch.     2007.09.03.     pnote notification.
   p. resch,
   p. koning     2007.03.15.     assorted other notification types.
   p. koning     2021.01.05      convert to python 3.
"""

import sys
import os
import time
import re
import smtplib
import email.message
import email.mime.text

global MAILHOST
MAILHOST = "localhost"

dump_re = re.compile ("^ +[0-7]+ +[0-7]+")
tprint_re = re.compile (r"note\(printit,nr\)/[a-z]+\.(?P<lesson>.+?),(?P<account>.+?),(?P<group>.+?),(?P<user>.+?)$")
mailto_re = re.compile (r"note\(printit,nr\)/\*(.+?)\.$")
notify_re = re.compile (r"\*\*\* (.notify) \*\*\*")

subjects = { "pnotify": "New Cyber1 Personal Note",
             "cnotify": "Cyber1 TERM-consult request",
             "onotify": "Cyber1 TERM-operator request",
             "nnotify": "New Cyber1 Notes" }
descriptions = { "pnotify": "%(lesson)s / %(account)s has a new Personal Note from %(user)s / %(group)s on Cyber1.",
                 "cnotify": "%(user)s / %(group)s requests a consultant on Cyber1.",
                 "onotify": "%(user)s / %(group)s requests an operator on Cyber1.",
                 "nnotify": "New notes in sequencer notesfiles list of %(lesson)s / %(account)s on Cyber1." }

def dofile (name):
    """Process a single file.  The file name (within the current
    working directory) is the argument.
    """
    try:
        inf = open (name, "r")
    except OSError as err:
        print ("Error opening %s: %s" % (name, err.strerror))
        return
    text = inf.readlines ()
    inf.close ()
    if "".join (text[:20]).find ("USER NAME =  ") != -1:
        print (name, "is a system job printout, saved to dumps")
        #os.remove (name)
        os.rename (name, os.path.join ("dumps", name))
        return
    if text[-1].find ("UCLP,") == -1:
        print (name, "appears to be an incomplete printout")
        os.rename (name, os.path.join ("misc", name))
        return
    if dump_re.match (text[-2]):
        print (name, "saved in dumps")
        os.rename (name, os.path.join ("dumps", name))
        return
    tail = iter (text[-40:])
    try:
        while True:
            line = next (tail)
            tm = tprint_re.search (line)
            if tm:
                params = tm.groupdict ()
                notify = False
                line = next (tail)
                m = notify_re.search (line)
                if m:
                    notify = True
                    action = m.group (1)
                    try:
                        subject = subjects[action]
                        desc = descriptions[action]
                    except KeyError:
                        print ("Unrecognized notification type %s in %s" % (action, name))
                        os.rename (name, os.path.join ("misc", name))
                        return
                elif line.find ("*** mail to ***") == -1:
                    print ("mail to section not found when expected in", name)
                    os.rename (name, os.path.join ("printed", name))
                    return
                line = next (tail)
                mm = mailto_re.search (line)
                if not mm:
                    print ("mail-to address not found in", name)
                    os.rename (name, os.path.join ("printed", name))
                    return
                mailto = mm.group (1)
                if line.find ("@") == -1:
                    print ("mail-to address", mailto, " appears invalid, in", name)
                    os.rename (name, os.path.join ("printed", name))
                    return
                msg = email.message.Message ()
                if notify:
                    msg.add_header ("From", "\"Cyber1\" <postmaster@cyberserv.org>")
                else:
                    msg.add_header ("From", "\"Cyber1 printout server\" <postmaster@cyberserv.org>")
                msg.add_header ("To", mailto)
                msg.add_header ("Reply-To", mailto)
                if notify:
                    msg.add_header ("Subject", subject)
                    msg.set_payload (desc % params)
                else:
                    subject = "Cyber1 lesson printout"
                    msg.add_header ("Subject", subject)
                    msg.add_header ("MIME-Version", "1.0")
                    msg.add_header ("Content-Type", "multipart/mixed")
                    desc = email.mime.text.MIMEText ("This is a Cyber1 lesson printout of lesson %(lesson)s, requested by %(user)s of %(group)s" % params)
                    desc.add_header ("Content-Description", "message body text")
                    printout = email.mime.text.MIMEText ("".join (text))
                    lesson = params["lesson"]
                    printout.add_header ("Content-Description",
                                     "printout of %s" % lesson)
                    printout.add_header ("Content-Disposition", "inline",
                                     filename = lesson)
                    msg.set_payload ([ desc, printout])
                s = smtplib.SMTP (MAILHOST)
                try:
                    s.sendmail ("postmaster@cyberserv.org", [ mailto ], msg.as_string ())
                    print ("sent", subject, "to", mailto)
                    if notify:
                        os.remove (name)
                    else:
                        print (name, "lesson", lesson, "sent to", mailto)
                        os.rename (name, os.path.join ("sent", name))
                except smtplib.SMTPException as data:
                    print ("error sending", name, "to", mailto, "\n ", data)
                    os.rename (name, os.path.join ("failed", name))
                break
    except StopIteration:
        print ("no mail to section found in", name)
        os.rename (name, os.path.join ("printed", name))
            
            
def main ():
    """Process all the pending printout files in the current working
    directory.
    """
    for f in os.listdir (os.getcwd ()):
        if f.startswith ("LP5xx_20"):
            dofile (f)

print ("sendprints.py loaded")
            
if __name__ == "__main__":
    if len (sys.argv) > 2:
        MAILHOST = sys.argv[2]
    if len (sys.argv) > 1:
        os.chdir (sys.argv[1])
    main ()
    
