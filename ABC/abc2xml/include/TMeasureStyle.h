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

#ifndef __TMeasureStyle__
#define __TMeasureStyle__

#include "exports.h"

#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

namespace MusicXML
{

/*!
\brief base class for all the elements of the \e measure-style.
*/
class TMeasureStyleElement : public virtual visitable, public virtual smartable {
	public:
		virtual void accept(TScoreVisitor& visitor);
     protected:
        TMeasureStyleElement () {}
        virtual ~TMeasureStyleElement() {}
};
typedef SMARTP<TMeasureStyleElement> 	SMeasureStyleElement;

/*!
\brief Represents multiple rests (for measure style).

	The text of the multiple-rest element indicates the
	number of measures in the multiple rest. Multiple
	rests may use the 1-bar / 2-bar / 4-bar rest
	symbols, or a single shape. The use-symbols
	attribute indicates which to use; it is no
	if not specified.
*/
class EXP TMultipleRest : public TMeasureStyleElement {

    public:
		enum { undefined=-1 };
        EXP friend SMARTP<TMultipleRest> newMultipleRest();
		virtual void accept(TScoreVisitor& visitor);

		void			setData(const string& data)		{ fData = data; }
		void			setUseSymbol(YesNo::type t)		{ fUseSymbol = t; }
		YesNo::type		getUseSymbol() const            { return fUseSymbol; }
		const string&   getData() const					{ return fData; }

    protected:
        TMultipleRest() : fUseSymbol(YesNo::undefined) {}
        virtual ~TMultipleRest() {};
    private:
		YesNo::type fUseSymbol;
		string		fData;
};
typedef SMARTP<TMultipleRest> SMultipleRest;

/*!
\brief Represents measure repeat (for measure style).

	The measure-repeat element is used for both
	single and multiple measure repeats. The text
	of the element indicates the number of measures
	to be repeated in a single pattern. The slashes
	attribute specifies the number of slashes to
	use in the repeat sign. It is 1 if not specified.
	Both the start and the stop of the measure-repeat
	must be specified.
*/
class EXP TMeasureRepeat : public TMeasureStyleElement {

    public:
		enum { undefined=-1 };
        EXP friend SMARTP<TMeasureRepeat> newMeasureRepeat(StartStop::type type=StartStop::undefined);
		virtual void accept(TScoreVisitor& visitor);

		void			setData(const string& data)		{ fData = data; }
		void			setSlashes(int n)				{ fSlashes = n; }
		void			setStartStop(StartStop::type t)	{ fStartStop = t; }
		int				getSlashes() const				{ return fSlashes; }
		const string&   getData() const					{ return fData; }
		StartStop::type getStartStop() const            { return fStartStop; }

    protected:
        TMeasureRepeat(StartStop::type t) : fStartStop(t), fSlashes(undefined) {}
        virtual ~TMeasureRepeat() {};
    private:
		StartStop::type fStartStop;
		int		fSlashes;
		string  fData;
};
typedef SMARTP<TMeasureRepeat> SMeasureRepeat;

/*!
\brief Represents beat repeat (for measure style).

	The beat-repeat element is used to indicate
	that a single beat (but possibly many notes)
	is repeated. Both the start and stop of the
	beat being repeated should be specified. The
	slashes attribute specifies the number of
	slashes to use in the symbol. The use-dots
	attribute indicates whether or not to use
	dots as well (for instance, with mixed
	rhythm patterns). By default, the value for
	slashes is 1 and the value for use-dots
	is no.
*/
class EXP TBeatRepeat : public TMeasureStyleElement {

    public:
		enum { undefined=-1 };
        EXP friend SMARTP<TBeatRepeat> newBeatRepeat(StartStop::type type=StartStop::undefined);
		virtual void accept(TScoreVisitor& visitor);

		void			setUseDots(YesNo::type t)		{ fUseDots = t; }
		void			setSlashes(int n)				{ fSlashes = n; }
		void			setStartStop(StartStop::type t)	{ fStartStop = t; }
		int				getSlashes() const				{ return fSlashes; }
		YesNo::type		getUseDots() const				{ return fUseDots; }
		StartStop::type getStartStop() const            { return fStartStop; }

    protected:
        TBeatRepeat(StartStop::type t) : fStartStop(t), fSlashes(undefined), fUseDots(YesNo::undefined) {}
        virtual ~TBeatRepeat() {};
    private:
		StartStop::type fStartStop;
		int				fSlashes;
		YesNo::type		fUseDots;
};
typedef SMARTP<TBeatRepeat> SBeatRepeat;

/*!
\brief the slash element (for measure style).

	The slash element is used to indicate that
	slash notation is to be used. If the slash is
	on every beat, use-stems is no (the default). 
	To indicate rhythms but not pitches, use-stems
	is set to yes. The type attribute indicates
	whether this is the start or stop of a slash 
	notation style. The use-dots attribute works 
	as for the beat-repeat element, and only 
	has effect if use-stems is no.
*/
class EXP TSlash : public TMeasureStyleElement {

    public:
        EXP friend SMARTP<TSlash> newSlash(StartStop::type type=StartStop::undefined);
		virtual void accept(TScoreVisitor& visitor);

		void			setUseDots(YesNo::type t)		{ fUseDots = t; }
		void			setUseStems(YesNo::type t)		{ fUseStems = t; }
		void			setStartStop(StartStop::type t)	{ fStartStop = t; }
		YesNo::type		getUseDots() const				{ return fUseDots; }
		YesNo::type		getUseStems() const				{ return fUseStems; }
		StartStop::type getStartStop() const            { return fStartStop; }

    protected:
        TSlash(StartStop::type t) : fStartStop(t), fUseDots(YesNo::undefined), fUseStems(YesNo::undefined) {}
        virtual ~TSlash() {};
    private:
		StartStop::type fStartStop;
		YesNo::type		fUseDots;
		YesNo::type		fUseStems;
};
typedef SMARTP<TSlash> SSlash;

/*!
\brief Represents measure style.

	A measure-style indicates a special way to print
	partial to multiple measures within a part. This 
	includes multiple rests over several measures,
	repeats of betas, single, or multiple measures,
	and use of slash notation.
	
	The multiple-rest and measure-repeat symbols
	indicate the number of measures covered in the
	element content. The beat-repeat and slash 
	elements can cover partial measures. All but the
	multiple-rest element use a type attribute to 
	indicate starting and stopping the use of the
	style. The optional number attribute specifies 
	the staff number from top to bottom on the system,
	as with clef.

	The measure-repeat and beat-repeat element specify
	a notation style for repetitions. The actual music
	being repeated needs to be repeated within the
	MusicXML file. These elements indicate to use the
	appropriate notation to indicate the repeat. 
*/
class EXP TMeasureStyle : public visitable, public smartable {

    public:
		enum { undefined=-1 };
        EXP friend SMARTP<TMeasureStyle> newMeasureStyle();
		virtual void accept(TScoreVisitor& visitor);
		
		void					setNumber(int num) { fNumber = num; }
		void					setStyle(SMeasureStyleElement& elt)		{ fStyle = elt; }
		SMeasureStyleElement&   getStyle()		{ return fStyle; }
		int						getNumber()		{ return fNumber; }

    protected:
        TMeasureStyle() : fNumber(undefined) {}
        virtual ~TMeasureStyle() {};
    private:
		int fNumber;
		SMeasureStyleElement	fStyle;
};
typedef SMARTP<TMeasureStyle> SMeasureStyle;


} // namespace MusicXML


#endif
