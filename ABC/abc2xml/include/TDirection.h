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

#ifndef __TDirection__
#define __TDirection__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TSound.h"

using namespace std;

namespace MusicXML 
{

/*! The MusicXML \e octave-shift element.

	Octave shifts indicate where notes are shifted up or down
	from their true pitched values because of printing
	difficulty. Thus a treble clef line noted with 8va will
	be indicated with an octave-shift down from the pitch
	data indicated in the notes. A size of 8 indicates one
	octave; a size of 15 indicates two octaves.
\n	Defined in direction.dtd.
*/
class EXP TOctaveShift : public TDirectionTypeElement, public Positionable {

    public:
        enum { undefined=-1, up=1, down, stop, last=stop };
        EXP friend SMARTP<TOctaveShift> newOctaveShift(int type = TOctaveShift::undefined);
        
		virtual void accept(TScoreVisitor& visitor);

        void	setType(int type);
        void 	setNumber(int num);
        void 	setSize(int size);

        int 	getType() const		{ return fType; }
        int 	getNumber() const	{ return fNumber; }
        int 	getSize() const		{ return fSize; }

	//! convert an numeric type value to a MusicXML string
	static const string	xmltype (int d);
	//! convert an MusicXML string to a numeric type value
	static        int	xmltype (const string str);
 
    protected:
        TOctaveShift(int type) : fType(type), fNumber(undefined), fSize(8) {}
        virtual ~TOctaveShift() {}
    private:
        int 		fType;
        int 		fNumber;
        int 		fSize;

	static bimap<string, int> fType2String;
	static int 		fTypeTbl[];
	static string 	fTypeStrings[];        
};
typedef SMARTP<TOctaveShift> SOctaveShift;

/*!
\brief The MusicXML \e bracket element.

	Brackets are combined with words in a variety of
	modern directions. The line-end attribute specifies
	if there is a jog up or down (or both), an arrow,
	or nothing at the start or end of the bracket.
	The line-type is solid by default.
\n
    The MusicXML \e bracket element is defined in direction.dtd.
*/
class EXP TBracket : public TDirectionTypeElement, public Positionable {

    public:
        enum { undefined=-1, up=1, down, both, arrow, none, last=none };
		EXP friend SMARTP<TBracket> newBracket(StartStop::type t = StartStop::undefined);
        
		virtual void accept(TScoreVisitor& visitor);

        void 	setType(StartStop::type type);
        void 	setNumber(int num);
        void 	setLineEnd(int lend);
        void	setLineType(LineType::type t);

        StartStop::type getType() const		{ return fType; }
        int 	getNumber() const			{ return fNumber; }
        int 	getLineEnd() const			{ return fLineEnd; }
        LineType::type	getLineType() const	{ return fLineType; }

	//! convert an numeric line end value to a MusicXML string
	static const string	xmllineend (int d);
	//! convert an MusicXML string to a numeric line end value
	static        int	xmllineend (const string str);
 
    protected:
        TBracket(StartStop::type t) 
            : fType(t), fNumber(undefined), fLineEnd(undefined), fLineType(LineType::undefined) {}
        virtual ~TBracket() {}

    private:
        StartStop::type fType;
        int 			fNumber;
        int 			fLineEnd;
        LineType::type	fLineType;

	static bimap<string, int> fLEnd2String;
	static int 		fLEndTbl[];
	static string 	fLEndStrings[];        
};
typedef SMARTP<TBracket> SBracket;

/*!
\brief Dashes, used for instance with cresc. and dim. marks.

    The MusicXML \e dashes element is defined in direction.dtd.
*/
class EXP TDashes : public TDirectionTypeElement, public Positionable {

    public:
        enum { undefined=-1 };
		EXP friend SMARTP<TDashes> newDashes(StartStop::type t = StartStop::undefined);

		virtual void accept(TScoreVisitor& visitor);

        void 			setType(StartStop::type t);
        void 			setNumber(int num);
        StartStop::type getType() const				{ return fType; }
        int 			getNumber() const			{ return fNumber; }
 
    protected:
        TDashes(StartStop::type t) : fType(t), fNumber(undefined) {}
        virtual ~TDashes() {}
    private:
        StartStop::type fType;
        int 			fNumber;
};
typedef SMARTP<TDashes> SDashes;

/*!
\brief The MusicXml \e words element.

	Left justification is assumed if not specified. 
	Language is Italian ("it") by default.
    Defined in direction.dtd.
*/
class EXP TWords : public TDirectionTypeElement, public Positionable, public Fontable {

    public:
        enum { undefined=-1, left=1, center, right, last=right };
        EXP friend SMARTP<TWords> newWords();
        
		virtual void accept(TScoreVisitor& visitor);

        void 		setJustify (int just);
        void		setLang (const string& lang);
        void		setValue(const string& value);

        int 		getJustify() const		{ return fJustify; }
        const string&	getLang() const		{ return fLang; }
        const string&	getValue() const	{ return fValue; }

	//! convert an numeric justification to a MusicXML string
	static const string	xmljustify (int d);
	//! convert an MusicXML string to a numeric justification
	static        int	xmljustify (const string str);

    protected:
        TWords() : fJustify(undefined) {}
        virtual ~TWords() {}

    private:
        int 		fJustify;
        string		fLang;
        string		fValue;

	static bimap<string, int> fJust2String;
	static int 		fJustTbl[];
	static string 	fJustStrings[];        
};
typedef SMARTP<TWords> SWords;

/*!
\brief the MusicXML \e wedge element.

	Wedge spread is measured in tenths of staff line space.
	The type is crescendo for the start of a wedge that is
	closed at the left side, and diminuendo for the start
	of a wedge that is closed on the right side. Spread 
	values at the start of a crescendo wedge or end of a
	diminuendo wedge are ignored.
\n	The MusicXML \e wedge element is defined in direction.dtd.
*/
class EXP TWedge : public TDirectionTypeElement, public Positionable {

    public:
        enum { undefined=-1, crescendo=1, diminuendo, stop, last=stop };

        EXP friend SMARTP<TWedge> newWedge(int type = TOctaveShift::undefined);
        
		virtual void accept(TScoreVisitor& visitor);

        void 	setType (int type);
        void	setNumber(int num);
        void	setSpread(int sp);

        int 	getType () const	{ return fType; }
        int		getNumber () const	{ return fNumber; }
        int		getSpread () const	{ return fSpread; }

	//! convert an numeric wedge type to a MusicXML string
	static const string	xmltype (int d);
	//! convert an MusicXML string to a numeric wedge type
	static        int	xmltype (const string str);

    protected:
        TWedge(int type) 
            : fType(type), fNumber(undefined), fSpread(undefined) {}
        virtual ~TWedge() {}
    private:
        int 	fType;
        int		fNumber;
        int		fSpread;

	static bimap<string, int> fType2String;
	static int 		fTypeTbl[];
	static string 	fTypeStrings[];        
};
typedef SMARTP<TWedge> SWedge;

/*!
\brief the base class for symbolic note duration
*/
class EXP TSymbolicNoteDuration : public virtual smartable {

    public:
        EXP friend SMARTP<TSymbolicNoteDuration> newSymbolicNoteDuration ();

        void	setBeat (NoteType::type beatUnit);
        void	setDots (unsigned dots);
        NoteType::type	getBeat () const 	{ return fBeatUnit; }
        unsigned 		getDots () const	{ return fDots; }

    protected:
        TSymbolicNoteDuration () : fBeatUnit(NoteType::undefined), fDots(0) {}
        virtual ~TSymbolicNoteDuration() {}

    private:
        NoteType::type 	fBeatUnit;
        unsigned		fDots;
};
typedef SMARTP<TSymbolicNoteDuration> SSymbolicNoteDuration;

/*!
\brief Standard metronome marks.

	The beat-unit values are the
	same as for a type element, and the beat-unit-dot works
	like the dot element. The per-minute element can
	be a number, or a text description including numbers.
	The parentheses attribute indicates whether or not
	to put the metronome mark in parentheses; its value
	is no if not specified.
\n	The MusicXML \e metronome element is defined in direction.dtd.
*/
class EXP TMetronome : public TDirectionTypeElement, public TSymbolicNoteDuration, public Positionable {

    public:
        EXP friend SMARTP<TMetronome> newMetronome();

		virtual void accept(TScoreVisitor& visitor);
        
        void			setParentheses (YesNo::type val);
        void			setPerMinute (string pm);
        void			setPerMinute (long pm);
        YesNo::type		getParentheses () const 			{ return fParentheses; }
        string			getPerMinute () const 				{ return fPerMinute; }

        /*! used to specify metronome marking such as \<symbolic duration = symbolic duration\> 
		   ("eight note = eight note" for example). When the metronome uses this symbolic form,
			the PerMinute value is ignored.
			To cancel the symbolic form, set equal()=0
		*/
		SSymbolicNoteDuration& equal()						{ return fEqual; }

    protected:
        TMetronome() : fParentheses(YesNo::undefined) {}
        virtual ~TMetronome() {}

    private:
        YesNo::type 	fParentheses;
        string 			fPerMinute;
        SSymbolicNoteDuration 	fEqual;
};
typedef SMARTP<TMetronome> SMetronome;

/*!
\brief Represents textual direction indications.

	Textual direction types may have more than 1 component
	due to multiple font numbers.
\n
    TDirectionType corresponds to the MusicXML \e direciton-type
    element as defined in direction.dtd.    
*/
class EXP TDirectionType : public visitable, public smartable {

    public:
        EXP friend SMARTP<TDirectionType> newDirectionType();

		virtual void accept(TScoreVisitor& visitor);
        long  add (const SDirectionTypeElement& elt);
		vvector<SDirectionTypeElement>& elements()		{ return fElements; }

    protected:
        TDirectionType() {}
        virtual ~TDirectionType() {}
    private:
        vvector<SDirectionTypeElement>	fElements;
};
typedef SMARTP<TDirectionType> SDirectionType;

/*!
\brief The MusicXML \e direction element.

	A direction is a musical indication that is not attached
	to a specific note. Two or more may be combined to
	indicate starts and stops of wedges, dashes, etc.
\n	Defined in direction.dtd.
*/
class EXP TDirection : public TMusicData, public Placementable, public Editorialable {

    public:
        //! an undefined constant, used for offset, voice and staff
        enum { undefined = -1 };
        
        EXP friend SMARTP<TDirection> newDirection();
        
		virtual void accept(TScoreVisitor& visitor);

        long add(const SDirectionType& dir);
        void setOffset(long offset);
        void setVoice(long voice);
        void setStaff(long staff);

        SSound& sound() 							{ return fSound; }

        long getOffset() const				{ return fOffset; }
        long getVoice() const				{ return fVoice; }
        long getStaff() const				{ return fStaff; }
       	vvector<SDirectionType>& types() 	{ return fTypeList; }

    protected:
        TDirection()
            : fOffset(0), fVoice(undefined), fStaff(undefined) {}
        virtual ~TDirection() {}

    private:
        //! at least one DirectionType is required
        vvector <SDirectionType> fTypeList;
        //! optional offset
        long 		fOffset;
        //! optional voice
        long 		fVoice;
        //! optional staff
        long 		fStaff;
        //! optional sound
        SSound		fSound;
};
typedef SMARTP<TDirection> SDirection;

} // namespace MusicXML


#endif
