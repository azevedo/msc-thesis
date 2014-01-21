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

#ifndef __TScorePart__
#define __TScorePart__

#include "exports.h"
#include "common.h"
#include "TIdentification.h"
#include "TMidiDevice.h"
#include "TMidiInstrument.h"
#include "TScoreInstrument.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

/*!
\brief Identifies a musical part in a score. 
	
	Each part has an ID that is used later
	within the musical data. Since parts may be encoded
	separately and combined later, identification elements
	are present at both the score and score-part levels.
\n	
	Each MusicXML part corresponds to a track in a Standard
	MIDI Format 1 file. The score-instrument elements are
	used when there are multiple instruments per track.
	The midi-device element is used to make a MIDI device
	or port assignment for the given track. Initial
	midi-instrument assignments may be made here as well.
\n	
	TScorePart corresponds to the MusicXML \e score-part element 
	as defined in score.dtd.
*/

class EXP TScorePart : public TPartListElement {
    
    public:
        EXP friend SMARTP<TScorePart> newScorePart(string id);

        virtual void accept(TScoreVisitor& visitor);
   
        const string& 	getID() const				{ return fID; }
        const string& 	getPartName() const			{ return fPartName; }
        const string& 	getPartAbbrev() const	 	{ return fPartAbbrev; }
        vvector<SScoreInstrument>& 	scoreInstruments() 	{ return fScoreInstrumentList; }
        vvector<SMidiInstrument>& 	midiInstruments()	{ return fMidiInstrumentList; }

        void 	setID(const string& id);
        void 	setPartName(const string& name);
        void 	setPartAbbrev(const string& abbrev);

        long	add(const SScoreInstrument& instr);
        long	add(const SMidiInstrument& instr);
        
        SIdentification& identification()	{ return fIdentification; }
        SMidiDevice& mididevice()			{ return fMidiDevice; }
    
    protected:
        TScorePart(string id) : fID(id) {}
        virtual ~TScorePart() {}

    private:
        string 					fID;
        SIdentification 		fIdentification;
        string 					fPartName;
        string 					fPartAbbrev;
        vvector<SScoreInstrument> fScoreInstrumentList;
        SMidiDevice 			fMidiDevice;
        vvector<SMidiInstrument> fMidiInstrumentList;
};
typedef SMARTP<TScorePart> SScorePart;

} // namespace MusicXML


#endif
