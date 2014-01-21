#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef linux
#include <unistd.h>
#endif


#include "config.h"
#include "amautils.h"
#include "abcparser.h"
#include "abc2xmlconverter.h"

#include "LibMusicXML.h"
#ifdef WIN32
#include <windows.h>
#endif

using namespace MusicXML;



void usage() {
	
	
}


//creates xml from one abctune
void convert_to_xml( struct abctune * myabctune, char * filename, AbcParser * abcparser)
{

	AbcToXmlConverter * converter = new AbcToXmlConverter(abcparser, myabctune);
    // and finally writes the score to the standard output
	// TODO: usefilename

    TMusicXMLFile file;
    file.write(converter->convert_abc_to_xml(), cout);   // convert and write result
}




int main (int argc, char * argv[]) {

	if (argc <= 1) 
		usage();

//	if (! initialize()) FATAL_ERROR(("could not initialize"));



	//read ABC
	AbcParser * abcparser = new AbcParser();
	
	//parse_arguments();

	
	//load = read + parse
	struct abctune * myabctune;

	if ((myabctune = abcparser->load_file(argv[1])) == NULL) 
	{
				OUT_DEBUG(("error load file"));

		#if defined(unix) || defined(__unix__)
			perror("load_file");
		#endif
		fprintf(stderr, "++++ Cannot read input file '%s'\n", argv[1]);	
		
		//TODO: cleanup
		OUT_DEBUG(("error load file - ending"));

		return false;
	};
	
	OUT_DEBUG(("file loaded"));
	
	//cycle through tunes in file
	struct abcsym * abcsymbol;
	int i;
//	while (myabctune != 0) {
		//i = -32000;
		//getting number of tune into variable i
		for (	abcsymbol = myabctune->first_sym; 
				abcsymbol != 0; 
				abcsymbol = abcsymbol->next	) {
			if (abcsymbol->type == ABC_T_INFO && abcsymbol->text[0] == 'X') {
				sscanf(abcsymbol->text, "X:%d", &i);
				break;
			}
		}
//		print_tune = i	 >= first_tune && i <= last_tune;
		if (!myabctune->client_data) {	/* (parse the global symbols) */
		OUT_DEBUG(("pisen s id=%d nalezena", i));

			//TODO druhy parametr je outputfilename
			convert_to_xml(myabctune,argv[1],abcparser);
//			do_tune(myabctune, !print_tune);
			
			myabctune->client_data = (void *) 1;	/* treated */
		}
//		if (i >= last_tune)
//			break;
//		myabctune = myabctune->next;
//	}

	return true;
}

