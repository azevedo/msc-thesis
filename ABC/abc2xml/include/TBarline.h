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

#ifndef __TBarline__
#define __TBarline__

#include "exports.h"
#include "TScoreVisitor.h"
#include "TOrnaments.h"
#include "common.h"
#include <string>

namespace MusicXML 
{

/*!
\brief Endings refers to multiple (e.g. first and second) endings.

    Typically, the start type is associated with
	the left barline of the first measure in an ending. The
	stop and discontinue types are associated with the right
	barline of the last measure in an ending. Stop is used
	when the ending mark concludes with a downward jog, as
	is the case for first endings. Discontinue is used when
	there is no downward jog, as in the final ending.
\n
    The MusicXML \e ending element is defined in barline.dtd.
\todo support of multiple numbers
*/

class EXP TEnding : public visitable, public smartable  {
    public:
        enum { start=1, stop, discontinue, last=discontinue };
        EXP friend SMARTP<TEnding> newEnding(long num, int type);

		virtual void accept(TScoreVisitor& visitor);
        
        void setNum (long num);
        void setType(int type);
        long getNum () const		{ return fNum; }
        int  getType() const		{ return fType; }

	//! convert an integer ending type to a MusicXML string
	static const string	xmltype (int d);
	//! convert an MusicXML string to an integer ending type
    static int			xmltype (const string str);

    protected:
        TEnding(long num, int type) : fNum(num), fType(type) {}
        virtual ~TEnding() {}

    private:
        //! a required number
        long 	fNum;
        //! a required type
        int 	fType;

	static bimap<string, int> fType2String;
	static int 		fTypeTbl[];
	static string 	fTypeStrings[];
};
typedef SMARTP<TEnding> SEnding;

/*!
\brief Represents repeat marks.

	The start of the repeat has a forward
	direction while the end of the repeat has a backward
	direction. Backward repeats that are not part of an
	ending can use the times attribute to indicate the
	number of times the repeated section is played.
\n
    The MusicXML \e repear element is defined in barline.dtd.
*/
class EXP TRepeat : public visitable, public smartable {
    public:
        enum { undefined = 0, backward, forward, last=forward };
        EXP friend SMARTP<TRepeat> newRepeat(int direction);
       
		virtual void accept(TScoreVisitor& visitor);

        int getDirection() const	{ return fDirection; }
        int getTimes() const		{ return fTimes; }
        void setDirection(int dir);
        void setTimes(int times);

	//! convert an integer direction to a MusicXML string
	static const string	xmldirection (int d);
	//! convert an MusicXML string to an integer direction
    static int			xmldirection (const string str);

    protected:
        TRepeat(int direction) : fDirection(direction), fTimes(undefined) {}
        virtual ~TRepeat() {}

    private:
        int fDirection;
        int fTimes;

	static bimap<string, int> fDirection2String;
	static int 		fDirectionTbl[];
	static string 	fDirectionStrings[];
};
typedef SMARTP<TRepeat> SRepeat;

/*!
\brief Represents a MusicXML barline element.
 
	If a barline is other than a normal single barline, it
	should be represented by a barline element that describes
	it. This includes information about repeats and multiple
	endings, as well as line style. 
    The two fermata elements allow for fermatas
	on both sides of the barline (the lower one inverted).
\n
	Barlines have a location attribute.It must match where the barline 
    element occurs within the rest of the musical data in the score. 
    If location is left, it should be the first element in the measure; if
	location is right, it should be the last element. If no
	location is specified, the right barline is the default.
\n
    The MusicXML \e barline element is defined in barline.dtd.
*/

class EXP TBarline : public TMusicData, public Editorialable {
    
    public:
        enum { undefined=0, right=1, left, middle, lastloc= middle };
        enum { none=1, regular, dotted, heavy, light_light, 
                light_heavy, heavy_light, heavy_heavy, last=heavy_heavy };
 
        EXP friend SMARTP<TBarline> newBarline();
 
		virtual void accept(TScoreVisitor& visitor);

        void		setLocation(int loc);
        void		setBarStyle(int style);
        int			getLocation()	const 	{ return fLocation; }
        int			getBarStyle()	const 	{ return fBarStyle; }

        SWavyLine& 	wavyLine()	 	{ return fWavyLine; }
        SSegno&		segno()		 	{ return fSegno; }
        SCoda&		coda()		 	{ return fCoda; }
        SFermata&	fermata()		{ return fFermata; }
        SEnding&	ending()		{ return fEnding; }
        SRepeat&	repeat()		{ return fRepeat; }
       
	//! convert an integer location to a MusicXML string
	static const string	xmllocation (int d);
	//! convert an MusicXML string to an integer location
	static int			xmllocation (const string str);
	//! convert an integer bar style to a MusicXML string
	static const string	xmlstyle (int d);
	//! convert an MusicXML string to an integer bar style
	static int			xmlstyle (const string str);

    protected:
        TBarline() : fLocation(undefined), fBarStyle(undefined) {}
        virtual ~TBarline() {}

    private:
        //! the barline location attribute (default to right)
        int 		fLocation;
        //! optionnal barline style
        int 		fBarStyle;
        SWavyLine  	fWavyLine;
        SSegno 		fSegno;
        SCoda 		fCoda;
        SFermata 	fFermata;
        SEnding 	fEnding;
        SRepeat 	fRepeat;


	static bimap<string, int> fLocation2String;
	static int 		fLocationTbl[];
	static string 	fLocationStrings[];

	static bimap<string, int> fStyle2String;
	static int 		fStyleTbl[];
	static string 	fStyleStrings[];
};
typedef SMARTP<TBarline> SBarline;

} // namespace MusicXML

#endif
