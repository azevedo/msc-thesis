P={paste_abc,cat_abc,learning_abc,canon_abc,find_chords_abc,wc_abc,detect_errors_abc}
V=../corpus/verbum
C=../corpus/pach_canon
T=test
SHELL=/bin/bash

t1:
	paste_abc $V/1*.abc > a.abc
	paste_abc $V/2*.abc > b.abc
	paste_abc $V/3*.abc > c.abc
	paste_abc $V/4*.abc > d.abc
	paste_abc $V/5*.abc > e.abc
	cat_abc [a-e].abc > verbum.abc
	cat_abc -d=2 a.abc > a_d2.abc
	cat_abc -r=1 a.abc > a_r1.abc
	learning_abc -voice 3 verbum.abc
	learning_abc -voice Tenor verbum.abc
	canon_abc $C/melody.abc+8 $C/melody.abc+16 $C/melody.abc+24 $C/bass.abc++ > pach_canon.abc
	find_chords_abc -c=maj_t -v=1 pach_canon.abc > pach_canon_with_maj_chords.abc
	wc_abc pach_canon.abc > wc
	detect_errors_abc $T/tune_multiple_errors.abc > tune_multiple_errors
	detect_errors_abc $T/tune_errors_key.abc > tune_errors_key


t2:
	diff a.abc $T/100.abc > /dev/null || echo Better check paste_abc >&2;
	diff b.abc $T/200.abc > /dev/null || echo Better check paste_abc >&2;
	diff c.abc $T/300.abc > /dev/null || echo Better check paste_abc >&2;
	diff d.abc $T/400.abc > /dev/null || echo Better check paste_abc >&2;
	diff e.abc $T/500.abc > /dev/null || echo Better check paste_abc >&2;
	diff verbum.abc $T/v.abc > /dev/null || echo Better check cat_abc >&2;
	diff a_d2.abc $T/ad2.abc > /dev/null || echo Better check cat_abc >&2;
	diff a_r1.abc $T/ar1.abc > /dev/null || echo Better check cat_abc >&2;
	diff verbum_all_but_3.abc 		$T/v_all.abc > /dev/null || echo Better check learning_abc >&2;
	diff verbum_just_3.abc 				$T/v_just.abc > /dev/null || echo Better check learning_abc >&2;
	diff verbum_all_but_Tenor.abc $T/v_all.abc > /dev/null || echo Better check learning_abc >&2;
	diff verbum_just_Tenor.abc 		$T/v_just.abc > /dev/null || echo Better check learning_abc >&2;
	diff pach_canon.abc $T/pcanon.abc > /dev/null || echo Better check canon_abc >&2;
	diff pach_canon_with_maj_chords.abc $T/pcanon_maj.abc > /dev/null || echo Better check find_chords_abc >&2;
	diff wc $T/wc > /dev/null || echo Better check wc_abc >&2;
	diff tune_multiple_errors $T/tune_multiple_errors > /dev/null || echo Better check detect_errors_abc >&2;
	diff tune_errors_key $T/tune_errors_key > /dev/null || echo Better check detect_errors_abc >&2;

install:
	cp    ./$P  /usr/local/bin
	chmod 755   /usr/local/bin/$P

clean:
	rm [a-e].abc verbum*.abc a_d2.abc a_r1.abc pach_canon*.abc wc tune_multiple_errors tune_errors_key
