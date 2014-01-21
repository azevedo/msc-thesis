/*

  MusicXML Library
  Copyright (C) 2003-2004  Grame

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
  research@grame.fr

*/

#ifndef __TStaffDetails__
#define __TStaffDetails__

#include "exports.h"

#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

namespace MusicXML
{

/*!
\brief the staff tuning element (for staff details)

	The tuning-step, tuning-alter, and tuning-octave
	elements are represented like the step, alter, and
	octave elements, with different names to reflect their
	different function. Staff lines are numbered from
	bottom to top.
*/
class EXP TStaffTuning : public visitable, public smartable {

    public:
        EXP friend SMARTP<TStaffTuning> newStaffTuning(int line);
		virtual void accept(TScoreVisitor& visitor);

        const string& 	getStep() const 	{ return fStep; }
        float 	getAlter() const 	{ return fAlter; }
        int 	getOctave() const 	{ return fOctave; }
        int 	getLine() const 	{ return fLine; }

        void 	setStep(const string& step)		{ fStep = step; }
        void 	setAlter(float alter)   { fAlter = alter; }
        void	setOctave(int oct)		{ fOctave = oct; }
		void	setLine(int line)		{ fLine = line; }

    protected:
        TStaffTuning(int line) : fLine(line), fOctave(0), fAlter(0) {}
        virtual ~TStaffTuning() {};
    private:
		int		fLine, fOctave;
		string	fStep;
		float   fAlter;
};
typedef SMARTP<TStaffTuning> SStaffTuning;

/*!
\brief Represents staff details.

	The staff-details element is used to indicate different
	types of staves. The staff-type element can be ossia,
	cue, editorial, regular, or alternate. An alternate
	staff indicates one that shares the same musical data
	as the prior staff, but displayed differently (e.g.,
	treble and bass clef, standard notation and tab).
	The staff-lines element specifies the number of lines for
	non 5-line staffs. The staff-tuning and capo elements are
	used to specify tuning when using tablature notation.
	The optional number attribute specifies the staff number
	from top to bottom on the system, as with clef. The
	optional show-frets attribute indicates whether to show
	tablature frets as numbers (0, 1, 2) or letters (a, b, c).
	The default choice is numbers.
*/
class EXP TStaffDetails : public visitable, public smartable {

    public:
		enum { undefined=-1, numbers, letters };
		enum type { undefType, ossia, cue, editorial, regular, alternate, last=alternate};

        EXP friend SMARTP<TStaffDetails> newStaffDetails();
		virtual void accept(TScoreVisitor& visitor);

        void 	setType(type t);
        void 	setNumber(int n);
        void 	setShowFret(int t);
        void 	setLines(int n);
        void 	setCapo(const string& capo);
        void 	add(SStaffTuning st);

        int 	getNumber() const		{ return fNumber; }
        int 	getShowFret() const		{ return fShowFrets; }
        int 	getLines() const		{ return fLines; }
        type	getType() const			{ return fType; }
		const string& getCapo() const   { return fCapo; }
        vvector<SStaffTuning>& tuning() { return fTuning; }

    //! convert a numeric staff type to a MusicXML string
    static const string	xml (type d);
    //! convert a MusicXML string to a numeric staff type
    static       type	xml (const string str);
		
    protected:
        TStaffDetails() 
			: fNumber(undefined), fShowFrets(undefined), fLines(undefined), fType(undefType) {}
        virtual ~TStaffDetails() {};
    private:
		int		fNumber, fShowFrets, fLines;
		type	fType;
		string  fCapo;
		vvector<SStaffTuning>   fTuning;

	static bimap<string, type> fType2String;
	static type 	fTypeTbl[];
	static string 	fTypeStrings[];        
};
typedef SMARTP<TStaffDetails> SStaffDetails;


} // namespace MusicXML


#endif
