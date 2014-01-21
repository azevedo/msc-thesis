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

#ifndef __TOrnaments__
#define __TOrnaments__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

#include <string>

namespace MusicXML 
{

/*!
\brief an accidental-mark

	An accidental-mark can be used as a separate
	notation or as part of an ornament. When used in
	an ornament, position and placement are relative
	to the ornament, not relative to the note.
*/
class EXP TAccidentalMark : public Positionable, public Placementable, public TNotationElement {
    public:
        EXP friend SMARTP<TAccidentalMark> newAccidentalMark();
        virtual void accept(TScoreVisitor& visitor);
		void			setData(const string& data)		{ fData = data; }
		const string&   getData() const					{ return fData; }
    protected:
        TAccidentalMark() {}
        virtual ~TAccidentalMark() {}
	private:
		string  fData;
};
typedef SMARTP<TAccidentalMark> SAccidentalMark;

/*!
\brief basic ornament representation

	Ornaments can be any of several types, followed
	optionally by accidentals. The accidental-mark
	element's content is represented the same as an
	accidental element, but with a different name to
	reflect the different musical meaning.
	This base TOrnament class is intended to be derived but may be used
	as is if additional information such as trill-sound is not
	required.
*/
class EXP TOrnament : public Positionable, public Placementable, public smartable, public visitable {
    public:
        enum type { trill_mark=1, turn, delayed_turn, shake,
				wavy_line, mordent, inverted_mordent,
				schleifer, other_ornament, last=other_ornament };
        
        EXP friend SMARTP<TOrnament> newOrnament(type t);
        virtual void accept(TScoreVisitor& visitor);
        
        void 	setType(type t);
        type	getType() const			{ return fType; }

        void 	add(SAccidentalMark acc);
		vvector<SAccidentalMark>&   accidents()		{ return fAccidents; }

    //! convert a numeric ornament type to a MusicXML string
    static const string	xml (type d);
    //! convert a MusicXML string to a numeric ornament type
    static       type	xml (const string str);

    protected:
        TOrnament(type t) : fType(t) {}
        virtual ~TOrnament() {}
	private:
		type	fType;
		vvector<SAccidentalMark>	fAccidents;
	static bimap<string, type> fType2String;
	static type 	fTypeTbl[];
	static string 	fTypeStrings[];        
};
typedef SMARTP<TOrnament> SOrnament;

/*!
\brief other ornaments
*/
class EXP TOtherOrnament : public TOrnament {
    public:
        EXP friend SMARTP<TOtherOrnament> newOtherOrnament();
        virtual void accept(TScoreVisitor& visitor);
		void			setData(const string& data)		{ fData = data; }
		const string&   getData() const					{ return fData; }
    protected:
        TOtherOrnament() : TOrnament(other_ornament) {}
        virtual ~TOtherOrnament() {}
	private:
		string  fData;
};
typedef SMARTP<TOtherOrnament> SOtherOrnament;

/*!
\brief ornaments that carry a trill-sound entity

	Trill-mark, turn, delayed-turn, shake, mordent, inverted-mordent
	and wavy-line are ornaments that carry the trill-sound entity.
*/
class EXP TTrillOrnament : public TOrnament, public Trillable {
    public:
        EXP friend SMARTP<TTrillOrnament> newTrillOrnament(type t);
        virtual void accept(TScoreVisitor& visitor);
    protected:
        TTrillOrnament(type t) : TOrnament(t) {}
        virtual ~TTrillOrnament() {}
};
typedef SMARTP<TTrillOrnament> STrillOrnament;

/*!
\brief mordent and inverted-mordent ornaments

	The long attribute for the mordent and inverted-mordent
	elements is "no" by default. The mordent element 
	represents the sign with the vertical line; the
	inverted-mordent represent the sign without the
	vertical line.	
*/
class EXP TMordentOrnament : public TTrillOrnament {
    public:
        EXP friend SMARTP<TMordentOrnament> newMordentOrnament(type t);
		
        virtual void accept(TScoreVisitor& visitor);
		void	setLong(YesNo::type yn)		{ fLong = yn; }
		YesNo::type	getLong() const			{ return fLong; }
    protected:
        TMordentOrnament(type t) : TTrillOrnament(t), fLong(YesNo::undefined) {}
        virtual ~TMordentOrnament() {}
    private:
		YesNo::type	fLong;
};
typedef SMARTP<TMordentOrnament> SMordentOrnament;

/*!
\brief Represents a MusicXML \e wavy-line element

	wavy-line elements can be applied both to
	notes and to measures.
	Wavy lines are one way to indicate trills; when used with a
	measure element, they should always have type="continue" set.
\n 
    The MusicXML \e wavy-line element is defined in common.dtd
*/
class EXP TWavyLine : public TTrillOrnament {
    public:
        enum { undefined = -1 };
		EXP friend SMARTP<TWavyLine> newWavyLine(StartStop::type t = StartStop::undefined);
        
        virtual void accept(TScoreVisitor& visitor);
        void setType (StartStop::type t)		{ fType = t; }
        StartStop::type getType () const		{ return fType; }
        void setNumber (int num)	{ fNumber = num; }
        int getNumber () const		{ return fNumber; }

    protected:
        TWavyLine(StartStop::type type) : TTrillOrnament(wavy_line), fType(type), fNumber(undefined) {}
        virtual ~TWavyLine() {}
        
    private:
        //! the required type attribute
        StartStop::type fType;
        //! an optional number attribute
        int 		fNumber;
};
typedef SMARTP<TWavyLine> SWavyLine;

/*!
\brief basic ornament representation

	Ornaments can be any of several types, followed
	optionally by accidentals. The accidental-mark
	element's content is represented the same as an
	accidental element, but with a different name to
	reflect the different musical meaning.
*/
class EXP TOrnaments : public TNotationElement {
    public:
        EXP friend SMARTP<TOrnaments> newOrnaments();
        virtual void accept(TScoreVisitor& visitor);

		void				add(SOrnament ornament);
		vvector<SOrnament>  ornaments()				{ return fOrnaments; }
		SOrnament getLast() { return fOrnaments[fOrnaments.size() - 1]; }

    protected:
        TOrnaments() {}
        virtual ~TOrnaments() {}
	private:
		vvector<SOrnament>  fOrnaments;
};
typedef SMARTP<TOrnaments> SOrnaments;


} // namespace MusicXML


#endif
