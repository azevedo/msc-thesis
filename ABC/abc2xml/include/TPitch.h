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

#ifndef __TPitch__
#define __TPitch__

#include "exports.h"
#include "bimap.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include <string>

namespace MusicXML 
{

/*!
\brief The MusicXML representation of the pitch.

	Pitch is represented as a combination of the step of the
	diatonic scale, the chromatic alteration, and the octave.
	The step element uses the English letters A through G:
	in a future revision, this could expand to international
	namings. The alter element represents chromatic 
	alteration in number of semitones (e.g., -1 for flat,
	1 for sharp). Decimal values like 0.5 (quarter tone
	sharp) may be used for microtones. The octave element
	is represented by the numbers 0 to 9, where 4 indicates
	the octave started by middle C.
\n
    The MusicXML \e pitch element is defined in note.dtd.
*/
class EXP TPitch : public visitable, public smartable {

      public:
        enum { undefined=0, C=1, D, E, F, G, A, B, last=B, diatonicSteps=last };
        EXP friend SMARTP<TPitch> newPitch();
        
        virtual void accept(TScoreVisitor& visitor);

        int 	getStep() const 	{ return fStep; }
        float 	getAlter() const 	{ return fAlter; }
        int 	getOctave() const 	{ return fOctave; }

        void 	setStep(int step);
        void 	setAlter(float alter);
        void	setOctave(int oct);

	//! convert a numeric pitch to a MusicXML string
	static const string	xmlpitch (int d);
	//! convert a MusicXML string to a numeric pitch
	static int			xmlpitch (const string str);

        //! the ++ operator adds a sharp
        TPitch&			operator++ (int)	{ fAlter+=1; return *this; }
        //! the -- operator adds a flat
        TPitch&			operator-- (int)	{ fAlter-=1; return *this; }

        TPitch&			operator+= (int n);
        //! adds n diatonic steps to the pitch field and update the octave when necessary
        TPitch&			operator+= (unsigned int n);

        TPitch&			operator-= (int n);
        //! substracts n diatonic steps to the pitch field and update the octave when necessary
        TPitch&			operator-= (unsigned int n);

        //! returns the corresponding MIDI pitch
        unsigned short 	MIDIPitch () const;
    
    protected:
        TPitch() : fStep(undefined), fAlter(0), fOctave(0) {}
        virtual ~TPitch() {}

        int 	fStep;
        float 	fAlter;
        int 	fOctave;

    private:
	static bimap<string, int> fPitch2String;
	static int 		fPitchTbl[];
	static string 	fPitchStrings[];
};
typedef SMARTP<TPitch> SPitch;

/*!
\brief The MusicXML representation of an unpitched note.

	The unpitched element indicates musical elements that are
	notated on the staff but lack definite pitch, such as
	unpitched percussion and speaking voice. Like notes, it
	uses step and octave elements to indicate placement on
	the staff, following the current clef. If percussion clef
	is used, the display-step and display-octave are
	interpreted as if in treble clef, with a G in octave 4 
	on line 2. If not present, the note is placed on the 
	middle line of the staff, generally used for one-line
	staffs.
\n
    The MusicXML \e unpitched element is defined in note.dtd.
*/
class EXP TUnpitched : public TPitch {
    public:
        EXP friend SMARTP<TUnpitched> newUnpitched();
        virtual void accept(TScoreVisitor& visitor);

    protected:
        TUnpitched() {}
        virtual ~TUnpitched() {}
};
typedef SMARTP<TUnpitched> SUnpitched;

/*!
\brief The MusicXML representation of a rest.

	The rest element indicates notated rests or silences.
	Rest are usually empty, but placement on the staff can
	be specified using display-step and display-octave
	elements.
\n
    The MusicXML \e rest element is defined in note.dtd.
*/
class EXP TRest : public TPitch {
      public:
        EXP friend SMARTP<TRest> newRest();
        virtual void accept(TScoreVisitor& visitor);

    protected:
        TRest() {}
        virtual ~TRest() {}
};
typedef SMARTP<TRest> SRest;

} // namespace MusicXML


#endif
