/*

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

#ifndef __guido__
#define __guido__

#include "exports.h"
#include "smartpointer.h"
#include "guidovisitor.h"

#include <vector>
#include <iosfwd>
#include <sstream>
#include <string>

using namespace std;

class guidovisitor;
class guidoelement;
class guidoparam;
typedef SMARTP<guidoelement> 	Sguidoelement;
typedef SMARTP<guidoparam> 		Sguidoparam;

VEXP std::ostream& operator<< (std::ostream& os, const Sguidoelement& elt);

/*!
\addtogroup guido
@{
*/

/*!
\brief A guidotag parameter representation.

	A parameter is represented by its value.
*/
class VEXP guidoparam : public smartable, public guidovisitable {
	public:
        VEXP friend SMARTP<guidoparam> new_guidoparam(string value, bool quote=true);
        VEXP friend SMARTP<guidoparam> new_guidoparam(long value, bool quote=true);

		//! the parameter value
		void set (string value, bool quote=true);
		void set (long value, bool quote=true);
		string get () const 						{ return fValue; }
		bool   quote () const 						{ return fQuote; }
        
        void accept(guidovisitor& visitor);

    protected:
		guidoparam(string value, bool quote);
		guidoparam(long value, bool quote);
		virtual ~guidoparam ();
        
    private:
		string 	fValue;
		bool	fQuote;
};

/*!
\brief A generic guido element representation.

	An element is represented by its name and the
	list of its enclosed elements plus optional
    parameters.
*/
class VEXP guidoelement : public smartable, public guidovisitable {
	public:
        VEXP friend SMARTP<guidoelement> new_guidoelement(string name, string sep=" ");
		
		long add (Sguidoelement& elt);
		long add (Sguidoparam& param);
#ifndef WIN32
		long add (Sguidoparam param);
#endif
		void print (std::ostream& os);

		//! the element name
		void 	setName (string name)			{ fName = name; }
		string 	getName () const				{ return fName; }
		string 	getStart () const				{ return fStartList; }
		string 	getEnd () const					{ return fEndList; }
		string 	getSep () const					{ return fSep; }
        const vector<Sguidoelement>& elements() const 	{ return fElements; }
        const vector<Sguidoparam>& parameters() const 	{ return fParams; }
		
		bool empty () const { return fElements.empty(); }

        virtual void accept(guidovisitor& visitor);

    protected:
		guidoelement(string name, string sep=" ");
		virtual ~guidoelement();

		string	fName;
		//! the contained element start marker (default to empty)
		string	fStartList;
		//! the contained element end marker (default to empty)
		string	fEndList;
		//! the element separator (default to space)
		string	fSep;
		//! list of the enclosed elements
		vector<Sguidoelement>	fElements;
		//! list of optional parameters
		vector<Sguidoparam>	fParams;		
};

/*!
\brief A guido note duration representation.

	A note duration is represented by a numerator 
    (denotes the number of beats), a denominator (denotes the beat value)
     and optional dots.
     Triplets are repesented as 1/3, 1/6, ... quintuplets, septuplets and so on
     are handled analogously.
*/
class VEXP guidonoteduration {
	public:
		guidonoteduration(long num, long denom, long dots=0) 
            { set (num, denom, dots); }
		virtual ~guidonoteduration() {}
        
        void set (long num, long denom, long dots=0) 
            { fNum=num; fDenom=denom; fDots=dots; }
        guidonoteduration& operator= (const guidonoteduration& dur)	
            { fNum=dur.fNum; fDenom=dur.fDenom; fDots=dur.fDots; return *this; }
        bool operator!= (const guidonoteduration& dur) const	
            { return (fNum!=dur.fNum) || (fDenom!=dur.fDenom) || (fDots!=dur.fDots); }

        long	fNum;
		long	fDenom;
		long	fDots;
};

/*!
\brief A guido note representation.

	A note is represented by its name, optional accidentals,
    duration (in the form of numerator/denominator) and optional dots.
*/
class VEXP guidonote : public guidoelement {
	public:
        VEXP friend SMARTP<guidonote> new_guidonote(unsigned short voice, string name, char octave,
                                                guidonoteduration& dur, string acc="");
		
		void set (unsigned short voice, string name, char octave, 
                    guidonoteduration& dur, string acc);

        void accept(guidovisitor& visitor);

		const char * 	name() const		{ return fNote.c_str(); }
		const char * 	accidental() const	{ return fAccidental.c_str(); }
		char 			octave() const		{ return fOctave; }
		const guidonoteduration& duration() const { return fDuration; }

	protected:
		guidonote(unsigned short voice, string name, char octave, 
                    guidonoteduration& dur, string acc="");
		virtual ~guidonote();
	
	string 	fNote;
	string 	fAccidental;
	char 	fOctave;
	guidonoteduration fDuration;

};
typedef SMARTP<guidonote> Sguidonote;

/*!
\brief Represents the current status of notes duration and octave.

    Octave and duration may be ommitted for guido notes. If so,
    they are infered from preceeding notes (or rest), within the same 
    sequence or chord, or assumed to have standard values.
\n
	The object is defined as a multi-voices singleton: a single
    object is allocated for a specific voice and thus it will
	not operate correctly on a same voice parrallel formatting 
    operations.

\todo handling the current beat value for \e *num duration form.
*/
class VEXP guidonotestatus {
	public:
        enum { kMaxInstances=128 };
        
		static guidonotestatus* get(unsigned short voice);
		static void resetall();

        enum { defoctave=1, defnum=1, defdenom=4 };
        
        void reset()	{ fOctave=defoctave; fDur.set(defnum, defdenom, 0); }
        guidonotestatus& operator= (const guidonoteduration& dur)	{ fDur = dur; return *this; }
        bool operator!= (const guidonoteduration& dur) const		{ return fDur!= dur; }
            
		char				fOctave;
		guidonoteduration 	fDur;
//		char				fBeat;

	protected:
		guidonotestatus() :	fOctave(defoctave), fDur(defnum, defdenom, 0) {}
	private:
		static guidonotestatus * fInstances[kMaxInstances];
};

/*!
\brief The guido sequence element
*/
class VEXP guidoseq : public guidoelement {
	public:
        VEXP friend SMARTP<guidoseq> new_guidoseq();
        void accept(guidovisitor& visitor);
	protected:
		guidoseq();
		virtual ~guidoseq();
};
typedef SMARTP<guidoseq> Sguidoseq;

/*!
\brief The guido chord element
*/
class VEXP guidochord : public guidoelement {
	public:
        VEXP friend SMARTP<guidochord> new_guidochord();
        void accept(guidovisitor& visitor);
	protected:
		guidochord ();
		virtual ~guidochord();
};
typedef SMARTP<guidochord> Sguidochord;

/*!
\brief A guido tag representation.

	A tag is represented by its name and optional parameters.
	A range tag contains enclosed elements.
*/
class VEXP guidotag : public guidoelement {
	public:
        VEXP friend SMARTP<guidotag> new_guidotag(string name);
        void accept(guidovisitor& visitor);
	protected:
		guidotag(string name);
		virtual ~guidotag();
};
typedef SMARTP<guidotag> Sguidotag;
/*! @} */


#endif
