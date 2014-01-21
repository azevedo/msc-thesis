/*

  MusicXML Library
  Copyright (C) 2004  Grame

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

#ifndef __TTechnical__
#define __TTechnical__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

#include <string>

namespace MusicXML 
{

/*!
\brief basic technical representation

	The basic TTechnical class is enough to cover almost all of 
	the MusicXML defined technicals which are empty elements
	including position and placement entities. 
	For simplification, a data field is included and the string and fret 
	elements are embedded although they don't carry a position and a placement.
*/
class EXP TTechnical : public Positionable, public Placementable, public smartable, public visitable {
    public:
        enum type { undefined, up_bow, down_bow, harmonic, open_string,
					thumb_position, fingering, double_tongue, triple_tongue, stopped, snap_pizzicato,
					fret, _string, hammer_on, pull_off, bend, tap, heel, toe, fingernails, 
					other_technical, last=other_technical };
        
        EXP friend SMARTP<TTechnical> newTechnical(type t);
        virtual void accept(TScoreVisitor& visitor);
        
        void 	setType(type t);
        type	getType() const			{ return fType; }
		void				setData(const std::string& data)	{ fData = data; }
		const std::string&	getData() const						{ return fData; }

    //! convert a numeric technical type to a MusicXML string
    static const std::string	xml (type d);
    //! convert a MusicXML string to a numeric technical type
    static       type	xml (const std::string str);
	
	static const string* geTechnicalStrings()   { return fTypeStrings; }

    protected:
        TTechnical(type t) : fType(t) {}
        virtual ~TTechnical() {}
	private:
		type	fType;
		std::string  fData;
	static bimap<std::string, type> fType2String;
	static type			fTypeTbl[];
	static std::string 	fTypeStrings[];        
};
typedef SMARTP<TTechnical> STechnical;

/*!
\brief MusicXML harmonic element.

	The harmonic element indicates natural and artificial
	harmonics. Natural harmonics usually notate the
	base pitch rather than the sounding pitch. Allowing
	the type of pitch to be specified, combined with
	MusicXML's controls for appearance/playback 
	differences, allows both the notation and the sound
	to be represented. Artificial harmonics can add a 
	notated touching-pitch; the pitch or fret at which
	the string is touched lightly to produce the harmonic.
	Artificial pinch harmonics will usually not notate
	a touching pitch.
*/
class EXP THarmonic : public TTechnical {
    public:
		enum harmonict { undefined, natural, artificial };
		enum pitch { undefined_pitch, base_pitch, touching_pitch, sounding_pitch };
		
        EXP friend SMARTP<THarmonic> newHarmonic();
        virtual void accept(TScoreVisitor& visitor);
		void		setHarmonic(harmonict t) { fHarmonic = t; }
		void		setPitch(pitch p)		{ fPitch = p; }
		harmonict	getHarmonic() const		{ return fHarmonic; }
		pitch		getPitch() const		{ return fPitch; }

    //! convert a numeric harmonic to a MusicXML string
    static const std::string	xmlHarmonic (harmonict d);
    //! convert a MusicXML std::string to a numeric harmonic
    static       harmonict xmlHarmonic (const std::string str);
    //! convert a numeric pitch to a MusicXML string
    static const std::string	xmlPitch (pitch d);
    //! convert a MusicXML string to a numeric pitch
    static       pitch	xmlPitch (const std::string str);

    protected:
        THarmonic() : TTechnical(harmonic), fHarmonic(undefined), fPitch(undefined_pitch) {}
        virtual ~THarmonic() {}
	private:
		harmonict	fHarmonic;
		pitch		fPitch;
	static bimap<std::string, harmonict> fHarmonic2String;
	static harmonict fHarmonicTbl[];
	static std::string 	fHarmonicStrings[];        
	static bimap<std::string, pitch> fPitch2String;
	static pitch 	fPitchTbl[];
	static std::string 	fPitchStrings[];        
};
typedef SMARTP<THarmonic> SHarmonic;


/*! the \e fingering element.

	Fingering is typically indicated 1,2,3,4,5. Multiple
	fingerings may be given, typically to substitute
	fingerings in the middle of a note. The substitution
	and alternate values are "no" if the attribute is 
	not present.
*/
class EXP TFingering : public TTechnical, public Fontable {
    public:
        EXP friend SMARTP<TFingering> newFingering();
        
		virtual void accept(TScoreVisitor& visitor);

        void	setSubstitution(YesNo::type sub);
        void	setAlternate(YesNo::type sub);
        void 	setData(const std::string& data);

        YesNo::type	getSubstitution() const { return fSubstitution; }
        YesNo::type	getAlternate() const	{ return fAlternate; }
        const std::string	getData() const	{ return fData; }
 
    protected:
        TFingering() : TTechnical(TTechnical::fingering) {}
        virtual ~TFingering() {}
    private:
		YesNo::type fSubstitution;
		YesNo::type fAlternate;
		std::string fData;
};
typedef SMARTP<TFingering> SFingering;


/*!
\brief MusicXML \e hammer-on and \e pull-off elements.

	The hammer-on and pull-off elements are used in
	guitar and fretted instrument notation. Since
	a single slur can be marked over many notes,
	the hammer-on and pull-off elements are 
	separate so the individual pair of notes
	can be specified. The element content can be
	used to specify how the hammer-on or pull-off
	should be notated. An empty element leaves
	this choice to the program defaults.
*/
class EXP THammerPull : public TTechnical, public Fontable {
    public:
		enum { undefined=-1 };
        EXP friend SMARTP<THammerPull> newHammerPull(bool hammer=true);
        virtual void accept(TScoreVisitor& visitor);

		//! differentiate between hammer-on (true) and pull-off (false)
		void			setData(const std::string& data) { fData = data; }
		void			setStartStop(StartStop::type t) { fSType = t; }
		void			setNumber(int n)				{ fNumber = n; }

		const std::string&   getData() const			{ return fData; }
		StartStop::type getStartStop() const			{ return fSType; }
		int				getNumber() const				{ return fNumber; }
		
    protected:
        THammerPull(StartStop::type t, bool h=true) 
			: TTechnical(h ? hammer_on : pull_off), fSType(t), fNumber(undefined) {}
        virtual ~THammerPull() {}
	private:
		StartStop::type fSType;
		int		fNumber;
		std::string  fData;
};
typedef SMARTP<THammerPull> SHammerPull;

/*!
\brief MusicXML with-bar (from bend) element.
*/
class EXP TBendWithBar : public Positionable, public Placementable, public smartable, public visitable {
    public:
        EXP friend SMARTP<TBendWithBar> newBendWithBar();
        virtual void accept(TScoreVisitor& visitor);
		void			setData(const std::string& data)		{ fData = data; }
		const std::string&   getData() const					{ return fData; }
    protected:
        TBendWithBar() {}
        virtual ~TBendWithBar() {}
	private:
		std::string  fData;
};
typedef SMARTP<TBendWithBar> SBendWithBar;

/*!
\brief MusicXML bend element.

	The bend element is used in guitar and tablature.
	The bend-alter element indicates the number of
	steps in the bend, similar to the alter element.
	As with the alter element, numbers like 0.5 can
	be used to indicate microtones. Negative numbers
	indicate pre-bends or releases; the pre-bend and
	release elements are used to distinguish what
	is intended. A with-bar element indicates that
	the bend is to be done at the bridge with a
	whammy or vibrato bar. The content of the
	element indicates how this should be notated.
*/
class EXP TBend : public TTechnical, public Bendable {
    public:
        EXP friend SMARTP<TBend> newBend();
        virtual void accept(TScoreVisitor& visitor);
		void		setAlter(float alter)		{ fBendAlter = alter; }
		void		setPreBend(bool v)			{ fPreBend = v; }
		void		setRelease(bool v)			{ fRelease = v; }
		void		setWithBar(SBendWithBar wb) { fWithBar = wb; }

		float		getAlter() const			{ return fBendAlter; }
		bool		getPreBend() const			{ return fPreBend; }
		bool		getRelease() const			{ return fRelease; }
		SBendWithBar getWithBar()				{ return fWithBar; }
    protected:
        TBend() : TTechnical(TTechnical::bend),fPreBend(false),fRelease(false) {}
        virtual ~TBend() {}
	private:
		float			fBendAlter;
		bool			fPreBend, fRelease;
		SBendWithBar	fWithBar;
};
typedef SMARTP<TBend> SBend;

/*!
\brief MusicXML \e technical element

	Technical indications give performance information for
	individual instruments. The current elements are
	weighted towards keyboard and string instruments.

	The technical element is a list of basic
	technical annotations.
*/
class EXP TTechnicals : public TNotationElement {
    public:
        EXP friend SMARTP<TTechnicals> newTechnicals();
        virtual void accept(TScoreVisitor& visitor);

		void				add(const STechnical& elt);
		vvector<STechnical>  technicals()				{ return fTechnicals; }

    protected:
        TTechnicals() {}
        virtual ~TTechnicals() {}
	private:
		vvector<STechnical>  fTechnicals;
};
typedef SMARTP<TTechnicals> STechnicals;

} // namespace MusicXML


#endif
