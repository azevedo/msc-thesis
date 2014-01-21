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

#ifndef __TScoreInstrument__
#define __TScoreInstrument__

#include "exports.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include <string>

namespace MusicXML 
{
/*!
\brief 	Represents an instrument on the score.

    The score-instrument element allows for multiple
	instruments per score-part. As with the score-part
	element, each score-instrument has a required ID
	attribute, a name, and an optional abbreviation.
	A score-instrument element is also required if the
	score specifies MIDI channels, banks, or programs.
	An initial midi-instrument assignment can also
	be made here. MusicXML software should be able to
	automatically assign reasonable channels and 
	instruments without these elements in simple cases,
	such as where part names match General MIDI
	instrument names.
\n
    The MusicXML \e score-instrument is defined in score.dtd.
*/
class EXP TScoreInstrument : public visitable, public smartable {
    
    public:
        EXP friend SMARTP<TScoreInstrument> newScoreInstrument(std::string id);

        virtual void accept(TScoreVisitor& visitor);

		//! a MusicXML required attribute used for midi-instrument assignment.
        void 	setID(const std::string& id);
		//! the instrument name.
        void 	setName(const std::string& name);
		//! the optional name abbreviation.
        void 	setAbbrev(const std::string& abbrev);

        const std::string& 	getID() const		{ return fID; }
        const std::string& 	getName() const		{ return fName; }
        const std::string& 	getAbbrev() const	{ return fAbbrev; }

    protected:
        TScoreInstrument(std::string id) : fID(id) {}
        virtual ~TScoreInstrument() {}
    private:
        std::string 	fID;
        std::string 	fName;
        std::string 	fAbbrev;
};
typedef SMARTP<TScoreInstrument> SScoreInstrument;

} // namespace MusicXML


#endif
