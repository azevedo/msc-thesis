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

#ifndef __TKey__
#define __TKey__

#include "exports.h"

#include <string>

#include "bimap.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

/*!
\brief Represents a key signature.

	Traditional key signatures are represented by the number
	of flats and sharps, plus an optional mode for major/
	minor/mode distinctions. Negative numbers are used for
	flats and positive numbers for sharps, reflecting the
	key's placement within the circle of fifths (hence the
	element name). A cancel element indicates that the old
	key signature should be cancelled before the new one
	appears. This will always happen when changing to C major
	or A minor and need not be specified then. The cancel
	value matches the fifths value of the cancelled key
	signature (e.g., a cancel of -2 will provide an explicit
	cancellation for changing from B flat major to F major).
\n	
	Non-traditional key signatures can be represented using
	the Humdrum/Scot concept of a list of altered tones.
	The key-step and key-alter elements are represented the
	same way as the step and alter elements are in the pitch
	element in note.dtd. The different element names indicate
	the different meaning of altering notes in a scale versus
	altering a sounding pitch.
\n	
	Valid mode values include major, minor, dorian, phrygian,
	lydian, mixolydian, aeolian, ionian, and locrian.
\n
	TKey corresponds to the MusicXML \e key element as defined 
	in attributes.dtd.
*/
class EXP TKey : public visitable, public smartable {
    
    public:
        enum { undefined=-1 };
        enum { major=1, minor, dorian, phrygian, lydian, mixolydian, aeolian, 
                ionian, locrian, last=locrian };

        EXP friend SMARTP<TKey> newKey();
        
        void setCancel(long cancel);
        void setFifths(long fifths);
        void setMode(long mode);

        long getCancel() const			{ return fCancel; }
        long getFifths() const			{ return fFifths; }
        long getMode() const			{ return fMode; }

		virtual void accept(TScoreVisitor& visitor);
 
	//! convert an integer mode to a MusicXML string
	static const string	xmlmode (int d);
	//! convert an MusicXML string to an integer mode
	static int	xmlmode (const string str);
   
    protected:
        TKey() : fCancel(0), fFifths(0), fMode(undefined) {}
        virtual ~TKey() {}

    private:
        long 	fCancel;
        long 	fFifths;
        long 	fMode;

	static bimap<string, int> fMode2String;
	static int 		fModeTbl[];
	static string 	fModeStrings[];
};
typedef SMARTP<TKey> SKey;

} // namespace MusicXML


#endif
