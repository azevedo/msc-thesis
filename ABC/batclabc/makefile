main.out: main.c abcparse.c areana.c
	cc -g main.c abcparse.c areana.c

c2perl: c2perl.c abcparse.c areana.c
	cc -g c2perl.c abcparse.c areana.c

clean:
	rm aux-abc2perl

install: aux-abc2perl
	cp aux-abc2perl /usr/local/bin/

aux-abc2perl:	c2perl.c abcparse.c areana.c
	cc -o aux-abc2perl -g c2perl.c abcparse.c areana.c

