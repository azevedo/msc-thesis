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

#ifndef __TTimeModification__
#define __TTimeModification__

#include "exports.h"
#include "smartpointer.h"
#include "TScoreVisitor.h"
#include "conversions.h"

namespace MusicXML 
{

/*!
\brief Represents tuplets and other durational changes.
 
	The actual-notes element describes
	how many notes are played in the time usually occupied
	by the number of normal-notes. If the normal-notes type
	is different than the current note type (e.g., a 
	quarter note within an eighth note triplet), then the
	normal-notes type (e.g. eighth) is specified in the
	normal-type and normal-dot elements.
\n
	TTimeModification corresponds to the MusicXML \e time-modification
    as defined in note.dtd.
*/
class EXP TTimeModification : public visitable, public smartable {
    
    public:
		enum { undefined = -1 };
        EXP friend SMARTP<TTimeModification> newTimeModification();

        virtual void accept(TScoreVisitor& visitor);

        void 	setActualNotes(long v);
        void	setNormalNotes(long v);
        void	setNormalType(NoteType::type v);
        void	setNormalDots(long v);

        long 	getActualNotes() const	{ return fActualNotes; }
        long	getNormalNotes() const	{ return fNormalNotes; }
        NoteType::type	getNormalType() const	{ return fNormalType; }
        long	getNormalDots() const	{ return fNormalDots; }

    protected:
		TTimeModification() 
            : fActualNotes(undefined), fNormalNotes(undefined), fNormalType(NoteType::undefined), fNormalDots(0) {}
        virtual ~TTimeModification() {}
    private:
        long 	fActualNotes;
        long	fNormalNotes;
        NoteType::type	fNormalType;
        long	fNormalDots;
};
typedef SMARTP<TTimeModification> STimeModification;

} // namespace MusicXML


#endif
