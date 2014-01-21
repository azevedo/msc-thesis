abc2perl: abc2perl.c abcparse.c areana.c
	cc -g abc2perl.c abcparse.c areana.c

clean:
	rm aux-abc2perl

install: aux-abc2perl
	cp aux-abc2perl /usr/local/bin/

aux-abc2perl:	abc2perl.c abcparse.c areana.c
	cc -o aux-abc2perl -g abc2perl.c abcparse.c areana.c

