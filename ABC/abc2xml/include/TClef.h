/*

  MusicXML Library
  Copyright (C) 2003  Grame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  Grame Research Laboratory, 9 rue du Garet, 69001 Lyon - France
  grame@grame.fr

*/

#ifndef __TClef__
#define __TClef__

#include "exports.h"
#include "TScoreVisitor.h"
#include "bimap.h"

#include <string>

namespace MusicXML 
{

/*!
\brief Represents a clef.

	Clefs are represented by the sign, line, and
	clef-octave-change elements. Sign values include G, F, C,
	percussion, TAB, and none. Line numbers are counted from
	the bottom of the staff. Standard values are 2 for the
	G sign (treble clef), 4 for the F sign (bass clef), 3
	for the C sign (alto clef) and 5 for TAB (on a 6-line
	staff). The clef-octave-change element is used for
	transposing clefs (e.g., a treble clef for tenors would
	have a clef-octave-change value of -1). The optional
	number attribute refers to staff numbers, from top to
	bottom on the system. A value of 1 is assumed if not
	present.
\n	TClef corresponds to the MusicXML \e clef element as defined
	in attributes.dtd
*/
class EXP TClef : public visitable, public smartable {
    
    public:
		enum { undefined=-1, none=1, G, F, C, percussion, TAB, last=TAB };
    
        EXP friend SMARTP<TClef> newClef();

		virtual void accept(TScoreVisitor& visitor);
        
		void setStaffNum 	(long num);
		void setSign 		(long sign);
		void setLine 		(long line);
		void setOctave 		(long oct);

		long getStaffNum () const	{ return fNum; }
		long getSign 	() const	{ return fSign; }
		long getLine 	() const	{ return fLine; }
		long getOctave 	() const	{ return fOctave; }

	//! convert an integer clef sign to a MusicXML string
	static const string	xmlsign (int d);
	//! convert an MusicXML string to an integer clef sign
	static int	xmlsign (const string str);

    protected:
        TClef() : fNum(undefined), fSign(G), fLine(undefined), fOctave(0) {}
        virtual ~TClef() {}

    private:
        //! optionnal number attribute that refers to staff numbers
		long 	fNum;
        //! required sign element
        long 	fSign;
        //! optionnal line number element, counted from the bottom of the staff
        long 	fLine;
        //! optionnal clef-octave-change element for transposing
        long 	fOctave;

	static bimap<string, int> fClef2String;
	static int 		fClefTbl[];
	static string 	fClefStrings[];
};
typedef SMARTP<TClef> SClef;

} // namespace MusicXML

#endif
