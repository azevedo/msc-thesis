#!/usr/bin/perl -w

# FILENAME
# tranpose_abc.pl

# HISTORY
# June 16, 2001 Version 1
# Original by Matthew J. Fisher
#
# March 8, 2007
# Modified by Sebastian Orlowski,:
#  - added support for ornaments in !...! (keep output unchanged)
#  - added support for text in "^..." or "_..." (keep output unchanged)
#  - added support for [K:treble] etc. statements (keep output unchanged)
#  - fixed a bug in transpose_line() (added '=')
#
# April 5, 2007 Version 2
# Complete overhaul by Christopher David Lane:
#  - argument order has changed (transposition first) for use in Unix pipes
#  - (old argument order recognized and reordered)
#  - ignores & passes through %%begin* ... %%end* blocks and abcpp macros
#  - handles minor keys directly and not as a special case of major keys
#  - transposition lower by more than an octave (or by 0) works correctly
#  - support for inline field key changes, e.g. [K:D]
#  - fixed handling of split guitar chords
#  - etc. and it's faster but it's still far from perfect
#
# December 23, 2008 Version 2.1
# Modified by Christoph Dalitz
#  - command line option -nochords for suppressing transposition of gchords

# DESCRIPTION
# This is a Perl script for transposing abc files.
# Permission to copy, redistribute, modify, etc.
# is granted without restriction -- especially if
# you feel like converting this to a CGI script.

# INSTRUCTIONS
# This program is designed to be run from the command line. To run it, you
# will need to have a Perl distribution installed on your computer, e.g.,
#
#   ActivePerl
#   http://www.activestate.com/
#   http://www.activestate.com/ASPN/Downloads/ActivePerl/
#
# Usage: transpose_abc.pl offset < input_file > output_file
# Example: transpose_abc.pl 3 < song1.abc > song2.abc
# Offset is given in semitones, and may be positive or negative

# BUGS AND AREAS FOR IMPROVEMENT
# Area for improvement includes proper handling key signature modifiers. 
# As things stand now, the input file is correctly transposed in terms of
# pitch.  But the way the pitch is encoded in the output file will need
# tweaking to see signature modifiers in the output.

use strict;

our $VERSION = 2.1;

use constant VERBOSE => 1; # extra warnings and informational messages.

# You probably don't want this; it helps when diff'ing the output of the two programs.
use constant EMULATE_ABC2ABC => 0; # emulate 'abc2abc -t'

use constant EMPTY => '';

use constant OCTAVE => 12;

use constant { DOUBLE_FLAT => -2, FLAT => -1, NATURAL => 0, SHARP => 1, DOUBLE_SHARP => 2 };
	
use constant { C => 0, D => 2, E => 4, F => 5, G => 7, A => 9, B => 11 };

my %notes_to_numbers = ( c => C, d => D, e => E, f => F, g => G, a => A, b => B );

my %accidental_to_text = ( DOUBLE_FLAT, 'bb', FLAT, 'b', NATURAL, EMPTY, SHARP, '#', DOUBLE_SHARP, '##' );

my %accidental_to_symbol = ( DOUBLE_FLAT, '__', FLAT, '_', NATURAL, '=', SHARP, '^', DOUBLE_SHARP, '^^' );

my %empty_accidentals = ( c => EMPTY, d => EMPTY, e => EMPTY, f => EMPTY, g => EMPTY, a => EMPTY, b => EMPTY );

# These key names correspond to those recognized by abc2midi

my @display_keys = (
	[ 'Cb',  'CbMaj', 'FbLyd', 'CbIon', 'GbMix' ],
	[ 'Gb',  'GbMaj', 'CbLyd', 'GbIon', 'DbMix' ],
	[ 'Db',  'DbMaj', 'GbLyd', 'DbIon', 'AbMix' ],
	[ 'Ab',  'AbMaj', 'DbLyd', 'AbIon', 'EbMix' ],
	[ 'Eb',  'EbMaj', 'AbLyd', 'EbIon', 'BbMix' ],
	[ 'Bb',  'BbMaj', 'EbLyd', 'BbIon',  'FMix' ],
	[ 'F',    'FMaj', 'BbLyd',  'FIon',  'CMix' ],
	[ 'C',    'CMaj',  'FLyd',  'CIon',  'GMix' ],
	[ 'G',    'GMaj',  'CLyd',  'GIon',  'DMix' ],
	[ 'D',    'DMaj',  'GLyd',  'DIon',  'AMix' ],
	[ 'A',    'AMaj',  'DLyd',  'AIon',  'EMix' ],
	[ 'E',    'EMaj',  'ALyd',  'EIon',  'BMix' ],
	[ 'B',    'BMaj',  'ELyd',  'BIon', 'F#Mix' ],
	[ 'F#',  'F#Maj',  'BLyd', 'F#Ion', 'C#Mix' ],
	[ 'C#',  'C#Maj', 'F#Lyd', 'C#Ion', 'G#Mix' ],
	
	[ 'Abm', 'AbMin', 'DbDor', 'AbAeo', 'EbPhr', 'BbLoc' ],
	[ 'Ebm', 'EbMin', 'AbDor', 'EbAeo', 'BbPhr',  'FLoc' ],
	[ 'Bbm', 'BbMin', 'EbDor', 'BbAeo',  'FPhr',  'CLoc' ],
	[ 'Fm',   'FMin', 'BbDor',  'FAeo',  'CPhr',  'GLoc' ],
	[ 'Cm',   'CMin',  'FDor',  'CAeo',  'GPhr',  'DLoc' ],
	[ 'Gm',   'GMin',  'CDor',  'GAeo',  'DPhr',  'ALoc' ],
	[ 'Dm',   'DMin',  'GDor',  'DAeo',  'APhr',  'ELoc' ],
	[ 'Am',   'AMin',  'DDor',  'AAeo',  'EPhr',  'BLoc' ],
	[ 'Em',   'EMin',  'ADor',  'EAeo',  'BPhr', 'F#Loc' ],
	[ 'Bm',   'BMin',  'EDor',  'BAeo', 'F#Phr', 'C#Loc' ],
	[ 'F#m', 'F#Min',  'BDor', 'F#Aeo', 'C#Phr', 'G#Loc' ],
	[ 'C#m', 'C#Min', 'F#Dor', 'C#Aeo', 'G#Phr', 'D#Loc' ],
	[ 'G#m', 'G#Min', 'C#Dor', 'G#Aeo', 'D#Phr', 'A#Loc' ],
	[ 'D#m', 'D#Min', 'G#Dor', 'D#Aeo', 'A#Phr', 'E#Loc' ],
	[ 'A#m', 'A#Min', 'D#Dor', 'A#Aeo', 'E#Phr', 'B#Loc' ],
	);
	
my %relative_keys;

foreach my $row (@display_keys) {
	foreach my $key (@{$row}) {
		$relative_keys{uc $key} = $row->[0];
	}
}

my %key_signatures = (
	'Cb' => { b => -1, e => -1, a => -1, d => -1, g => -1, c => -1, f => -1 },
	'Gb' => { b => -1, e => -1, a => -1, d => -1, g => -1, c => -1 },
	'Db' => { b => -1, e => -1, a => -1, d => -1, g => -1 },
	'Ab' => { b => -1, e => -1, a => -1, d => -1 },
	'Eb' => { b => -1, e => -1, a => -1 },
	'Bb' => { b => -1, e => -1 },
	'F'  => { b => -1 },
	'C'  => { c =>  0, d =>  0, e =>  0, f =>  0, g =>  0, a =>  0, b =>  0 },
	'G'  => { f =>  1 },
	'D'  => { f =>  1, c =>  1 },
	'A'  => { f =>  1, c =>  1, g =>  1 },
	'E'  => { f =>  1, c =>  1, g =>  1, d =>  1 },
	'B'  => { f =>  1, c =>  1, g =>  1, d =>  1, a =>  1 },
	'F#' => { f =>  1, c =>  1, g =>  1, d =>  1, a =>  1, e =>  1 },
	'C#' => { f =>  1, c =>  1, g =>  1, d =>  1, a =>  1, e =>  1, b =>  1 },
	);
	
@key_signatures{'Abm','Ebm','Bbm','Fm','Cm','Gm','Dm','Am','Em','Bm','F#m','C#m','G#m','D#m','A#m'} =
	@key_signatures{'Cb','Gb','Db','Ab','Eb','Bb','F','C','G','D','A','E','B','F#','C#'};

# The bulk of the keys are added if/when needed by &transpose_key() by rotating 'C' or 'Am'
my %transpose_keys = (
	'Cb'  => [ 'Cb',  'C',   'Db',  'D',   'Eb',  'E',   'F',   'Gb',  'G',   'Ab',  'A',   'Bb'  ],
	'Gb'  => [ 'Gb',  'G',   'Ab',  'A',   'Bb',  'Cb',  'C',   'Db',  'D',   'Eb',  'E',   'F'   ],
#	...
	'C'   => [ 'C',   'Db',  'D',   'Eb',  'E',   'F',   'F#',  'G',   'Ab',  'A',   'Bb',  'B'   ],
#	...
	'C#'  => [ 'C#',  'D',   'Eb',  'E',   'F',   'F#',  'G',   'Ab',  'A',   'Bb',  'B',   'C'   ],

	'Abm' => [ 'Abm', 'Am',  'Bbm', 'Bm',  'Cm',  'C#m', 'Dm',  'Ebm', 'Em',  'Fm',  'F#m', 'Gm'  ],
	'Ebm' => [ 'Ebm', 'Em',  'Fm',  'F#m', 'Gm',  'Abm', 'Am',  'Bbm', 'Bm',  'Cm',  'C#m', 'Dm'  ],
#	...
	'Am'  => [ 'Am',  'Bbm', 'Bm',  'Cm',  'C#m', 'Dm',  'D#m', 'Em',  'Fm',  'F#m', 'Gm',  'G#m' ],
#	...
	'A#m' => [ 'A#m', 'Bm',  'Cm',  'C#m', 'Dm',  'D#m', 'Em',  'Fm',  'F#m', 'Gm',  'G#m', 'Am'  ],
	);

my %numbers_to_notes = (
	'Cb'  => [qw/__d   d __e   e   f  =f   g __a   a __b   b   c /],
	'Gb'  => [qw/ =c   d __e   e  _f   f   g __a   a __b   b   c /],
	'Db'  => [qw/  c   d __e   e  _f   f   g  =g   a __b   b  _c /],
	'Ab'  => [qw/  c   d  =d   e  _f   f  _g   g   a __b   b  _c /],
	'Eb'  => [qw/  c  _d   d   e  _f   f  _g   g   a  =a   b  _c /],
	'Bb'  => [qw/  c  _d   d   e  =e   f  _g   g  _a   a   b  _c /],
	'F'   => [qw/  c  _d   d  _e   e   f  _g   g  _a   a   b  =b /],
	'C'   => [qw/  c  _d   d  _e   e   f  ^f   g  _a   a  _b   b /],
	'G'   => [qw/  c  ^c   d  _e   e  =f   f   g  _a   a  _b   b /],
	'D'   => [qw/ =c   c   d  _e   e  =f   f   g  ^g   a  _b   b /],
	'A'   => [qw/ =c   c   d  ^d   e  =f   f  =g   g   a  _b   b /],
	'E'   => [qw/ =c   c  =d   d   e  =f   f  =g   g   a  ^a   b /],
	'B'   => [qw/ =c   c  =d   d   e  ^e   f  =g   g  =a   a   b /],
	'F#'  => [qw/ ^b   c  =d   d  =e   e   f  =g   g  =a   a   b /],
	'C#'  => [qw/  b   c  =d   d  =e   e   f ^^f   g  =a   a  =b /],

	'Abm' => [qw/ =c   d  =d   e   f  =f   g  =g   a __b   b   c /],
	'Ebm' => [qw/ =c   d  =d   e  _f   f   g  =g   a  =a   b   c /],
	'Bbm' => [qw/  c   d  =d   e  =e   f   g  =g   a  =a   b  _c /],
	'Fm'  => [qw/  c   d  =d   e  =e   f  _g   g   a  =a   b  =b /],
	'Cm'  => [qw/  c  _d   d   e  =e   f  ^f   g   a  =a   b  =b /],
	'Gm'  => [qw/  c  ^c   d   e  =e   f  ^f   g  _a   a   b  =b /],
	'Dm'  => [qw/  c  ^c   d  _e   e   f  ^f   g  ^g   a   b  =b /],
	'Am'  => [qw/  c  ^c   d  ^d   e   f  ^f   g  ^g   a  _b   b /],
	'Em'  => [qw/  c  ^c   d  ^d   e  =f   f   g  ^g   a  ^a   b /],
	'Bm'  => [qw/ =c   c   d  ^d   e  ^e   f   g  ^g   a  ^a   b /],
	'F#m' => [qw/ ^b   c   d  ^d   e  ^e   f  =g   g   a  ^a   b /],
	'C#m' => [qw/ ^b   c  =d   d   e  ^e   f ^^f   g   a  ^a   b /],
	'G#m' => [qw/ ^b   c ^^c   d   e  ^e   f ^^f   g  =a   a   b /],
	'D#m' => [qw/ ^b   c ^^c   d  =e   e   f ^^f   g ^^g   a   b /],
	'A#m' => [qw/  b   c ^^c   d ^^d   e   f ^^f   g ^^g   a  =b /],
	);

$numbers_to_notes{+EMPTY} = $numbers_to_notes{'C'};

# These global variables are set by side-effect in &transpose_key_signature()
# and/or used freely by &transpose_line() and &number_to_note()

my $new_key = EMPTY;
my $within_tune = 0;

my $old_key_signature = $key_signatures{'C'};
my $new_key_signature = $key_signatures{'C'};

my %old_accidentals;
my %new_accidentals = %empty_accidentals;

my $tune_count = 0;

# Top level code

unless (@ARGV) {
	print STDERR "Usage: transpose_abc.pl [-nochords] offset {input_file} {output_file}\n";
	print STDERR "Usage: transpose_abc.pl [-nochords] offset < input_file > output_file\n";
	print STDERR "Example: transpose_abc.pl 3 song1.abc song2.abc\n";
	print STDERR "Offset is given in semitones, and may be positive or negative\n";
	exit 1;
}

# script relies on hard coded argument order
# => for support of optional arguments,
#    we must locate and remove them beforehand
my $NOCHORDS = 0;
my @newargv = ();
while (@ARGV) {
	my $arg = shift @ARGV;
	if ($arg eq "-nochords") {  # do not transpose chords
		$NOCHORDS = 1;
	} else {
		push(@newargv, $arg);
	}
}
@ARGV = @newargv;

if (2 == $#ARGV && $ARGV[2] =~ m/^[+-]?[\d]+$/) {
	# old argument order, rotate offset from last argument to first argument
	unshift(@ARGV, pop @ARGV);
	warn "% Reordering arguments: $0 @ARGV\n" if VERBOSE;
}

my ($OFFSET, $INPUT_FILE, $OUTPUT_FILE) = @ARGV;

if (defined $INPUT_FILE) {
	open(INPUT, $INPUT_FILE) or die "$INPUT_FILE: $!\n";
}
else {
	*INPUT = *STDIN;
}

if (defined $OUTPUT_FILE) {
	open(OUTPUT, ">$OUTPUT_FILE") or die "$OUTPUT_FILE: $!\n";
}
else {
	*OUTPUT = *STDOUT;
}

while (defined(my $line = <INPUT>)) {
	chomp $line;

	if ($line !~ m/\S/) {
		# print empty lines (e.g. between pieces) unchanged
		$within_tune = 0;
	}
	elsif ($line =~ m/^%%begin/ .. $line =~ m/^%%end/) {
		# print blocks unchanged (e.g. %%begintext or %%beginps)
	}
	elsif ($line =~ m/^%/) {
		# print full line comments unchanged
	}
	elsif ($line =~ m/^#/) {
		# print abcpp preprocessor lines unchanged (e.g. #ifdef)
	} 
	else {
		$line = &transpose_line($line);
	}
	
	print OUTPUT $line, "\n";
}

close OUTPUT if defined $OUTPUT_FILE;
close INPUT if defined $INPUT_FILE;

# Subroutines follow

sub transpose_line {
	my $line = shift;

	# Transpose to key in K: header line
	# Return other header field, part labels, etc., unchanged 
	# Header fields, part labels, etc. start with [a-zA-Z]:

	if ($line =~ m/^[A-Z]:/i) {
		$within_tune = 1;

		if ($line =~ m/^X:/i) {
			if (VERBOSE) {
				print STDERR "\n" if $tune_count++;
				print STDERR "% $line\n";
			}
		}
		elsif ($line =~ m/^K:/i) {
			my $transposed_line = &transpose_key_signature($line);

			# initialize or reset the the accidentals hashes
			%old_accidentals = %{$old_key_signature};
			%new_accidentals = %empty_accidentals;

			return $transposed_line;
		}

		return $line;
	}
	
	# we're not inside a tune yet, so return free text lines unchanged
	return $line unless $within_tune;

	# read tokens from @music, transpose them, and concatenate them onto $transposed_music

	# while characters left in @music
	# read a character
	# append it to existing token
	# or transpose the existing token
	# then concatenate it to $transposed_music
	# and then start a new token

	my $transposed_music;

	my @music = split(//, $line);

	while (defined(my $token = shift @music)) {

		SWITCH: {
			if ($token =~ m/[A-Ga-g]/) { # note
				while (@music && $music[0] =~ m/[,']/) { #'
					$token .= shift @music;
				}
				while (@music && $music[0] =~ m/[0-9\/]/) {
					$token .= shift @music;
				}

				$token = &transpose_note($token, $new_key, \%old_accidentals, \%new_accidentals);

				last SWITCH;
			}

			if ($token =~ m/[=_^]/) { # note
				while (@music && $music[0] =~ m/[_^]/) {
					$token .= shift @music;
				}
				if (@music && $music[0] =~ m/[A-Ga-g]/) {
					$token .= shift @music;
				}
				else {
					warn "% Malformed note: $token"
				}
				while (@music && $music[0] =~ m/[,']/) { #'
					$token .= shift @music;
				}
				while (@music && $music[0] =~ m/[0-9\/]/) {
					$token .= shift @music;
				}

				$token = &transpose_note($token, $new_key, \%old_accidentals, \%new_accidentals);

				last SWITCH;
			}

			if ($token eq '|') { # bar line, # reset accidentals
				%old_accidentals = %{$old_key_signature};
				%new_accidentals = %empty_accidentals;

				last SWITCH;
			}

			if ($token eq '"') { # guitar chord or free text string
				while (@music && $music[0] ne '"') {
					$token .= shift @music;
				}

				$token = &transpose_chord($token . shift(@music));

				last SWITCH;
			}

			if ($token eq '!') { # ornament or forced line break
				while (@music && $music[0] ne '!') {
					$token .= shift @music;
				}

				if (@music) {
					# This was probably an ornament
					$token .= shift @music;
				}

				last SWITCH;
			}

			if ($token eq '[') { # ending, inline field, invisible bar or chord!
				# if this isn't an ending, e.g. [2 or ["fine"
				if (@music && $music[0] !~ m/\d/ && $music[0] ne '"') {
					while (@music && $music[0] ne ']') {
						$token .= shift @music;
					}

					if (@music) {
						$token .= shift @music;
					}
					else {
						warn "% Unrecognized use of '[' in $token" if VERBOSE;
					}

					if ($token =~ m/^\[[A-Z]:/i) {
						# inline field [K:F treble]
						if ($token =~ m/^\[(K:.*)\]$/i) {
							$token = '[' . &transpose_key_signature($1) . ']'; # inline key change
						}
					}
					elsif ($token =~ m/^\[(.+)\]$/i) {
						# chord [A,G] or invisible bar [|]
						$token = '[' . &transpose_line($1) . ']';
					}
				}

				last SWITCH;
			}

			if ($token eq '%') { # EOL comment 
				$token .= join(EMPTY, @music);

				@music = ();

				last SWITCH;
			}
		}

		$transposed_music .= $token;
	}

	return $transposed_music;
}

sub transpose_key_signature {
	my $line = shift;
	
	if (EMULATE_ABC2ABC && $OFFSET) {
		$line =~ s/([^=])(treb|alto|teno|bari|bass|mezz|sopr|perc|none)/$1clef=$2/i
	}

	if (my ($field, $old_display_key, $old_global_accidentals, $rest) = ($line =~ m/^
		(K:\s*) # field name and optional whitespace
		([A-G][#b]?(?:[A-Z]{1,10})?)? # optional key and optional mode if key
		((?:\s*[=^_]{1,2}[A-G])*) # optional global accidentals
		(.*) # everything else that's allowed including a comment
		$/ix)) {

		return $line unless defined $old_display_key;

		# only a clef parameter but only worry about clefs that start with [A-G]
		return $line if ($old_display_key =~ m/(clef|alto|baritone|bass)/i);

		# truncate mode to 3 letters or less (should really preserve complete mode, but...)
		if ($old_display_key =~ m/^([A-G][b#]?[A-Z]{0,3})/i) {
			$old_display_key = $1;
			}

		# standardize the $old_key to its relative major, if needed
		my $old_key = $relative_keys{uc $old_display_key};

		warn "% Invalid (old) key: $old_display_key" unless defined $old_key;

		# get old key signature
		$old_key_signature = &get_key_signature($old_key, $old_global_accidentals);

		# get the $new_key as a major key, which we'll use internally
		$new_key = &transpose_key($old_key);

		# get the $new_display_key, something matching $old_display_key
		my $new_display_key = &get_new_display_key($old_display_key, $new_key);

		# transpose the key signature modifiers
		# my $new_global_accidentals = lc &transpose_line($old_global_accidentals);
		# $new_global_accidentals =~ s/[',]//g; #'

		# I haven't quite thought through how to implement modifiers
		# in the new key. So for now, I'm going to ignore them.
		# The pitches will transpose correctly, but the notation will
		# be done in the unmodified new key.
		my $new_global_accidentals = EMPTY; # this line goes away eventually...

		# get new key signature
		$new_key_signature = &get_key_signature($new_key, $new_global_accidentals);

		if (VERBOSE) {
			print STDERR "% Old ${field}${old_display_key} (";
			print STDERR join(' ', &get_scale($old_key, $old_key_signature));
			print STDERR ")\n";

			print STDERR "% New ${field}${new_display_key} (";
			print STDERR join(' ', &get_scale($new_key, $new_key_signature));
			print STDERR ")\n";
			}

		# return new K: header line
		my $transposed_line = $field . $new_display_key . $new_global_accidentals . $rest;

		return $transposed_line;
	}

	# no specific key mentioned, e.g. K:treble
	return $line;
}

sub get_new_display_key {
	my ($old_display_key, $new_key) = @_;

	# Find $old_display_key in the array, and note the column in which you found it.
	# Find $new_key in the array, and note the row in which you found it.
	# In that row, go the column where you found $old_display_key.
	# That's your new display key.
	
	my $row_where_found;

	for (my $row = 0; $row < @display_keys; $row++) {
		my $keys = $display_keys[$row];

		if ($keys->[0] eq $new_key) {
			$row_where_found = $row;
			last;
		}
	}

	warn "% Invalid (new) key: $new_key" unless defined $row_where_found;

	my $old_key = uc $old_display_key;

	my $col_where_found;

outer:	for (my $row = 0; $row < @display_keys; $row++) {
		my $keys = $display_keys[$row];

		for (my $col = 0; $col < @{$keys}; $col++) {
			if (uc($keys->[$col]) eq $old_key) {
				$col_where_found = $col;
				last outer;
			}
		}
	}
	
	warn "% Invalid (old) key: $old_display_key" unless defined $col_where_found;
	
	if (EMULATE_ABC2ABC && $OFFSET && 0 == $col_where_found && $row_where_found < @display_keys / 2) {
		$col_where_found++;
	}

	my $new_display_key = $display_keys[$row_where_found][$col_where_found];
	
	if (EMULATE_ABC2ABC && $OFFSET) {
		$new_display_key = ucfirst lc $new_display_key;
	}

	return $new_display_key;
}

sub get_key_signature {
	my ($key, $accidentals) = @_;

	warn "% Invalid key: $key" unless exists $key_signatures{$key};

	my %signature = (%{$key_signatures{'C'}}, %{$key_signatures{$key}});

	while ($accidentals =~ m/\^([a-g])/g) {
		$signature{$1} = SHARP;
	}
	while ($accidentals =~ m/\^\^([a-g])/g) {
		$signature{$1} = DOUBLE_SHARP;
	}
	while ($accidentals =~ m/_([a-g])/g) {
		$signature{$1} = FLAT;
	}
	while ($accidentals =~ m/__([a-g])/g) {
		$signature{$1} = DOUBLE_FLAT;
	}
	while ($accidentals =~ m/=([a-g])/g) {
		$signature{$1} = NATURAL;
	}

	return \%signature;
}

sub transpose_note {
	my ($note, $key, $old_accidentals, $new_accidentals) = @_;

	my ($explicit, $number, $octave, $duration) = &note_to_number($note, $old_accidentals);

	if ($OFFSET) {
		$number += $OFFSET;

		if ($OFFSET > 0) {
			$octave += int($number / OCTAVE);
		}
		else {
			$octave += int(($number - (OCTAVE-1)) / OCTAVE);
		}
	
		$number %= OCTAVE;
	}
	
	my $transposed_note = &number_to_note($explicit, $number, $octave, $duration, $key, $new_accidentals);

	return $transposed_note;
}

sub note_to_number {
	my ($note, $accidentals) = @_;

	my $accidental = NATURAL;
	my $explicit_natural = ($note =~ y/=//d);

	if (! $explicit_natural) {
		if (NATURAL == ($accidental = ($note =~ y/^//d))) {
			$accidental = - ($note =~ y/_//d);
		}
	}
	
	my $octave = ($note =~ y/A-G/a-g/) ? -1 - ($note =~ y/,//d) : ($note =~ y/'//d); #'

	my $duration = EMPTY;

	if ($note =~ s/([0-9\/]+)$//) {
		$duration = $1;
	}

	my $explicit_accidental = ($explicit_natural || $accidental) && ($accidental == $accidentals->{$note});

	my $number = $notes_to_numbers{$note};

	warn "% Invalid note: $note" unless defined $number;

	if ($explicit_natural) {
		unless ($explicit_accidental) {
			$accidentals->{$note} = NATURAL;
		}
	}
	elsif ($accidental && ! $explicit_accidental) {
		$accidentals->{$note} = $accidental;
		$number += $accidental; 
	}
	else {
		$number += $accidentals->{$note}; 
	}

	if (EMULATE_ABC2ABC && $duration =~ m{^/+$}) {
		$duration = '/' . 2 ** ($duration =~ y|/||d);
	}

	return ($explicit_accidental, $number, $octave, $duration);
}

sub number_to_note {
	my ($explicit_accidental, $number, $octave, $duration, $key, $accidentals) = @_;

	warn "% Invalid key: $key" unless exists $numbers_to_notes{$key};

	my $note = $numbers_to_notes{$key}->[$number];
	my $pitch = substr($note, -1, 1);
	my $accidental = substr($note, 0, (length($note) - 1));

	if ($accidental) {
		if ($accidentals->{$pitch} eq $accidental) {
			$note = $pitch unless $explicit_accidental;
		}
		else {
			$accidentals->{$pitch} = $accidental;
		}
	}
	elsif ($accidentals->{$pitch}) {
		$note = $accidental_to_symbol{$new_key_signature->{$pitch}} . $note;
		
		$accidentals->{$pitch} = EMPTY;
	}
	elsif ($explicit_accidental) {
		$note = $accidental_to_symbol{$new_key_signature->{$pitch}} . $note;
	}
	
	if ($pitch eq 'b') {
		# In some keys, c is written as ^b so it actually falls in the octave below
		$octave -= 1 if (C == $number);
	}
	elsif ($pitch eq 'c') {
		# In some keys, b is written as _c so it actually falls in the octave above
		$octave += 1 if (B == $number);
	}

	if ($octave > 0) {
		$note .= "'" x $octave;
	}
	elsif ($octave < 0) {
		$note = uc($note) . ',' x (-1 - $octave);
	}
	
	$note .= $duration if $duration;

	return $note;
}

sub transpose_chord {
	my $text = shift;

	# If the chord starts with one of [_<>@^] then it is a free
	# text string and not a guitar chord.  Return it unchanged.
	if ($NOCHORDS || $text =~ m/^"[_<>@^]/ ) { #"
		return $text;
	}
	
	my $transposed_chord = EMPTY;

	while ($text =~ m/
		([A-G][b#]?) # root
		([A-Z0-9+-]+)? # optional chord type
		(?:\/([A-G][b#]?))? # optional base note
		(;|\\n)? # optional multiple chords delimiter
		/gxi) {

		my ($root, $type, $bass, $delimiter) = ($1, $2, $3, $4);

		$transposed_chord .= &transpose_key($root);

		$transposed_chord .= $type if defined $type;

		$transposed_chord .= '/' . &transpose_key($bass) if defined $bass;

		$transposed_chord .= $delimiter if defined $delimiter;
	}

	if ($transposed_chord eq EMPTY) {
		warn "% Untagged [_<>@^] free text: $text" if VERBOSE;
		return $text;
	}
	else {
		$transposed_chord = '"' . $transposed_chord . '"';
	}

	return $transposed_chord;
}

sub transpose_key {
	my $old_key = shift;

	my $hash_key = ucfirst lc $old_key;

	warn "% Invalid (old) key: $old_key" unless exists $key_signatures{$hash_key};

	unless (exists $transpose_keys{$hash_key}) {
		my $base_key = $transpose_keys{&is_minor_key($hash_key) ? 'Am' : 'C'};

		$transpose_keys{$hash_key} = [ &rotate_notes($hash_key, @{ $base_key }) ];
		}

	my $transposed_key = $transpose_keys{$hash_key}->[$OFFSET % OCTAVE];

	return $transposed_key;
}

sub is_minor_key { $_[0] =~ m/m$/ }

sub get_scale {
	my ($key, $key_signature) = @_;
	
	my @pitches = rotate_notes(lc substr($key, 0, 1), ('a' .. 'g'));

	my @scale = map { ucfirst($_ . $accidental_to_text{$key_signature->{$_}}) } @pitches;

	return @scale;
}

sub rotate_notes {
	my ($note, @notes) = @_;

	if ($note ne $notes[0]) {
		for (my $i = 0; $i < @notes; $i++) {
			if ($note eq $notes[$i]) {
				push(@notes, splice(@notes, 0, $i));
				last;
			}
		}

		warn "% $note not a member of (@notes)\n" if ($note ne $notes[0]);
	}

	return @notes;
}
