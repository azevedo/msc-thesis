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

#ifndef __TPart__
#define __TPart__

#include "smartpointer.h"
#include "TScoreVisitor.h"
#include <string>

namespace MusicXML 
{

/*!
\brief The base class for the MusicXML timewise and partwise \e part elements.

	In either format (partwise or timewise), the \e part element has 
	a required id attribute that is an IDREF back to a score-part 
	in the part-list.
\n
	The MusicXML \e part element is defined in score.dtd.
*/
class TPart : public visitable, public smartable {
    public:
        void	setID(const std::string& id);
        const std::string&	getID()	const	{ return fID; }
    protected:
        TPart(const std::string& id) : fID(id) {}
        virtual ~TPart() {}
    private:
        //! the required id attribute
		std::string 	fID;
};

} // namespace MusicXML


#endif
