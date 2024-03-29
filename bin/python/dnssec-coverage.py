#!
############################################################################
# Copyright (C) 2013  Internet Systems Consortium, Inc. ("ISC")
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
############################################################################

import argparse
import os
import glob
import sys
import re
import time
import calendar
from collections import defaultdict
import pprint

prog='dnssec-coverage'

########################################################################
# Class Event
########################################################################
class Event:
    """ A discrete key metadata event, e.g., Publish, Activate, Inactive,    
    Delete. Stores the date of the event, and identifying information about 
    the key to which the event will occur."""

    def __init__(self, _what, _key):
        now = time.time()
        self.what = _what
        self.when = _key.metadata[_what]
        self.key = _key
        self.keyid = _key.keyid
        self.sep = _key.sep
        self.zone = _key.zone
        self.alg = _key.alg

    def __repr__(self):
        return repr((self.when, self.what, self.keyid, self.sep,
                     self.zone, self.alg))

    def showtime(self):
        return time.strftime("%a %b %d %H:%M:%S UTC %Y", self.when)

    def showkey(self):
        return self.key.showkey()

    def showkeytype(self):
        return self.key.showkeytype()

########################################################################
# Class Key
########################################################################
class Key:
    """An individual DNSSEC key.  Identified by path, zone, algorithm, keyid.
    Contains a dictionary of metadata events."""

    def __init__(self, keyname):
        directory = os.path.dirname(keyname)
        key = os.path.basename(keyname)
        (zone, alg, keyid) = key.split('+')
        keyid = keyid.split('.')[0]
        key = [zone, alg, keyid]
        key_file = directory + os.sep + '+'.join(key) + ".key"
        private_file = directory + os.sep + '+'.join(key) + ".private"

        self.zone = zone[1:-1]
        self.alg = int(alg)
        self.keyid = int(keyid)

        kfp = file(key_file, "r")
        for line in kfp:
            if line[0] == ';':
                continue
            tokens = line.split()
            if not tokens:
                continue

            if tokens[1].lower() in ('in', 'ch', 'hs'):
                septoken = 3
                self.ttl = args.keyttl
                if not self.ttl:
                    vspace()
                    print("WARNING: Unable to determine TTL for DNSKEY %s." %
                           self.showkey())
                    print("\t Using 1 day (86400 seconds); re-run with the -d "
                          "option for more\n\t accurate results.")
                    self.ttl = 86400
            else:
                septoken = 4
                self.ttl = int(tokens[1]) if not args.keyttl else args.keyttl

            if (int(tokens[septoken]) & 0x1) == 1:
                self.sep = True
            else:
                self.sep = False
        kfp.close()

        pfp = file(private_file, "rU")
        propDict = dict()
        for propLine in pfp:
            propDef = propLine.strip()
            if len(propDef) == 0:
                continue
            if propDef[0] in ('!', '#'):
                continue
            punctuation = [propDef.find(c) for c in ':= '] + [len(propDef)]
            found = min([ pos for pos in punctuation if pos != -1 ])
            name = propDef[:found].rstrip()
            value =  propDef[found:].lstrip(":= ").rstrip()
            propDict[name] = value

        if("Publish" in propDict):
            propDict["Publish"] = time.strptime(propDict["Publish"],
                                                "%Y%m%d%H%M%S")

        if("Activate" in propDict):
            propDict["Activate"] = time.strptime(propDict["Activate"],
                                                 "%Y%m%d%H%M%S")

        if("Inactive" in propDict):
            propDict["Inactive"] = time.strptime(propDict["Inactive"],
                                                 "%Y%m%d%H%M%S")

        if("Delete" in propDict):
            propDict["Delete"] = time.strptime(propDict["Delete"],
                                               "%Y%m%d%H%M%S")

        if("Revoke" in propDict):
            propDict["Revoke"] = time.strptime(propDict["Revoke"],
                                               "%Y%m%d%H%M%S")
        pfp.close()
        self.metadata = propDict

    def showkey(self):
        return "%s/%03d/%05d" % (self.zone, self.alg, self.keyid);

    def showkeytype(self):
        return ("KSK" if self.sep else "ZSK")

    # ensure that the gap between Publish and Activate is big enough
    def check_prepub(self):
        now = time.time()

        if (not "Activate" in self.metadata):
            debug_print("No Activate information in key: %s" % self.showkey())
            return False
        a = calendar.timegm(self.metadata["Activate"])

        if (not "Publish" in self.metadata):
            debug_print("No Publish information in key: %s" % self.showkey())
            if a > now:
                vspace()
                print("WARNING: Key %s (%s) is scheduled for activation but \n"
                  "\t not for publication." %
                  (self.showkey(), self.showkeytype()))
            return False
        p = calendar.timegm(self.metadata["Publish"])

        now = time.time()
        if p < now and a < now:
            return True

        if p == a:
            vspace()
            print ("WARNING: %s (%s) is scheduled to be published and\n"
                   "\t activated at the same time. This could result in a\n"
                   "\t coverage gap if the zone was previously signed." %
                   (self.showkey(), self.showkeytype()))
            print("\t Activation should be at least %s after publication."
                    % duration(self.ttl))
            return True

        if a < p:
            vspace()
            print("WARNING: Key %s (%s) is active before it is published" %
                  (self.showkey(), self.showkeytype()))
            return False

        if (a - p < self.ttl):
            vspace()
            print("WARNING: Key %s (%s) is activated too soon after\n"
                  "\t publication; this could result in coverage gaps due to\n"
                  "\t resolver caches containing old data."
                  % (self.showkey(), self.showkeytype()))
            print("\t Activation should be at least %s after publication." %
                  duration(self.ttl))
            return False

        return True

    # ensure that the gap between Inactive and Delete is big enough
    def check_postpub(self, timespan = None):
        if not timespan:
            timespan = self.ttl

        now = time.time()

        if (not "Delete" in self.metadata):
            debug_print("No Delete information in key: %s" % self.showkey())
            return False
        d = calendar.timegm(self.metadata["Delete"])

        if (not "Inactive" in self.metadata):
            debug_print("No Inactive information in key: %s" % self.showkey())
            if d > now:
                vspace()
                print("WARNING: Key %s (%s) is scheduled for deletion but\n"
                      "\t not for inactivation." %
                      (self.showkey(), self.showkeytype()))
            return False
        i = calendar.timegm(self.metadata["Inactive"])

        if d < now and i < now:
            return True

        if (d < i):
            vspace()
            print("WARNING: Key %s (%s) is scheduled for deletion before\n"
                  "\t inactivation." % (self.showkey(), self.showkeytype()))
            return False

        if (d - i < timespan):
            vspace()
            print("WARNING: Key %s (%s) scheduled for deletion too soon after\n"
                  "\t deactivation; this may result in coverage gaps due to\n"
                  "\t resolver caches containing old data."
                  % (self.showkey(), self.showkeytype()))
            print("\t Deletion should be at least %s after inactivation." %
                  duration(timespan))
            return False

        return True

########################################################################
# class Zone
########################################################################
class Zone:
    """Stores data about a specific zone"""

    def __init__(self, _name, _keyttl = None, _maxttl = None):
        self.name = _name
        self.keyttl = _keyttl
        self.maxttl = _maxttl

    def load(self, filename):
        if not args.compilezone:
            sys.stderr.write(prog + ': FATAL: "named-compilezone" not found\n')
            exit(1)

        if not self.name:
            return

        maxttl = keyttl = None

        fp = os.popen("%s -o - %s %s 2> /dev/null" %
                      (args.compilezone, self.name, filename))
        for line in fp:
            fields = line.split()
            if not maxttl or int(fields[1]) > maxttl:
                maxttl = int(fields[1])
            if fields[3] == "DNSKEY":
                keyttl = int(fields[1])
        fp.close()

        self.keyttl = keyttl
        self.maxttl = maxttl

############################################################################
# debug_print:
############################################################################
def debug_print(debugVar):
    """pretty print a variable iff debug mode is enabled"""
    if not args.debug_mode:
        return
    if type(debugVar) == str:
        print("DEBUG: " + debugVar)
    else:
        print("DEBUG: " + pprint.pformat(debugVar))
    return

############################################################################
# vspace:
############################################################################
_firstline = True
def vspace():
    """adds vertical space between two sections of output text if and only
    if this is *not* the first section being printed"""
    global _firstline
    if _firstline:
        _firstline = False
    else:
        print

############################################################################
# vreset:
############################################################################
def vreset():
    """reset vertical spacing"""
    global _firstline
    _firstline = True

############################################################################
# getunit
############################################################################
def getunit(secs, size):
    """given a number of seconds, and a number of seconds in a larger unit of
    time, calculate how many of the larger unit there are and return both
    that and a remainder value"""
    bigunit = secs // size 
    if bigunit:
        secs %= size
    return (bigunit, secs)

############################################################################
# addtime
############################################################################
def addtime(output, unit, t):
    """add a formatted unit of time to an accumulating string"""
    if t:
        output += ("%s%d %s%s" %
                  ((", " if output else ""),
                   t, unit, ("s" if t > 1 else "")))

    return output

############################################################################
# duration:
############################################################################
def duration(secs):
    """given a length of time in seconds, print a formatted human duration
    in larger units of time
    """
    # define units:
    minute = 60
    hour = minute * 60
    day = hour * 24
    month = day * 30
    year = day * 365

    # calculate time in units:
    (years, secs) = getunit(secs, year)
    (months, secs) = getunit(secs, month)
    (days, secs) = getunit(secs, day)
    (hours, secs) = getunit(secs, hour)
    (minutes, secs) = getunit(secs, minute)

    output = ''
    output = addtime(output, "year", years)
    output = addtime(output, "month", months)
    output = addtime(output, "day", days)
    output = addtime(output, "hour", hours)
    output = addtime(output, "minute", minutes)
    output = addtime(output, "second", secs)
    return output

############################################################################
# parse_time
############################################################################
def parse_time(s):
    """convert a formatted time (e.g., 1y, 6mo, 15mi, etc) into seconds"""
    s = s.strip()

    # if s is an integer, we're done already
    try:
        n = int(s)
        return n
    except:
        pass

    # try to parse as a number with a suffix indicating unit of time
    r = re.compile('([0-9][0-9]*)\s*([A-Za-z]*)')
    m = r.match(s)
    if not m:
        raise Exception("Cannot parse %s" % s)
    (n, unit) = m.groups()
    n = int(n)
    unit = unit.lower()
    if unit[0] == 'y':
        return n * 31536000
    elif unit[0] == 'm' and unit[1] == 'o':
        return n * 2592000
    elif unit[0] == 'w':
        return n * 604800
    elif unit[0] == 'd':
        return n * 86400
    elif unit[0] == 'h':
        return n * 3600
    elif unit[0] == 'm' and unit[1] == 'i':
        return n * 60 
    elif unit[0] == 's':
        return n
    else:
        raise Exception("Invalid suffix %s" % unit)

############################################################################
# algname:
############################################################################
def algname(alg):
    """return the mnemonic for a DNSSEC algorithm"""
    names = (None, 'RSAMD5', 'DH', 'DSA', 'ECC', 'RSASHA1',
            'NSEC3DSA', 'NSEC3RSASHA1', 'RSASHA256', None,
            'RSASHA512', None, 'ECCGOST', 'ECDSAP256SHA256', 
            'ECDSAP384SHA384')
    name = None
    if alg in range(len(names)):
        name = names[alg]
    return (name if name else str(alg))

############################################################################
# list_events:
############################################################################
def list_events(eventgroup):
    """print a list of the events in an eventgroup"""
    if not eventgroup:
        return
    print ("  " + eventgroup[0].showtime() + ":")
    for event in eventgroup:
        print ("    %s: %s (%s)" %
               (event.what, event.showkey(), event.showkeytype()))

############################################################################
# process_events:
############################################################################
def process_events(eventgroup, active, published):
    """go through the events in an event group in time-order, add to active
     list upon Activate event, add to published list upon Publish event,
     remove from active list upon Inactive event, and remove from published
     upon Delete event. Emit warnings when inconsistant states are reached"""
    for event in eventgroup:
        if event.what == "Activate":
            active.add(event.keyid)
        elif event.what == "Publish":
            published.add(event.keyid)
        elif event.what == "Inactive":
            if event.keyid not in active:
                vspace()
                print ("\tWARNING: %s (%s) scheduled to become inactive "
                       "before it is active" %
                       (event.showkey(), event.showkeytype()))
            else:
                active.remove(event.keyid)
        elif event.what == "Delete":
            if event.keyid in published:
                published.remove(event.keyid)
            else:
                vspace()
                print ("WARNING: key %s (%s) is scheduled for deletion before "
                       "it is published, at %s" %
                       (event.showkey(), event.showkeytype()))
        elif event.what == "Revoke":
            # We don't need to worry about the logic of this one;
            # just stop counting this key as either active or published
            if event.keyid in published:
                published.remove(event.keyid)
            if event.keyid in active:
                active.remove(event.keyid)

    return (active, published)

############################################################################
# check_events:
############################################################################
def check_events(eventsList, ksk):
    """create lists of events happening at the same time, check for 
    inconsistancies"""
    active = set()
    published = set()
    eventgroups = list()
    eventgroup = list()
    keytype = ("KSK" if ksk else "ZSK")

    # collect up all events that have the same time
    eventsfound = False
    for event in eventsList:
        # if checking ZSKs, skip KSKs, and vice versa
        if (ksk and not event.sep) or (event.sep and not ksk):
            continue

        # we found an appropriate (ZSK or KSK event)
        eventsfound = True

        # add event to current eventgroup
        if (not eventgroup or eventgroup[0].when == event.when):
            eventgroup.append(event)

        # if we're at the end of the list, we're done.  if
        # we've found an event with a later time, start a new
        # eventgroup
        if (eventgroup[0].when != event.when):
            eventgroups.append(eventgroup)
            eventgroup = list()
            eventgroup.append(event)

    if eventgroup:
        eventgroups.append(eventgroup)

    for eventgroup in eventgroups:
        (active, published) = \
           process_events(eventgroup, active, published)

        list_events(eventgroup)

        # and then check for inconsistencies:
        if len(active) == 0:
            print ("ERROR: No %s's are active after this event" % keytype)
            return False
        elif len(published) == 0:
            sys.stdout.write("ERROR: ")
            print ("ERROR: No %s's are published after this event" % keytype)
            return False
        elif len(published.intersection(active)) == 0:
            sys.stdout.write("ERROR: ")
            print (("ERROR: No %s's are both active and published " +
                    "after this event") % keytype)
            return False

    if not eventsfound:
        print ("ERROR: No %s events found in '%s'" %
               (keytype, args.path))
        return False

    return True

############################################################################
# check_zones:
# ############################################################################
def check_zones(eventsList):
    """scan events per zone, algorithm, and key type, in order of occurrance,
    noting inconsistent states when found"""
    global foundprob

    foundprob = False
    zonesfound = False
    for zone in eventsList:
        if args.zone and zone != args.zone:
            continue

        zonesfound = True
        for alg in eventsList[zone]:
            vspace()
            print("Checking scheduled KSK events for zone %s, algorithm %s..." %
                   (zone, algname(alg)))
            if not check_events(eventsList[zone][alg], True):
                foundprob = True
            else:
                print ("No errors found")

            vspace()
            print("Checking scheduled ZSK events for zone %s, algorithm %s..." %
                  (zone, algname(alg)))
            if not check_events(eventsList[zone][alg], False):
                foundprob = True
            else:
                print ("No errors found")

    if not zonesfound:
        print("ERROR: No key events found for %s in '%s'" %
               (args.zone, args.path))
        exit(1)

############################################################################
# fill_eventsList:
############################################################################
def fill_eventsList(eventsList):
    """populate the list of events"""
    for zone, algorithms in keyDict.items():
        for alg, keys in  algorithms.items():
            for keyid, keydata in keys.items():
                if("Publish" in keydata.metadata):
                    eventsList[zone][alg].append(Event("Publish", keydata))
                if("Activate" in keydata.metadata):
                    eventsList[zone][alg].append(Event("Activate", keydata))
                if("Inactive" in keydata.metadata):
                    eventsList[zone][alg].append(Event("Inactive", keydata))
                if("Delete" in keydata.metadata):
                    eventsList[zone][alg].append(Event("Delete", keydata))

            eventsList[zone][alg] = sorted(eventsList[zone][alg],
                                           key=lambda event: event.when)

    foundprob = False
    if not keyDict:
        print("ERROR: No key events found in '%s'" % args.path)
        exit(1)

############################################################################
# set_path:
############################################################################
def set_path(command, default=None):
    """find the location of a specified command.  if a default is supplied
    and it works, we use it; otherwise we search PATH for a match.  If
    not found, error and exit"""
    fpath = default
    if not fpath or not os.path.isfile(fpath) or not os.access(fpath, os.X_OK):
        path = os.environ["PATH"]
        if not path:
            path = os.path.defpath
        for directory in path.split(os.pathsep):
            fpath = directory + os.sep + command
            if os.path.isfile(fpath) or os.access(fpath, os.X_OK):
                break
            fpath = None

    return fpath

############################################################################
# parse_args:
############################################################################
def parse_args():
    """Read command line arguments, set global 'args' structure"""
    global args

    compilezone = set_path('named-compilezone',
                           '/usr/local/named/sbin/named-compilezone')

    parser = argparse.ArgumentParser(description=prog + ': checks future ' +
                                     'DNSKEY coverage for a zone')

    parser.add_argument('zone', type=str, help='zone to check')
    parser.add_argument('-K', dest='path', default='.', type=str,
                        help='a directory containing keys to process',
                        metavar='dir')
    parser.add_argument('-f', dest='filename', type=str,
                        help='zone master file', metavar='file')
    parser.add_argument('-m', dest='maxttl', type=str,
                        help='the longest TTL in the zone(s)',
                        metavar='int')
    parser.add_argument('-d', dest='keyttl', type=str,
                        help='the DNSKEY TTL', metavar='int')
    parser.add_argument('-r', dest='resign', default='1944000', 
                        type=int, help='the RRSIG refresh interval '
                                       'in seconds [default: 22.5 days]',
                        metavar='int')
    parser.add_argument('-c', dest='compilezone',
                        default=compilezone, type=str,
                        help='path to \'named-compilezone\'',
                        metavar='path')
    parser.add_argument('-D', '--debug', dest='debug_mode',
                        action='store_true', default=False,
                        help='Turn on debugging output')
    parser.add_argument('-v', '--version', action='version', version='9.9.1')

    args = parser.parse_args()

    # convert from time arguments to seconds
    try:
        if args.maxttl:
            m = parse_time(args.maxttl)
            args.maxttl = m
    except:
        pass

    try:
        if args.keyttl:
            k = parse_time(args.keyttl)
            args.keyttl = k
    except:
        pass

    try:
        if args.resign:
            r = parse_time(args.resign)
            args.resign = r
    except:
        pass

    # if we've got the values we need from the command line, stop now
    if args.maxttl and args.keyttl:
        return

    # load keyttl and maxttl data from zonefile
    if args.zone and args.filename:
        try:
            zone = Zone(args.zone)
            zone.load(args.filename)
            if not args.maxttl:
                args.maxttl = zone.maxttl
            if not args.keyttl:
                args.keyttl = zone.maxttl
        except Exception as e:
            print("Unable to load zone data from %s: " % args.filename, e)

    if not args.maxttl:
        vspace()
        print ("WARNING: Maximum TTL value was not specified.  Using 1 week\n"
               "\t (604800 seconds); re-run with the -m option to get more\n"
               "\t accurate results.")
        args.maxttl = 604800

############################################################################
# Main
############################################################################
def main():
    global keyDict

    parse_args()
    path=args.path

    print ("PHASE 1--Loading keys to check for internal timing problems")
    keyDict = defaultdict(lambda : defaultdict(dict))
    files = glob.glob(os.path.join(path, '*.private'))
    for infile in files:
        key = Key(infile)
        if args.zone and key.zone != args.zone:
            continue
        keyDict[key.zone][key.alg][key.keyid] = key
        key.check_prepub()
        if key.sep:
            key.check_postpub()
        else:
            key.check_postpub(args.maxttl + args.resign)

    vspace()
    print ("PHASE 2--Scanning future key events for coverage failures")
    vreset()

    eventsList = defaultdict(lambda : defaultdict(list))
    fill_eventsList(eventsList)
    check_zones(eventsList)

    if foundprob:
        exit(1)
    else:
        exit(0)

if __name__ == "__main__":
    main()
