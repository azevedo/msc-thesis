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

#ifndef __TScore__
#define __TScore__

#include <string>

#include "exports.h"
#include "common.h"
#include "TScoreHeader.h"
#include "TScoreVisitor.h"
#include "smartpointer.h"
#include "xml.h"

namespace MusicXML 
{

/*!
\brief a MusicXML score.

	Base class for the partwise and timewise MusicXML formats.
*/
class EXP TScore : public visitable, public smartable {
    public:
        Sxmlheader&			xmlheader()		{ return fHeader; }
		SScoreHeader& 		scoreHeader()	{ return fScoreHeader; }

    protected:
        TScore () {}
        virtual ~TScore() {}

        Sxmlheader		fHeader;
		SScoreHeader 	fScoreHeader;
};
typedef SMARTP<TScore> SScore;

/*!
\brief the \p feature element (used by the \p grouping element).
*/
class EXP TFeature : public visitable, public smartable {
    public:
        EXP friend SMARTP<TFeature> newFeature();

	virtual void	accept(TScoreVisitor& visitor);
			void	setType (const string& type);
			void	setDate (const string& data);
	const string&   getType () const		{ return fType; }
	const string&   getData () const		{ return fData; }

    protected:
        TFeature () {}
        virtual ~TFeature() {}
	private:
		string  fType;
		string  fData;
};
typedef SMARTP<TFeature> SFeature;

/*!
\brief the \p grouping element.

	The grouping element is used for musical analysis. When
	the element type is "start", it usually contains one or
	more feature elements. The number attribute is used for
	distinguishing between overlapping and hierarchical
	groupings. The member-of attribute allows for easy
	distinguishing of what grouping elements are in what
	hierarchy. Feature elements contained within a "stop"
	type of grouping may be ignored.
*/
class EXP TGrouping : public TMusicData {
    public:
        enum { undefined=-1 };
		EXP friend SMARTP<TGrouping> newGrouping(StartStop::type type);

	virtual void	accept(TScoreVisitor& visitor);
			void	setType (StartStop::type type);
			void	setNumber (long number);
			void	setMemberOf (const string& data);
			void	add (const SFeature& feature);

	const StartStop::type   getType () const		{ return fType; }
	const string&			getMemberOf () const	{ return fMemberOf; }
			long			getNumber () const		{ return fNumber; }
	vvector<SFeature>&		getFeatures()			{ return fFeatures; }

    protected:
        TGrouping (StartStop::type type) {}
        virtual ~TGrouping() {}
	private:
		StartStop::type fType;
		long		fNumber;
		string		fMemberOf;
		vvector<SFeature>  fFeatures;
};
typedef SMARTP<TGrouping> SGrouping;

}

#endif
