#!/usr/bin/perl
#
# Copyright (C) 2010, 2012  Internet Systems Consortium, Inc. ("ISC")
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

# $Id: check-secure-delegation.pl.in,v 1.2 2010/12/21 02:32:21 marka Exp $

use warnings;
use FileHandle;
use IPC::Open2;
use POSIX qw/strftime/;

#
# We only compare keyid / DNSSEC algorithm pairs.  If this succeeds then
# the crypto will likely succeed.  If it fails then the crypto will definitely
# fail.
#
$prefix = "/usr/local/named";
$dig = "$prefix/bin/dig +cd +dnssec +noall +answer";
$dsfromkey = "$prefix/sbin/dnssec-dsfromkey -1 -A -f /dev/stdin";

# Get "now" in a RRSIG datestamp format.
$now = strftime "%Y%m%d%H%M%S", gmtime;

foreach $zone (@ARGV) {
	my %algorithms = ();
	my %dnskeygood = ();
	my %dnskeyalg = ();
	my %dnskey = ();
	my %dsgood = ();
	my %ds = ();

	# Read the DS records and extract the key id, algorithm pairs
	open(DS, "$dig -t DS -q $zone|") || die("dig DS failed");
	while(<DS>) {
		@words = split; 
		if ($words[3] eq "RRSIG" && $words[4] eq "DS") {
			next if ($words[8] >= $now && $words[9] <= $now);
			print "BAD SIG DATES: $_";
		}
		next if ($words[3] ne "DS");
		$ds{"$words[4] $words[5]"} = 1;
		$algorithms{"$words[5]"} = 1;
	}
	close(DS);

	# Read the RRSIG(DNSKEY) records and extract the key id,
	# algorithm pairs.  Set good if we have a match against the DS
	# records.  DNSKEY records should be before the RRSIG records.
	open(DNSKEY, "$dig -t DNSKEY -q $zone|") || die("dig DNSKEY failed");
	while (<DNSKEY>) {
		@words = split;
		if ($words[3] eq "DNSKEY") {
			$dnskeyalg{"$words[6]"} = 1;
			next if (! -e "/dev/stdin");
			# get the key id ($dswords[3]).
			$pid = open2(*Reader, *Writer, "$dsfromkey $zone");
			die("dsfromkey failed") if ($pid == -1);
			print Writer "$_";
			close(Writer);
			$line = <Reader>;
			close(Reader);
			@dswords = split /\s/, $line;
			$dnskey{"$dswords[3] $dswords[4]"} = 1;
			next;
		}
		next if ($words[3] ne "RRSIG" || $words[4] ne "DNSKEY");
		if ($words[8] >= $now && $words[9] <= $now) {
			# If we don't have /dev/stdin then just check for the
			# RRSIG otherwise check for both the DNSKEY and
			# RRSIG.
			$dsgood{"$words[5]"} = 1
				if (! -e "/dev/stdin" &&
				    exists($ds{"$words[10] $words[5]"}));
			$dsgood{"$words[5]"} = 1
				if (exists($ds{"$words[10] $words[5]"}) &&
				    exists($dnskey{"$words[10] $words[5]"}));
			$dnskeygood{"$words[5]"} = 1
				if (! -e "/dev/stdin");
			$dnskeygood{"$words[5]"} = 1
				if (exists($dnskey{"$words[10] $words[5]"}));
		} else {
			$dnskeygood{"$words[5]"} = 1;
			print "BAD SIG DATES: $_";
		}
	}
	close(DNSKEY);

	# Do we have signatures for all DNSKEY algorithms?
	foreach $alg ( keys %dnskeyalg ) {
		print "Missing $zone DNSKEY RRSIG for algorithm $alg\n"
			if (!exists($dnskeygood{$alg}));
	}

	# Do we have a matching self signed DNSKEY for all DNSSEC algorithms
	# in the DS records.
	$count = 0;
	foreach $alg ( keys %algorithms ) {
		if (exists($dsgood{$alg})) {
			print "$zone algorithm $alg good " .
			      "(found DS / self signed DNSKEY pair)\n";
		} else {
			print "$zone algorithm $alg bad " .
			      "(no DS / self signed DNSKEY pair found)\n";
		}
		$count++;
	}
	print "$zone has no secure delegation records\n"
		if (! $count);
}
