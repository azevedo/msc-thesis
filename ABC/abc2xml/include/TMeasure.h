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

#ifndef __TMeasure__
#define __TMeasure__

#include "exports.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include "common.h"

using namespace std;

namespace MusicXML 
{

/*!
\brief The MusicXML base class for partwise and timewise \e measure elements.

	Measures have a required measure number attribute.

	The \e implicit attribute is set to "yes" for measures where
	the measure number should never appear, such as pickup
	measures and the last half of mid-measure repeats. The
	implicit attribute is "no" if not specified.
	
	The \e non-controlling attribute indicates that this measure
	in this part does not necessarily synchronize with other
	measures in other parts. 
\n
	The MusicXML \e measure element is defined in score.dtd.

\todo support of the \e implicit and \e non-controlling attributes.
*/
class TMeasure : public visitable, public smartable {
    public:
        void	setStringNumber(const string& num);
        void	setImplicit( YesNo::type type);
        void	setNonControlling( YesNo::type type);

        const string&	getStringNumber() const	{ return fNum; }
        long	getNumber() const	{ return atoi(fNum.c_str()); }
        YesNo::type	getImplicit() const	{ return fImplicit; }
        YesNo::type	getNonControlling() const	{ return fNonControling; }
   
    protected:
        TMeasure(long num);
        TMeasure(string num) : fNum(num),fImplicit(YesNo::undefined),fNonControling(YesNo::undefined) {}
        virtual ~TMeasure() {}
    private:
        //! the required measure number attribute
		string 	fNum;
        YesNo::type fImplicit;
        YesNo::type fNonControling;
};

} // namespace MusicXML


#endif
