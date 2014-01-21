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

#ifndef __TOtherDirections__
#define __TOtherDirections__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"

using namespace std;

namespace MusicXML 
{

/*! additional MusicXML direction elements.
*/
class EXP TOtherDirections : public TDirectionTypeElement, public Positionable {

    public:
        enum type { damp=1, damp_all, eyeglasses, last=eyeglasses };
        EXP friend SMARTP<TOtherDirections> newOtherDirections(type t);
        
		virtual void accept(TScoreVisitor& visitor);

        void	setType(type t)		{ fType = t;}
        type 	getType() const		{ return fType; }

	//! convert an numeric type value to a MusicXML string
	static const string	xml (type d);
	//! convert an MusicXML string to a numeric type value
	static        type	xml (const string str);
 
    protected:
        TOtherDirections(type t) : fType(t) {}
        virtual ~TOtherDirections() {}
    private:
		type	fType;
	static bimap<string, type> fType2String;
	static type		fTypeTbl[];
	static string 	fTypeStrings[];        
};
typedef SMARTP<TOtherDirections> SOtherDirections;

/*! MusicXML \e rehearsal element.
*/
class EXP TRehearsal : public TDirectionTypeElement, public Positionable, public Fontable {
    public:
        EXP friend SMARTP<TRehearsal> newRehearsal();        
		virtual void accept(TScoreVisitor& visitor);

		void			setData(const string& data)		{ fData = data; }
		const string&   getData() const					{ return fData; }
 
    protected:
        TRehearsal() {}
        virtual ~TRehearsal() {}
    private:
		string  fData;
};
typedef SMARTP<TRehearsal> SRehearsal;

/*! MusicXML \e other-direction element.

	The other-direction element is used to define
	any direction symbols not yet in the current version
	of MusicXML. This allows extended representation,
	though without application interoperability.

*/
class EXP TOtherDirection : public TDirectionTypeElement, public Positionable {
    public:
        EXP friend SMARTP<TOtherDirection> newOtherDirection();        
		virtual void accept(TScoreVisitor& visitor);
		void			setData(const string& data)		{ fData = data; }
		const string&   getData() const					{ return fData; } 
    protected:
        TOtherDirection() {}
        virtual ~TOtherDirection() {}
    private:
		string  fData;
};
typedef SMARTP<TOtherDirection> SOtherDirection;

/*! MusicXML \e pedal element.
*/
class EXP TPedal : public TDirectionTypeElement, public Positionable {
    public:
		EXP friend SMARTP<TPedal> newPedal(StartStop::type t = StartStop::undefined);        
		virtual void accept(TScoreVisitor& visitor);

		void			setType(StartStop::type st)	{ fType = st; }
		void			setLine(YesNo::type line)			{ fLine = line; }
		StartStop::type getType() const		{ return fType; }
		YesNo::type		getLine() const				{ return fLine; }
 
    protected:
        TPedal(StartStop::type t) : fType(t) {}
        virtual ~TPedal() {}
    private:
		StartStop::type fType;
		YesNo::type		fLine;
};
typedef SMARTP<TPedal> SPedal;

/*! MusicXML \e print element.

	The print element contains general printing parameters.
	
	Staff spacing between multiple staves is measured in
	tenths of staff lines (e.g. 100 = 10 staff lines).
	
	The new-system and new-page attributes indicate
	whether to force a system or page break, or to
	force the current music onto the same system or
	page as the preceding music. Normally this is the
	first music data within a measure. If used in
	multi-part music, they should be placed in the
	same positions within each part, or the results
	are undefined.

*/
class EXP TPrint : public TMusicData {
    public:
		enum { undefined=-1 };
        EXP friend SMARTP<TPrint> newPrint();        
		virtual void accept(TScoreVisitor& visitor);

		void			setStaffSpacing(int tenth)		{ fSpacing = tenth; }
		void			setNewSystem(YesNo::type ns)	{ fNewSystem = ns; }
		void			setNewPage(YesNo::type ns)		{ fNewPage = ns; }

		int				getStaffSpacing() const		{ return fSpacing; }
		YesNo::type		getNewSystem() const		{ return fNewSystem; }
		YesNo::type		getNewPage() const			{ return fNewPage; }
 
    protected:
        TPrint() : fSpacing(undefined), fNewSystem(YesNo::undefined), fNewPage(YesNo::undefined) {}
        virtual ~TPrint() {}
    private:
		int				fSpacing;
		YesNo::type		fNewSystem;
		YesNo::type		fNewPage;
};
typedef SMARTP<TPrint> SPrint;


} // namespace MusicXML


#endif
