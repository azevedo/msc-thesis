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

#ifndef __TPartGroup__
#define __TPartGroup__

#include "exports.h"
#include "common.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"

namespace MusicXML 
{

/*!
\brief Indicates groupings of parts in a score.
	
	Groupings of parts in the score are usually indicated by braces and brackets.
	The number attribute is used to distinguish overlapping
	and nested part-groups, not the sequence of groups. As
	with parts, groups can have a name and abbreviation.
	The group-symbol element indicates how the group is 
	indicated on the score. Values include none (default),
	brace, line, and bracket. Values for the child elements
	are ignored at the stop of a group.
\n	
	TPartGroup corresponds to the MusicXML \e part-group element 
	as defined in score.dtd.
*/
class EXP TPartGroup : public TPartListElement, public Editorialable {
    
    public:
        EXP friend SMARTP<TPartGroup> newPartGroup(StartStop::type type);
        //! the possible values for the group symbol
        enum symbol { undefined=-1, none=1, brace, line, bracket, last=bracket };

        virtual void accept(TScoreVisitor& visitor);
 
        //! the required type attribute
        void 	setType (StartStop::type type);
        //! the optional group number
        void 	setNumber(int num);
        //! optional group name
        void 	setGroupName(string name);
		//! optional name abbreviation
		void 	setGroupAbbrev(string abbrev);
		//! optional group barline
		void 	setGroupBarline(string barline);
        //! optional group symbol
        void 	setSymbol(symbol sym);

        StartStop::type getType() const		{ return fType; }
        int 		getNumber() const		{ return fNumber; }
        string 		getGroupName() const	{ return fGroupName; }
        string 		getGroupAbbrev() const	{ return fGroupAbbrev; }
        string 		getGroupBarline() const	{ return fGroupBarline; }
        symbol 		getSymbol() const		{ return fSymbol; }

	//! convert an integer dynamic to a MusicXML string
	static const string	xmlsymbol (symbol d);
	//! convert an MusicXML string to an integer dynamic
	static symbol xmlsymbol (const string str);
           
    protected:
        TPartGroup(StartStop::type type)
			: fType(type), fNumber(undefined), fSymbol(undefined) {}
        virtual ~TPartGroup() {}
    private:
        StartStop::type fType;
        int 		fNumber;
        string 		fGroupName;
        string 		fGroupAbbrev;
        string 		fGroupBarline;
        symbol 		fSymbol;

	static bimap<string, symbol> fSymbol2String;
	static symbol 		fSymbolTbl[];
	static string 		fSymbolStrings[];
};
typedef SMARTP<TPartGroup> SPartGroup;

} // namespace MusicXML


#endif
